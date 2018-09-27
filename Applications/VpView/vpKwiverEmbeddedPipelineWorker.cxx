/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpKwiverEmbeddedPipelineWorker.h"

#include "vpFileDataSource.h"
#include "vtkVpTrackModel.h"

#include <vtkVgTrack.h>
#include <vtkVgTrackTypeRegistry.h>

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

//-----------------------------------------------------------------------------
std::map<int, double> getTrackToc(vtkVgTrack* in)
{
  auto const type = in->GetType();
  if (type >= 0)
  {
    return {{type, 1.0}};
  }

  return in->GetTOC();
}

//-----------------------------------------------------------------------------
kv::track_sptr convertTrack(
  vtkVgTrack* in, vtkVgTrackTypeRegistry const* trackTypes, double videoHeight)
{
  auto out = kv::track::create();
  out->set_id(in->GetId());

  vtkIdType id;
  vtkVgTimeStamp timeStamp;
  in->InitPathTraversal();

  // Generate object type classifier
  auto const& toc = getTrackToc(in);
  auto dotp =
    (!toc.empty() ? std::make_shared<kv::detected_object_type>() : nullptr);
  for (auto const& c : toc)
  {
    auto const& n = std::string{trackTypes->GetEntityType(c.first).GetName()};
    dotp->set_score(n, c.second);
  }

  // Iterate over track states
  while ((id = in->GetNextPathPt(timeStamp)) != -1)
  {
    const vtkBoundingBox& head = in->GetHeadBoundingBox(timeStamp);
    if (head.IsValid())
    {
      const auto xmin = head.GetBound(0);
      const auto xmax = head.GetBound(1);
      const auto ymin = videoHeight - head.GetBound(3);
      const auto ymax = videoHeight - head.GetBound(2);
      const auto bbox = kv::bounding_box_d{xmin, ymin, xmax, ymax};
      auto obj = std::make_shared<kv::detected_object>(bbox, 1.0, dotp);

      const auto frame = timeStamp.GetFrameNumber();
      const auto time = [](const vtkVgTimeStamp& ts){
        return (ts.HasTime() ? static_cast<kv::time_usec_t>(ts.GetTime())
                             : std::numeric_limits<kv::time_usec_t>::min());
      }(timeStamp);
      out->append(
        std::make_shared<kv::object_track_state>(frame, time, obj));
    }
  }

  return out;
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

  QStringList framePaths;

  kwiver::arrows::vxl::image_io loader;
  kwiver::embedded_pipeline pipeline;

  vpKwiverEmbeddedPipelineEndcap endcap;

  std::vector<kv::track_sptr> tracksIn;
  std::map<kv::track_id_t, kv::track_sptr> tracksOut;

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

  const auto totalFrames = this->framePaths.count();
  emit q->progressRangeChanged(0, totalFrames);
  emit q->progressValueChanged(0);

  const auto ports = this->pipeline.input_port_names();
  const auto image_port = get(ports, "image");
  const auto timestamp_port = get(ports, "timestamp");
  const auto tracks_port = get(ports, "object_track_set");

  auto tracks = std::make_shared<kv::object_track_set>(this->tracksIn);

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
      // FIXME must use VTK loader
      auto frameImage = this->loader.load(
        stdString(this->framePaths[currentFrame]));

      ids->add_value(*image_port, frameImage);
    }

    if (timestamp_port)
    {
      auto ts = kv::timestamp{};
      ts.set_frame(static_cast<kv::frame_id_t>(currentFrame));

      ids->add_value(*timestamp_port, ts);
    }

    if (tracks_port)
    {
      ids->add_value(*tracks_port, tracks);
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
        q->tracksOut[tid] = tracks->get_track(tid);
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
  vtkVpTrackModel* trackModel, vtkVgTrackTypeRegistry const* trackTypes,
  double videoHeight)
{
  QTE_D();

  // Copy inputs
  d->framePaths = dataSource->frameNames();

  trackModel->InitTrackTraversal();
  while (const auto& track = trackModel->GetNextTrack())
  {
    d->tracksIn.push_back(
      convertTrack(track.GetTrack(), trackTypes, videoHeight));
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
  for (const auto ti : d->tracksOut)
  {
    tracks.push_back(ti.second);
  }

  return std::make_shared<kv::object_track_set>(tracks);
}
