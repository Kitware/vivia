/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpKwiverEmbeddedPipelineWorker.h"

#include "vpFileDataSource.h"
#include "vtkVpTrackModel.h"

#include <vtkVgTrack.h>

#include <qtIndexRange.h>
#include <qtStlUtil.h>

#include <QApplication>
#include <QEventLoop>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QThread>

#include <sprokit/processes/adapters/embedded_pipeline.h>

#include <arrows/vxl/image_io.h>

#include <atomic>
#include <functional>

namespace kv = kwiver::vital;

namespace
{

//-----------------------------------------------------------------------------
template <typename T, typename K>
const T* get(const std::vector<T>& c, K const& key)
{
  auto const iter = std::find(c.begin(), c.end(), T{key});
  return (iter == c.end() ? nullptr : std::addressof(*iter));
}

}

//-----------------------------------------------------------------------------
class vpKwiverEmbeddedPipelineWorkerPrivate : public QThread
{
public:
  vpKwiverEmbeddedPipelineWorkerPrivate(vpKwiverEmbeddedPipelineWorker* q)
  : q_ptr{q} {}

  std::vector<std::string> framePaths;

  kwiver::arrows::vxl::image_io loader;
  kwiver::embedded_pipeline pipeline;

  std::atomic<bool> canceled = {false};
  QScopedPointer<QProgressDialog> cancelDialog;

  QString error;

protected:
  QTE_DECLARE_PUBLIC_PTR(vpKwiverEmbeddedPipelineWorker)

  virtual void run() override;

private:
  QTE_DECLARE_PUBLIC(vpKwiverEmbeddedPipelineWorker)
};

QTE_IMPLEMENT_D_FUNC(vpKwiverEmbeddedPipelineWorker)

//-----------------------------------------------------------------------------
void vpKwiverEmbeddedPipelineWorkerPrivate::run()
{
  QTE_Q();

  const auto totalFrames = this->framePaths.size();
  emit q->progressRangeChanged(0, static_cast<int>(totalFrames));
  emit q->progressValueChanged(0);

  const auto ports = this->pipeline.input_port_names();
  const auto image_port = get(ports, "image");
  const auto timestamp_port = get(ports, "timestamp");

  for (auto currentFrame : qtIndexRange(totalFrames))
  {
    if (this->canceled)
    {
      break;
    }

    // Build input data set
    auto ids = kwiver::adapter::adapter_data_set::create();

    if (image_port)
    {
      auto frameImage = this->loader.load(
        this->framePaths[currentFrame]);

      ids->add_value(*image_port, frameImage);
    }

    if (timestamp_port)
    {
      auto ts = kv::timestamp{};
      ts.set_frame(static_cast<kv::frame_id_t>(currentFrame));

      ids->add_value(*timestamp_port, ts);
    }

    // Step pipeline
    this->pipeline.send(ids);
    const auto& ods = this->pipeline.receive();

    if (ods->is_end_of_data())
    {
      this->error = "Pipeline shut down unexpectedly";
      return;
    }

    // Report progress
    emit q->progressValueChanged(static_cast<int>(currentFrame));

    // TODO do something with data received
  }

  this->pipeline.send_end_of_input();
  this->pipeline.wait();
}

//-----------------------------------------------------------------------------
vpKwiverEmbeddedPipelineWorker::vpKwiverEmbeddedPipelineWorker(
  QObject* parent) :
  QObject{parent}, d_ptr{new vpKwiverEmbeddedPipelineWorkerPrivate{this}}
{
}

//-----------------------------------------------------------------------------
vpKwiverEmbeddedPipelineWorker::~vpKwiverEmbeddedPipelineWorker()
{
}

//-----------------------------------------------------------------------------
bool vpKwiverEmbeddedPipelineWorker::initialize(
  const QString& pipelineFile, vpFileDataSource* dataSource,
  const vtkVpTrackModel* trackModel)
{
  QTE_D();

  // Copy inputs
  for (const auto i : qtIndexRange(dataSource->getFileCount()))
  {
    d->framePaths.push_back(dataSource->getDataFile(i));
  }

  // Set up pipeline
  try
  {
    const auto& pipelineDir = QFileInfo{pipelineFile}.canonicalPath();

    std::ifstream pipelineStream;
    pipelineStream.open(stdString(pipelineFile), std::ifstream::in);
    if (!pipelineStream)
    {
      QMessageBox::warning(qApp->activeWindow(), "Pipeline Error",
                           "Failed to initialize pipeline: pipeline file '" +
                           pipelineFile + "' could not be read");
      return false;
    }

    d->pipeline.build_pipeline(pipelineStream, stdString(pipelineDir));
    d->pipeline.start();
  }
  catch (std::exception& e)
  {
    QMessageBox::warning(qApp->activeWindow(), "Pipeline Error",
                         "Failed to initialize pipeline: " +
                         QString::fromLocal8Bit(e.what()));
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
void vpKwiverEmbeddedPipelineWorker::execute()
{
  QTE_D();

  QEventLoop loop;

  connect(d, SIGNAL(finished()), &loop, SLOT(quit()));

  d->start();
  loop.exec();
  d->wait();
}

//-----------------------------------------------------------------------------
void vpKwiverEmbeddedPipelineWorker::cancel()
{
  QTE_D();

  d->canceled = true;

  d->cancelDialog.reset(new QProgressDialog);
  d->cancelDialog->setLabelText("Canceling...");
  d->cancelDialog->setCancelButton(nullptr);
  d->cancelDialog->setRange(0, 0);
  d->cancelDialog->show();
}
