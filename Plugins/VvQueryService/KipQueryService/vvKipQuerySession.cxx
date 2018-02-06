/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvKipQuerySession.h"

#include <vvQueryInstance.h>

#include <vvAdaptKwiver.h>

#include <vvChecksum.h>
#include <vvHeader.h>
#include <vvReader.h>

#include <vgCheckArg.h>

#include <qtKstReader.h>
#include <qtMath.h>
#include <qtRand.h>
#include <qtStlUtil.h>

#include <QApplication>
#include <QDataStream>
#include <QList>
#include <QMap>
#include <QSet>
#include <QDir>
#include <QFileInfo>

#include <sprokit/processes/adapters/embedded_pipeline.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <vital/types/descriptor_request.h>
#include <vital/types/database_query.h>
#include <vital/types/iqr_feedback.h>
#include <vital/types/query_result_set.h>

#include <fstream>
#include <limits>
#include <memory>
#include <mutex>

QTE_IMPLEMENT_D_FUNC(vvKipQuerySession)

using kwiver::vital::database_query_sptr;
using kwiver::vital::descriptor_request_sptr;
using kwiver::vital::iqr_feedback_sptr;
using kwiver::vital::query_result_set_sptr;
using kwiver::vital::track_descriptor_set_sptr;

namespace // anonymous
{

std::shared_ptr<kwiver::embedded_pipeline> pipeline;
std::mutex pipelineMutex;

//-----------------------------------------------------------------------------
template <typename T, typename... Args>
std::unique_ptr<T> makeUnique(Args&&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace <anonymous>

//BEGIN vvKipQuerySessionPrivate

//-----------------------------------------------------------------------------
class vvKipQuerySessionPrivate
{
public:
  vvKipQuerySessionPrivate(vvKipQuerySession* q, QUrl s);

  bool initialize();

  bool stWait();

  bool stQueryFormulate();
  bool stQueryExecute();
  bool stQueryRefine();
  bool stQueryProcess();

  void acceptInitialResult(vvQueryResult);
  void mergeResults();
  vvQueryResult mergeResults(const QList<vvQueryResult>&);

  enum
  {
    Shutdown = 0,
    QueryFormulate,
    QueryExecute,
    QueryRefine,
    QueryProcess,
    Wait
  } op;

  QEventLoop* eventLoop;

  const QUrl server;

  vvProcessingRequest qfRequest;
  vvQueryInstance query;
  vvIqr::ScoringClassifiers feedback;

  char const* type = "Process";

protected:
  QTE_DECLARE_PUBLIC_PTR(vvKipQuerySession)

private:
  QTE_DECLARE_PUBLIC(vvKipQuerySession)
};

//-----------------------------------------------------------------------------
vvKipQuerySessionPrivate::vvKipQuerySessionPrivate(
  vvKipQuerySession* q, QUrl s) : op(Wait), server(s), q_ptr(q)
{
}

//-----------------------------------------------------------------------------
bool vvKipQuerySessionPrivate::initialize()
{
  QTE_Q(vvKipQuerySession);

  std::lock_guard<std::mutex> lock(pipelineMutex);

  if (pipeline)
  {
    return true;
  }

  auto const& pipePath = this->server.queryItemValue("Pipeline");
  auto const& pipeDir = QFileInfo(pipePath).dir().canonicalPath();

  auto new_pipeline = makeUnique<kwiver::embedded_pipeline>();

  try
  {
    std::ifstream pipeStream;
    pipeStream.open(qPrintable(pipePath), std::ifstream::in);
    if (!pipeStream)
    {
      q->postError("Failed to initialize pipeline from file " + pipePath);
      return false;
    }

    new_pipeline->build_pipeline(pipeStream, qPrintable(pipeDir));
    new_pipeline->start();
  }
  catch (std::exception& e)
  {
    q->postError("Failed to initialize pipeline: " +
                 QString::fromLocal8Bit(e.what()));
    return false;
  }

  pipeline = std::move(new_pipeline);
  return true;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySessionPrivate::stWait()
{
  bool activeTask = (this->op != Wait);
  this->eventLoop->exec();

  if (this->op == Shutdown)
  {
    if (activeTask)
    {
      QTE_Q(vvKipQuerySession);
      q->postStatus("Aborted", true);
    }
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySessionPrivate::stQueryFormulate()
{
  QTE_Q(vvKipQuerySession);

  auto const& exemplarUri = qtUrl(this->qfRequest.VideoUri);
  auto const& queryType =
    exemplarUri.queryItemValue("FormulationType").toLower();

  q->postStatus(QString("Processing exemplar %1...").arg(queryType), -1.0);

  if (!this->initialize())
  {
    return false;
  }

  // Set request on pipeline inputs
  auto ids = kwiver::adapter::adapter_data_set::create();
  ids->add_value("descriptor_request", toKwiver(this->qfRequest));
  ids->add_value("database_query", database_query_sptr{});
  ids->add_value("iqr_feedback", iqr_feedback_sptr{});

  // Send the request through the pipeline and wait for a result
  pipeline->send(ids);
  auto const& ods = pipeline->receive();

  if (ods->is_end_of_data())
  {
    q->postError("Pipeline shut down unexpectedly");
    return false;
  }

  // Grab result from pipeline output data set
  auto const& iter = ods->find("track_descriptor_set");
  if (iter == ods->end())
  {
    q->postError("Output data from pipeline is missing?");
    return false;
  }

  auto const& kwiverDescriptors =
    iter->second->get_datum<track_descriptor_set_sptr>();

  if (!kwiverDescriptors)
  {
    q->postError("No descriptors were computed");
    return false;
  }

  // Convert descriptors
  QList<vvDescriptor> vvDescriptors;
  for (auto const& dp : *kwiverDescriptors)
  {
    vvDescriptors.append(fromKwiver(*dp));
  }

  // Success; emit descriptors
  emit q->formulationComplete(vvDescriptors);
  q->postStatus(QString("Query %1 processing complete").arg(queryType), true);

  // Return to wait state
  this->op = Wait;
  return true;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySessionPrivate::stQueryExecute()
{
  QTE_Q(vvKipQuerySession);

  q->postStatus("Executing query...", -1.0);

  if (!this->initialize())
  {
    return false;
  }

  // Sanity check query
  if (!this->query.isValid())
  {
    q->postError("Active query is not valid?");
    return false;
  }

  // Set pipeline inputs and start the pipe executing
  auto ids = kwiver::adapter::adapter_data_set::create();
  ids->add_value("descriptor_request", descriptor_request_sptr{});
  ids->add_value("database_query", toKwiver(this->query));
  ids->add_value("iqr_feedback", iqr_feedback_sptr{});
  pipeline->send(ids);

  // Switch state
  this->op = QueryProcess;
  this->type = "Query";
  return true;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySessionPrivate::stQueryRefine()
{
  QTE_Q(vvKipQuerySession);

  q->postStatus("Refining query...", -1.0);

  if (!this->initialize())
  {
    return false;
  }

  // Get query ID
  auto const query = this->query.abstractQuery();
  if (!query)
  {
    q->postError("Active query is not valid?");
    return false;
  }
  auto const& queryId = query->QueryId;

  // Set pipeline inputs and start the pipe executing
  auto ids = kwiver::adapter::adapter_data_set::create();
  ids->add_value("descriptor_request", descriptor_request_sptr{});
  ids->add_value("database_query", database_query_sptr{});
  ids->add_value("iqr_feedback", toKwiver(queryId, this->feedback));
  pipeline->send(ids);

  // Switch state
  this->op = QueryProcess;
  this->type = "Refinement";
  return true;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySessionPrivate::stQueryProcess()
{
  QTE_Q(vvKipQuerySession);

  // Wait for a result from the pipeline
  auto const& ods = pipeline->receive();

  if (ods->is_end_of_data())
  {
    q->postError("Pipeline shut down unexpectedly");
    return false;
  }

  // Grab result from pipeline output data set
  auto const& iter = ods->find("query_result");
  if (iter == ods->end())
  {
    q->postError("Output data from pipeline is missing?");
    return false;
  }

  auto const& kwiverResults =
    iter->second->get_datum<query_result_set_sptr>();

  // Convert and emit results
  auto resultCount = 0;
  for (auto const& kwiverResult : *kwiverResults)
  {
    auto vvResult = fromKwiver(*kwiverResult);
    vvResult.Rank = ++resultCount;
    emit q->resultAvailable(vvResult);
  }

  // Emit completion message
  static auto const completedMessage =
    QString{"%1 completed; %2 results received"};
  q->postStatus(completedMessage.arg(this->type).arg(resultCount), true);
  emit q->resultSetComplete();

  // Return to wait state
  this->op = Wait;
  return true;
}

//END vvKipQuerySessionPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvKipQuerySession

//-----------------------------------------------------------------------------
vvKipQuerySession::vvKipQuerySession(QUrl server)
  : d_ptr(new vvKipQuerySessionPrivate(this, server))
{
  static auto const pluginsLoaded [[maybe_unused]] = [](){
    kwiver::vital::plugin_manager::instance().load_all_plugins();
    return true;
  }();
}

//-----------------------------------------------------------------------------
vvKipQuerySession::~vvKipQuerySession()
{
  this->shutdown();
}

//-----------------------------------------------------------------------------
void vvKipQuerySession::run()
{
  QTE_D(vvKipQuerySession);

  d->eventLoop = new QEventLoop(this);

  // State machine loop
  bool alive = true;
  while (alive)
  {
    switch (d->op)
    {
      case vvKipQuerySessionPrivate::Shutdown:
        alive = false;
        break;
      case vvKipQuerySessionPrivate::QueryFormulate:
        alive = d->stQueryFormulate();
        break;
      case vvKipQuerySessionPrivate::QueryExecute:
        alive = d->stQueryExecute();
        break;
      case vvKipQuerySessionPrivate::QueryRefine:
        alive = d->stQueryRefine();
        break;
      case vvKipQuerySessionPrivate::QueryProcess:
        alive = d->stQueryProcess();
        break;
      default:
        alive = d->stWait();
        break;
    }
  }

  delete d->eventLoop;
  d->eventLoop = 0;
  emit this->finished();
}

//-----------------------------------------------------------------------------
void vvKipQuerySession::notify()
{
  if (QThread::currentThread() != this->thread())
  {
    QMetaObject::invokeMethod(this, "notify");
    return;
  }

  QTE_D(vvKipQuerySession);
  d->eventLoop->quit();
}

//-----------------------------------------------------------------------------
void vvKipQuerySession::shutdown()
{
  CHECK_ARG(this->thread()->isRunning());

  if (QThread::currentThread() != this->thread())
  {
    QMetaObject::invokeMethod(this, "shutdown", Qt::QueuedConnection);
    this->wait();
    return;
  }

  QTE_D(vvKipQuerySession);

  d->op = vvKipQuerySessionPrivate::Shutdown;
  this->notify();
}

//-----------------------------------------------------------------------------
void vvKipQuerySession::endQuery()
{
  CHECK_ARG(this->thread()->isRunning());

  if (QThread::currentThread() != this->thread())
  {
    QMetaObject::invokeMethod(this, "endQuery");
    return;
  }

  QTE_D(vvKipQuerySession);

  // Check if we need to abort a currently-running query
  if (d->op == vvKipQuerySessionPrivate::QueryExecute)
  {
    /*
    d->rawResults.clear();
    d->mergedResults.clear();
    d->emitResultSet("Query terminated");
    */
  }

  d->op = vvKipQuerySessionPrivate::Wait;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySession::formulateQuery(vvProcessingRequest request)
{
  if (QThread::currentThread() != this->thread())
  {
    this->start();
    QMetaObject::invokeMethod(this, "formulateQuery",
                              Q_ARG(vvProcessingRequest, request));
    return true;
  }

  QTE_D(vvKipQuerySession);

  d->qfRequest = request;
  d->op = vvKipQuerySessionPrivate::QueryFormulate;
  this->notify();
  return true;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySession::processQuery(
  vvQueryInstance query, int workingSetSize)
{
  if (QThread::currentThread() != this->thread())
  {
    this->start();
    QMetaObject::invokeMethod(this, "processQuery",
                              Q_ARG(vvQueryInstance, query),
                              Q_ARG(int, workingSetSize));
    return true;
  }

  QTE_D(vvKipQuerySession);

  d->query = query;
  d->op = vvKipQuerySessionPrivate::QueryExecute;
  this->notify();
  return true;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySession::requestRefinement(int resultsToScore)
{
  if (QThread::currentThread() != this->thread())
  {
    this->start();
    QMetaObject::invokeMethod(this, "requestRefinement",
                              Q_ARG(int, resultsToScore));
    return true;
  }

  // KWIVER does not provide feedback requests, so nothing to do
  emit this->resultSetComplete(true);
  return true;
}

//-----------------------------------------------------------------------------
bool vvKipQuerySession::refineQuery(vvIqr::ScoringClassifiers feedback)
{
  if (QThread::currentThread() != this->thread())
  {
    this->start();
    QMetaObject::invokeMethod(this, "refineQuery",
                              Q_ARG(vvIqr::ScoringClassifiers, feedback));
    return true;
  }

  QTE_D(vvKipQuerySession);

  d->feedback = feedback;
  d->op = vvKipQuerySessionPrivate::QueryRefine;
  this->notify();
  return true;
}

//END vvKipQuerySession
