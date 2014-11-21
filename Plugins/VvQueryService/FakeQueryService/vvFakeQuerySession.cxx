/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QApplication>
#include <QDataStream>
#include <QList>
#include <QMap>
#include <QSet>
#include <QDir>
#include <QFileInfo>

#include <qtKstReader.h>
#include <qtMath.h>
#include <qtRand.h>
#include <qtStlUtil.h>

#include <vgCheckArg.h>

#include <limits>

#include "vvChecksum.h"
#include "vvHeader.h"
#include "vvReader.h"

#include "vvFakeQuerySession.h"
#include "vvQueryInstance.h"

QTE_IMPLEMENT_D_FUNC(vvFakeQuerySession)

//BEGIN vvFakeQuerySessionPrivate

//-----------------------------------------------------------------------------
class vvFakeQuerySessionPrivate
{
public:
  vvFakeQuerySessionPrivate(vvFakeQuerySession* q, QUrl s);

  bool stWait();

  bool stQueryFormulate();
  bool stQueryExecute();

  void acceptInitialResult(vvQueryResult);
  void mergeResults();
  vvQueryResult mergeResults(const QList<vvQueryResult>&);

  void emitResultSet(const QString& message);

  static bool testOverlap(const vvQueryResult&,
                          const QList<vvQueryResult>&);
  static bool testOverlap(const vvDescriptor&, const vvDescriptor&);
  static long long convertTime(const vgTimeStamp&);

  static bool resultScoreGreaterThan(const vvQueryResult&,
                                     const vvQueryResult&);

  enum
    {
    Shutdown = 0,
    QueryFormulate,
    QueryExecute,
    Wait
    } op;

  QEventLoop* eventLoop;

  const QUrl server;

  vvProcessingRequest qfRequest;
  vvQueryInstance query;

  QList<QString> archiveFiles;

  double progress;
  double progressIncrement;

  QMap<long long, vvQueryResult> rawResults;
  QList<vvQueryResult> mergedResults;

  quint16 seed;
  quint16 lastFeedbackChecksum;

protected:
  QTE_DECLARE_PUBLIC_PTR(vvFakeQuerySession)

private:
  QTE_DECLARE_PUBLIC(vvFakeQuerySession)
};

