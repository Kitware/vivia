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

#include <vital/types/object_track_set.h>

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
class vpKwiverEmbeddedPipelineEndcap : public QThread
{
public:
  vpKwiverEmbeddedPipelineEndcap(vpKwiverEmbeddedPipelineWorkerPrivate* q)
  : q_ptr{q} {}

protected:
  QTE_DECLARE_PUBLIC_PTR(vpKwiverEmbeddedPipelineWorkerPrivate)

  virtual void run() override;

private:
  QTE_DECLARE_PUBLIC(vpKwiverEmbeddedPipelineWorkerPrivate)
};

//-----------------------------------------------------------------------------
class vpKwiverEmbeddedPipelineWorkerPrivate : public QThread
{
public:
  vpKwiverEmbeddedPipelineWorkerPrivate(vpKwiverEmbeddedPipelineWorker* q)
  : endcap{this}, q_ptr{q} {}

  void reportProgress(int);

  std::vector<std::string> framePaths;

  kwiver::arrows::vxl::image_io loader;
  kwiver::embedded_pipeline pipeline;

  vpKwiverEmbeddedPipelineEndcap endcap;

  std::map<kv::track_id_t, kv::track_sptr> tracks;

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

  this->endcap.start();

  // Iterate over frames
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

    // Push data into pipeline
    this->pipeline.send(ids);
  }

  this->pipeline.send_end_of_input();
  this->pipeline.wait();
  this->endcap.wait();
}

//-----------------------------------------------------------------------------
void vpKwiverEmbeddedPipelineWorkerPrivate::reportProgress(int value)
{
  QTE_Q();
  emit q->progressValueChanged(value);
}

//-----------------------------------------------------------------------------
void vpKwiverEmbeddedPipelineEndcap::run()
{
  QTE_Q();

  for (int currentFrame = 0;; ++currentFrame)
  {
    const auto& ods = q->pipeline.receive();

    if (ods->is_end_of_data())
    {
      return;
    }

    // Report progress
    q->reportProgress(currentFrame);

    // Look for tracks
    auto const& iter = ods->find("object_track_set");
    if (iter != ods->end())
    {
      auto const& tracks =
        iter->second->get_datum<kv::object_track_set_sptr>();

      // Update internal track collection
      for (const auto tid : tracks->all_track_ids())
      {
        q->tracks[tid] = tracks->get_track(tid);
      }
    }
  }
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

  if (!d->error.isEmpty())
  {
    QMessageBox::warning(qApp->activeWindow(), "Pipeline Error", d->error);
  }
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

//-----------------------------------------------------------------------------
kv::object_track_set_sptr vpKwiverEmbeddedPipelineWorker::tracks() const
{
  QTE_D();

  std::vector<kv::track_sptr> tracks;
  for (const auto ti : d->tracks)
  {
    tracks.push_back(ti.second);
  }

  return std::make_shared<kv::object_track_set>(tracks);
}
