/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpKwiverImproveTrackWorker.h"

#include "vpKwiverVideoSource.h"
#include "vtkVpTrackModel.h"

#include <vtkVgTrack.h>

#include <qtStlUtil.h>

#include <vital/algo/interpolate_track.h>
#include <vital/exceptions/plugin.h>

#include <QEventLoop>
#include <QMessageBox>
#include <QSettings>
#include <QThread>

#include <functional>

namespace kv = kwiver::vital;

//-----------------------------------------------------------------------------
class vpKwiverImproveTrackWorkerPrivate : public QThread
{
public:
  std::shared_ptr<kv::algo::interpolate_track> Algorithm;
  kv::track_sptr InitialTrack;
  kv::track_sptr ImprovedTrack;

  int ProgressMaximum = -1; // Access this from worker thread ONLY!

protected:
  virtual void run() override;
};

QTE_IMPLEMENT_D_FUNC(vpKwiverImproveTrackWorker)

//-----------------------------------------------------------------------------
void vpKwiverImproveTrackWorkerPrivate::run()
{
  try
    {
    this->ImprovedTrack = this->Algorithm->interpolate(this->InitialTrack);
    }
  catch (...)
    {
    }
}

//-----------------------------------------------------------------------------
vpKwiverImproveTrackWorker::vpKwiverImproveTrackWorker(QObject* parent) :
  QObject(parent), d_ptr{new vpKwiverImproveTrackWorkerPrivate}
{
}

//-----------------------------------------------------------------------------
vpKwiverImproveTrackWorker::~vpKwiverImproveTrackWorker()
{
}

//-----------------------------------------------------------------------------
bool vpKwiverImproveTrackWorker::initialize(
  vtkVgTrack* track, const vtkVpTrackModel* trackModel,
  std::shared_ptr<vpKwiverVideoSource> videoSource, double videoHeight)
{
  namespace ph = std::placeholders;

  QTE_D(vpKwiverImproveTrackWorker);

  const auto trackId = track->GetId();

  // Get algorithm to use for algorithm-assisted annotation (AAA)
  QSettings settings;
  settings.beginGroup("AAA");
  auto algorithmClass = settings.value("Algorithm", "spline").toString();

  // Instantiate AAA algorithm
  try
    {
    d->Algorithm =
      kv::algo::interpolate_track::create(stdString(algorithmClass));
    }
  catch (kv::plugin_factory_not_found)
    {
    d->Algorithm = nullptr;
    }

  if (!d->Algorithm)
    {
    QMessageBox::warning(0, "Algorithm Error",
                         "Failed to instantiate \"" + algorithmClass +
                         "\" algorithm.");
    return false;
    }

  // Set algorithm configuration
  settings.beginGroup("Configuration");
  try
    {
    auto config = d->Algorithm->get_configuration();
    for (auto key : config->available_values())
      {
      auto const& qkey = qtString(key);
      auto const& defaultValue = qtString(config->get_value<std::string>(key));
      auto const& value = settings.value(qkey, defaultValue).toString();
      config->set_value(key, stdString(value));
      }
    d->Algorithm->set_configuration(config);
    }
  catch (std::exception e)
    {
    QMessageBox::warning(0, "Algorithm Error",
                         "Failed to configure algorithm: " +
                         QString::fromLocal8Bit(e.what()));
    return false;
    }

  d->Algorithm->set_video_input(videoSource);

  // Bind progress callback
  const auto& callback =
    std::mem_fn(&vpKwiverImproveTrackWorker::updateProgress);

  d->Algorithm->set_progress_callback(
    std::bind(callback, this, ph::_1, ph::_2));

  // Convert selected track to KWIVER track (keeping only keyframes)
  d->InitialTrack = kv::track::create();
  d->InitialTrack->set_id(track->GetId());

  vtkIdType id;
  vtkVgTimeStamp timeStamp;
  track->InitPathTraversal();

  // Iterate over track states
  while ((id = track->GetNextPathPt(timeStamp)) != -1)
    {
    if (trackModel->GetIsKeyframe(trackId, timeStamp))
      {
      const vtkBoundingBox& head = track->GetHeadBoundingBox(timeStamp);
      if (head.IsValid())
        {
        const auto xmin = head.GetBound(0);
        const auto xmax = head.GetBound(1);
        const auto ymin = videoHeight - head.GetBound(3);
        const auto ymax = videoHeight - head.GetBound(2);
        const auto bbox = kv::bounding_box_d{xmin, ymin, xmax, ymax};
        auto obj = std::make_shared<kv::detected_object>(bbox);

        const auto frame = timeStamp.GetFrameNumber();
        const auto time = [](const vtkVgTimeStamp& ts){
          return (ts.HasTime() ? static_cast<kv::time_us_t>(ts.GetTime())
                               : std::numeric_limits<kv::time_us_t>::min());
        }(timeStamp);
        d->InitialTrack->append(
          std::make_shared<kv::object_track_state>(frame, time, obj));
        }
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
std::shared_ptr<kwiver::vital::track> vpKwiverImproveTrackWorker::execute()
{
  QTE_D(vpKwiverImproveTrackWorker);

  QEventLoop loop;

  connect(d, SIGNAL(finished()), &loop, SLOT(quit()));

  d->start();
  loop.exec();
  d->wait();

  return d->ImprovedTrack;
}

//-----------------------------------------------------------------------------
void vpKwiverImproveTrackWorker::updateProgress(int current, int maximum)
{
  // CAUTION: This is executed in the worker thread!
  QTE_D(vpKwiverImproveTrackWorker);

  if (d->ProgressMaximum != maximum)
    {
    d->ProgressMaximum = maximum;
    emit this->progressRangeChanged(0, maximum);
    }

  emit this->progressValueChanged(current);
}