//-----------------------------------------------------------------------------
vvFakeQuerySessionPrivate::vvFakeQuerySessionPrivate(
  vvFakeQuerySession* q, QUrl s)
  : op(Wait), server(s), lastFeedbackChecksum(0), q_ptr(q)
{
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySessionPrivate::stWait()
{
  bool activeTask = (this->op != Wait);
  this->eventLoop->exec();

  if (this->op == Shutdown)
    {
    if (activeTask)
      {
      QTE_Q(vvFakeQuerySession);
      q->postStatus("Aborted", true);
      }
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySessionPrivate::stQueryFormulate()
{
  QTE_Q(vvFakeQuerySession);

  // Get the path where we will expect to find descriptors
  QUrl uri = qtUrl(this->qfRequest.VideoUri);
  uri.setEncodedPath(uri.encodedPath() + ".vsd");

  vvReader reader;
  vvHeader header;

  // Open file with video descriptors
  if (!(reader.open(uri) && reader.readHeader(header)
        && header.type == vvHeader::Descriptors))
    {
    q->postError("Unable to read file " + uri.toString());
    return false;
    }

  // Read descriptors
  QList<vvDescriptor> descriptors;
  if (!reader.readDescriptors(descriptors))
    {
    q->postError("Error reading descriptors from file " + uri.toString());
    return false;
    }

  // Success; emit descriptors
  emit q->formulationComplete(descriptors);
  q->postStatus("Query video processing complete", true);

  // Return to wait state
  this->op = Wait;
  return true;
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySessionPrivate::stQueryExecute()
{
  QTE_Q(vvFakeQuerySession);

  // \TODO add support for retrieval queries (filtered tracks)

  // If no archives remain, we are done gathering results
  if (this->archiveFiles.isEmpty())
    {
    this->mergedResults.clear();
    this->mergeResults();
    this->emitResultSet("Query completed");
    this->op = Wait;
    return true;
    }

  // Set up to process results from the next archive in the list
  q->postStatus("Executing query...", this->progress);
  QString archiveFile = this->archiveFiles.takeFirst();
  qtKstReader kst(QUrl::fromLocalFile(archiveFile));
  vvReader reader;
  vvHeader header;

  // Open archive file
  if (!(reader.open(QUrl::fromLocalFile(archiveFile))
        && reader.readHeader(header)
        && header.type == vvHeader::QueryResults))
    {
    q->postError("Unable to read file " + archiveFile);
    return false;
    }

  // Read results
  QList<vvQueryResult> kstResults;
  if (!reader.readQueryResults(kstResults))
    {
    q->postError("Error reading results from archive " + archiveFile);
    return false;
    }

  // Process results
  foreach (const vvQueryResult& result, kstResults)
    {
    this->acceptInitialResult(result);
    }

  this->progress += this->progressIncrement;
  q->postStatus("Executing query...", this->progress);

  // Continue with next archive
  return true;
}

//-----------------------------------------------------------------------------
void vvFakeQuerySessionPrivate::acceptInitialResult(vvQueryResult result)
{
  // Split (legacy) results containing multiple descriptors and process each
  // descriptor separately
  if (result.Descriptors.size() != 1)
    {
    typedef std::vector<vvDescriptor> List;
    List descriptors = result.Descriptors;
    foreach_iter (List::const_iterator, iter, descriptors)
      {
      result.Descriptors.clear();
      result.Descriptors.push_back(*iter);
      this->acceptInitialResult(result);
      }
    return;
    }

  // \TODO filter by geospatial and temporal bounds, stream ID

  // \TODO remove 'if' when retrieval mode is implemented; will always be true
  if (this->query.isSimilarityQuery())
    {
    // Compute 'fake' relevancy score
    const double t = this->query.constSimilarityQuery()->SimilarityThreshold;
    quint16 rseed = this->seed ^ vvChecksum(result.Descriptors);
    qsrand(rseed);
    result.RelevancyScore = pow(qtRandD(), 2.0);
    if (result.RelevancyScore < t)
      {
      return;
      }
    }

  // This result passed the filters, so (re)set start/end times and accept it
  const vvDescriptorRegionMap& rmap = result.Descriptors[0].Region;
  if (rmap.size())
    {
    result.StartTime =
      vvFakeQuerySessionPrivate::convertTime(rmap.begin()->TimeStamp);
    result.EndTime =
      vvFakeQuerySessionPrivate::convertTime(rmap.rbegin()->TimeStamp);
    }
  else
    {
    result.StartTime = -1;
    result.EndTime = -1;
    }
  this->rawResults.insert(result.StartTime, result);
}

//-----------------------------------------------------------------------------
void vvFakeQuerySessionPrivate::mergeResults()
{
  QTE_Q(vvFakeQuerySession);

  this->progress = 0.7;
  this->progressIncrement = 0.3 / (1.0 + this->rawResults.count());
  q->postStatus("Executing query...", this->progress);

  while (!this->rawResults.isEmpty())
    {
    QList<vvQueryResult> results;
    long long end = std::numeric_limits<long long>::min();
    QMap<long long, vvQueryResult>::iterator iter = this->rawResults.begin();

    // Generate merge set
    while (iter != this->rawResults.end())
      {
      // Always take at least one
      if (!results.isEmpty())
        {
        // Stop if we've gone past any results that might overlap
        if (iter.key() > end)
          {
          break;
          }

        // Skip results with no overlapping track segments
        if (!vvFakeQuerySessionPrivate::testOverlap(iter.value(), results))
          {
          ++iter;
          continue;
          }
        }

      // Update merged result end time
      long long start = iter.key();
      end = qMax(end, iter.value().EndTime);

      // Append result to merge set
      results.append(iter.value());
      this->rawResults.erase(iter);
      this->progress += this->progressIncrement;

      // Restart from front of list (because a, b may not have overlapped, but
      // if we just added c overlapping a, c might also overlap b, and so we
      // must re-test)
      iter = this->rawResults.begin();

      // Don't merge results that are missing start/end times
      if (start == -1 && end == -1)
        {
        break;
        }
      }

    // Create merged result and add to merged result set
    this->mergedResults.append(this->mergeResults(results));
    q->postStatus("Executing query...", this->progress);
    }
}

//-----------------------------------------------------------------------------
vvQueryResult vvFakeQuerySessionPrivate::mergeResults(
  const QList<vvQueryResult>& results)
{
  // Initialize merged result to first result and set fields that don't (or
  // shouldn't) come from the archive data
  vvQueryResult mr = results.first();
  mr.QueryId = this->query.constAbstractQuery()->QueryId;
  mr.InstanceId = this->mergedResults.count();
  mr.UserScore = vvIqr::UnclassifiedExample;
  int validLocations = (mr.Location.GCS != -1 ? 1 : 0);

  // Merge other results
  for (int i = 1; i < results.count(); ++i)
    {
    const vvQueryResult& r = results[i];

    // Append descriptor
    mr.Descriptors.push_back(r.Descriptors.front());

    // Merge Mission ID and Stream ID
    if (mr.MissionId != r.MissionId)
      {
      mr.MissionId.clear();
      }
    if (mr.StreamId  != r.StreamId)
      {
      mr.StreamId.clear();
      }

    // Sum relevancy scores (we'll divide by number later to get average)
    mr.RelevancyScore += r.RelevancyScore;

    // Merge location, if available
    if (validLocations >= 0 && r.Location.GCS != -1)
      {
      ++validLocations;
      if (r.Location.GCS == mr.Location.GCS)
        {
        // Add location coordinates (will compute average later)
        mr.Location.Northing += r.Location.Northing;
        mr.Location.Easting += r.Location.Easting;
        }
      else if (mr.Location.GCS == -1)
        {
        // So far had no valid location, but this result has one, so use it
        mr.Location = r.Location;
        }
      else
        {
        // Incompatible GCS's; invalidate location in merged result
        mr.Location = vgGeocodedCoordinate();
        validLocations = -1;
        }
      }

    // Grow start/end times
    mr.StartTime = qMin(mr.StartTime, r.StartTime);
    mr.EndTime = qMax(mr.EndTime, r.EndTime);
    }

  // Calculate average relevancy
  mr.RelevancyScore /= results.count();

  // Calculate average location
  if (validLocations > 0 && mr.Location.GCS != -1)
    {
    const double k = 1.0 / validLocations;
    mr.Location.Northing *= k;
    mr.Location.Easting *= k;
    }

  // Return merged result
  return mr;
}

//-----------------------------------------------------------------------------
void vvFakeQuerySessionPrivate::emitResultSet(const QString& message)
{
  QTE_Q(vvFakeQuerySession);

  // Sort results by score
  qSort(this->mergedResults.begin(), this->mergedResults.end(),
        &vvFakeQuerySessionPrivate::resultScoreGreaterThan);

  // (Re)assign rank and emit results
  for (int i = 0, k = this->mergedResults.count(); i < k; ++i)
    {
    vvQueryResult& r = this->mergedResults[i];
    r.Rank = i;
    emit q->resultAvailable(r);
    }

  QString fullMessage("%2; %1 results received");
  q->postStatus(
    fullMessage.arg(this->mergedResults.count()).arg(message), true);
  emit q->resultSetComplete();
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySessionPrivate::testOverlap(
  const vvQueryResult& result, const QList<vvQueryResult>& list)
{
  foreach (const vvQueryResult& other, list)
    {
    // Only consider results from the same stream
    if (other.StreamId != result.StreamId)
      {
      continue;
      }

    // Loop over all descriptor combinations
    size_t i = result.Descriptors.size();
    while (i--)
      {
      const vvDescriptor& di = result.Descriptors[i];
      size_t j = other.Descriptors.size();
      while (j--)
        {
        const vvDescriptor& dj = other.Descriptors[j];
        // If a descriptor pair overlaps, the result overlaps the list
        if (vvFakeQuerySessionPrivate::testOverlap(di, dj))
          {
          return true;
          }
        }
      }
    }

  // No overlap was found
  return false;
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySessionPrivate::testOverlap(
  const vvDescriptor& a, const vvDescriptor& b)
{
  // Test temporal overlap
  if (a.Region.empty() || b.Region.empty())
    {
    return false; // No overlap if regions are empty
    }
  if (a.Region.begin()->TimeStamp > b.Region.rbegin()->TimeStamp)
    {
    return false; // start(a) > end(b)
    }
  if (b.Region.begin()->TimeStamp > a.Region.rbegin()->TimeStamp)
    {
    return false; // start(b) > end(a)
    }

  // Test if descriptors were computed for disjoint track sets
  bool tracksOverlap = true;
  if (a.TrackIds.size() && b.TrackIds.size())
    {
    tracksOverlap = false;
    size_t i = a.TrackIds.size();
    while (i-- && !tracksOverlap)
      {
      const vvTrackId& at = a.TrackIds[i];
      size_t j = b.TrackIds.size();
      while (j--)
        {
        if (at == b.TrackIds[j])
          {
          // Overlap found
          tracksOverlap = true;
          break;
          }
        }
      }
    }

  // If descriptors were computed for disjoint track sets, we don't consider
  // that an overlap, even if the tracks happened to cross
  if (!tracksOverlap)
    {
    return false;
    }

  // Test if any descriptor regions overlap
  typedef vvDescriptorRegionMap::const_iterator Iterator;
  foreach_iter (Iterator, iiter, a.Region)
    {
    Iterator jiter = b.Region.find(*iiter);
    if (jiter != b.Region.end())
      {
      // Test regions at same time for spatial overlap
      const vvImageBoundingBox& ir = iiter->ImageRegion;
      const vvImageBoundingBox& jr = jiter->ImageRegion;
      if (ir.TopLeft.X > jr.BottomRight.X)
        {
        continue; // left(a) > right(b)
        }
      if (jr.TopLeft.X > ir.BottomRight.X)
        {
        continue; // left(b) > right(a)
        }
      if (ir.TopLeft.Y > jr.BottomRight.Y)
        {
        continue; // top(a) > bottom(b)
        }
      if (jr.TopLeft.Y > ir.BottomRight.Y)
        {
        continue; // top(b) > bottom(a)
        }
      // If none of the above tests failed, the region intersection is
      // non-empty, and we have an overlap
      return true;
      }
    }

  // Didn't find an intersection
  return false;
}

//-----------------------------------------------------------------------------
long long vvFakeQuerySessionPrivate::convertTime(const vgTimeStamp& ts)
{
  return (ts.HasTime() ? static_cast<long long>(ts.Time) : -1);
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySessionPrivate::resultScoreGreaterThan(
  const vvQueryResult& a, const vvQueryResult& b)
{
  return a.RelevancyScore > b.RelevancyScore;
}

//END vvFakeQuerySessionPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvFakeQuerySession

//-----------------------------------------------------------------------------
vvFakeQuerySession::vvFakeQuerySession(QUrl server)
  : d_ptr(new vvFakeQuerySessionPrivate(this, server))
{
}

//-----------------------------------------------------------------------------
vvFakeQuerySession::~vvFakeQuerySession()
{
  this->shutdown();
}

//-----------------------------------------------------------------------------
void vvFakeQuerySession::run()
{
  QTE_D(vvFakeQuerySession);

  d->eventLoop = new QEventLoop(this);

  // State machine loop
  bool alive = true;
  while (alive)
    {
    switch (d->op)
      {
      case vvFakeQuerySessionPrivate::Shutdown:
        alive = false;
        break;
      case vvFakeQuerySessionPrivate::QueryFormulate:
        alive = d->stQueryFormulate();
        break;
      case vvFakeQuerySessionPrivate::QueryExecute:
        alive = d->stQueryExecute();
        break;
      default:
        alive = d->stWait();
        break;
      }
    }

  // Clean up
  delete d->eventLoop;
  d->eventLoop = 0;
  emit this->finished();
}

//-----------------------------------------------------------------------------
void vvFakeQuerySession::notify()
{
  if (QThread::currentThread() != this->thread())
    {
    QMetaObject::invokeMethod(this, "notify");
    return;
    }

  QTE_D(vvFakeQuerySession);
  d->eventLoop->quit();
}

//-----------------------------------------------------------------------------
void vvFakeQuerySession::shutdown()
{
  CHECK_ARG(this->thread()->isRunning());

  if (QThread::currentThread() != this->thread())
    {
    QMetaObject::invokeMethod(this, "shutdown", Qt::QueuedConnection);
    this->wait();
    return;
    }

  QTE_D(vvFakeQuerySession);

  d->op = vvFakeQuerySessionPrivate::Shutdown;
  this->notify();
}

//-----------------------------------------------------------------------------
void vvFakeQuerySession::endQuery()
{
  CHECK_ARG(this->thread()->isRunning());

  if (QThread::currentThread() != this->thread())
    {
    QMetaObject::invokeMethod(this, "endQuery");
    return;
    }

  QTE_D(vvFakeQuerySession);

  // Check if we need to abort a currently-running query
  if (d->op == vvFakeQuerySessionPrivate::QueryExecute)
    {
    d->rawResults.clear();
    d->mergedResults.clear();
    d->emitResultSet("Query terminated");
    }

  d->op = vvFakeQuerySessionPrivate::Wait;
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySession::formulateQuery(vvProcessingRequest request)
{
  if (QThread::currentThread() != this->thread())
    {
    this->start();
    QMetaObject::invokeMethod(this, "formulateQuery",
                              Q_ARG(vvProcessingRequest, request));
    return true;
    }

  QTE_D(vvFakeQuerySession);

  d->qfRequest = request;
  d->op = vvFakeQuerySessionPrivate::QueryFormulate;
  this->notify();
  return true;
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySession::processQuery(
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

  QTE_D(vvFakeQuerySession);

  // Reset internal state (shouldn't be necessary, but just in case...)
  d->rawResults.clear();
  d->mergedResults.clear();
  d->archiveFiles.clear();

  // Locate archive
  QDir archiveDir(d->server.queryItemValue("Archive"));
  if (archiveDir.exists())
    {
    QStringList filter;

    // Get list of files to process
    filter.append("*.vqr");
    foreach (QString archiveFile, archiveDir.entryList(filter))
      {
      QFileInfo afi(archiveDir, archiveFile);
      d->archiveFiles.append(afi.absoluteFilePath());
      }

    // Store query and reset seed (if applicable) and progress counters
    d->query = query;
    if (query.isSimilarityQuery())
      {
      d->seed = vvChecksum(query.constSimilarityQuery()->Descriptors);
      }
    d->progress = 0.0;
    d->progressIncrement = 0.7 / (1.0 + d->archiveFiles.count());

    // Begin query execution
    d->op = vvFakeQuerySessionPrivate::QueryExecute;
    this->notify();
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySession::requestRefinement(int resultsToScore)
{
  if (QThread::currentThread() != this->thread())
    {
    this->start();
    QMetaObject::invokeMethod(this, "requestRefinement",
                              Q_ARG(int, resultsToScore));
    return true;
    }

  QTE_D(vvFakeQuerySession);

  this->postStatus("Requesting scoring results for refinement...", -1.0);

  const int k = d->mergedResults.count();
  QList<vvQueryResult> srResults;

  if (k <= resultsToScore)
    {
    srResults = d->mergedResults;
    }
  else
    {
    // Pick a subset of the total results at random
    qsrand(1);
    QSet<int> picks;
    while (resultsToScore)
      {
      int i = qrand() % k;
      while (picks.contains(i))
        {
        i = (i + 1) % k;
        }
      picks.insert(i);
      --resultsToScore;
      }
    foreach (int i, picks)
      {
      srResults.append(d->mergedResults[i]);
      }
    }

  // Send the results
  foreach (vvQueryResult result, srResults)
    {
    result.Rank = -1;
    result.PreferenceScore = qtRandD();
    emit this->resultAvailable(result, true);
    }
  emit this->resultSetComplete(true);
  this->clearStatus();

  return true;
}

//-----------------------------------------------------------------------------
bool vvFakeQuerySession::refineQuery(vvIqr::ScoringClassifiers feedback)
{
  if (QThread::currentThread() != this->thread())
    {
    this->start();
    QMetaObject::invokeMethod(this, "refineQuery",
                              Q_ARG(vvIqr::ScoringClassifiers, feedback));
    return true;
    }

  QTE_D(vvFakeQuerySession);

  this->postStatus("Refining query...", -1.0);

  vvIqr::ScoringClassifiers::Iterator fbi = feedback.begin();
  while (fbi != feedback.end())
    {
    (fbi.value() == vvIqr::UnclassifiedExample
     ? fbi = feedback.erase(fbi)
     : ++fbi);
    }

  if (feedback.count())
    {
    // Compute hash from feedback to use to rescore results; this will give us
    // rescoring that is consistent, but also changes based on the feedback
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    foreach (long long iid, feedback.keys())
      {
      stream << iid << feedback[iid];
      }

    quint16 checksum = qChecksum(data.constData(), data.size());
    if (checksum != d->lastFeedbackChecksum)
      {
      d->seed ^= checksum;
      d->lastFeedbackChecksum = checksum;
      }
    }

  // Rescore the results
  const double t = d->query.constSimilarityQuery()->SimilarityThreshold;
  QList<vvQueryResult>::iterator iter = d->mergedResults.begin();
  while (iter != d->mergedResults.end())
    {
    double newScore = 0.0;
    size_t numDescriptors = iter->Descriptors.size();
    for (size_t n = 0; n < numDescriptors; ++n)
      {
      // Set random seed based on the accumulated feedback (this prevents us
      // changing scores when no feedback has been provided)
      quint16 seed = d->seed ^ vvChecksum(iter->Descriptors[n]);
      qsrand(seed);
      // Add new score for this descriptor to accumulator
      // \TODO it would be better if this took the feedback into account in a
      //       less arbitrary fashion
      newScore += pow(qtRandD(), 2.0);
      }
    newScore /= numDescriptors;

    // Update score, and drop some results
    if (newScore < t)
      {
      iter = d->mergedResults.erase(iter);
      }
    else
      {
      iter->RelevancyScore = newScore;
      ++iter;
      }
    }

  // Emit remaining result set
  d->emitResultSet("Refinement completed");
  return true;
}

//END vvFakeQuerySession
