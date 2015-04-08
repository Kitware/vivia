/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvQueryVideoPlayer.h"
#include "vvQueryVideoPlayerPrivate.h"

#include "vvClipVideoRepresentation.h"

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QHash>
#include <QMap>
#include <QMultiHash>
#include <QScopedPointer>

// QtExtensions includes
#include <qtMap.h>
#include <qtUtil.h>

// visgui includes
#include <vtkVgAdapt.h>
#include <vtkVgCompositeEventRepresentation.h>
#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgTimeStamp.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>
#include <vtkVgUtil.h>
#include <vtkVgVideoMetadata.h>
#include <vtkVgVideoModel0.h>
#include <vtkVgVideoNode.h>
#include <vtkVgVideoProviderBase.h>
#include <vtkVgVideoViewer.h>

// VTK includes
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vgColor.h>

namespace
{

enum EventMagicID
{
  EID_TweeningRegions = -3,
  EID_KeyframeRegions = -2,
  EID_FirstDescriptor = 0
};

//-----------------------------------------------------------------------------
double interpolate(double t, double p0, double p1, double p2, double p3)
{
  const double t2 = t * t;
  const double t3 = t2 * t;

  const double m0 = (p2 - p0) * 0.5;
  const double m1 = (p3 - p1) * 0.5;
  const double a0 =  2 * t3  +  -3 * t2  + 1;
  const double a1 =  1 * t3  +  -2 * t2  + t;
  const double a2 =  1 * t3  +  -1 * t2;
  const double a3 = -2 * t3  +   3 * t2;

  return (a0 * p1) + (a1 * m0) + (a2 * m1) + (a3 * p2);
}

//-----------------------------------------------------------------------------
#define interpolate_coords(_c_) \
  interpolate(t, r0._c_(), r1._c_(), r2._c_(), r3._c_())

QRect interpolate(
  double t, const QRect& r0, const QRect& r1, const QRect& r2, const QRect& r3)
{
  QRect rr;
  rr.setTop(floor(interpolate_coords(top)));
  rr.setLeft(floor(interpolate_coords(left)));
  rr.setRight(ceil(interpolate_coords(right)));
  rr.setBottom(ceil(interpolate_coords(bottom)));

  return rr;
}

} // end anonymous namespace

//-----------------------------------------------------------------------------
static inline bool operator==(const vvDescriptor& a, const vvDescriptor& b)
{
  return vvDescriptor::compare(a, b, vvDescriptor::CompareDetection);
}

//-----------------------------------------------------------------------------
vvQueryVideoPlayer::vvQueryVideoPlayer(QWidget* parent)
  : vvVideoPlayer(new vvQueryVideoPlayerPrivate, parent)
{
  // HACK: should properly use QTE_D instead
  this->Internal =
    static_cast<vvQueryVideoPlayerPrivate*>(this->d_ptr.data());

  this->Internal->UpdatePending = false;
  this->Internal->ColorTracksByPVO = false;
  this->Internal->JumpToTrackId = -1;
  connect(this, SIGNAL(pickedEvent(vtkIdType)),
          this, SLOT(eventSelected(vtkIdType)));
}

//-----------------------------------------------------------------------------
vvQueryVideoPlayer::~vvQueryVideoPlayer()
{
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::loadVideo()
{
  this->loadExternal(*this->Internal->VideoNode);

  // \NOTE:Workaround for reset view zooming in
  // little too much first time.
  this->Internal->ResetView = 1;
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::buildRegionMap()
{
  // Clear existing map
  this->Internal->QueryRegions.clear();

  // Remove existing region "events"
  this->Internal->EventModel->RemoveEvent(EID_TweeningRegions);
  this->Internal->EventModel->RemoveEvent(EID_KeyframeRegions);

  // Anything to do?
  if (!this->Internal->Keyframes.count())
    return;

  // Insert keyframes
  vvQueryVideoPlayerPrivate::KeyframeIter kfIter, kfEnd;
  kfEnd = this->Internal->Keyframes.end();
  for (kfIter = this->Internal->Keyframes.begin(); kfIter != kfEnd; ++kfIter)
    {
    vvQueryVideoPlayerPrivate::RegionInfo r;
    r.IsKeyframe = true;
    r.KeyframeId = kfIter.key();
    r.Region = kfIter.value().Region;
    this->Internal->QueryRegions.insert(kfIter.value().Time, r);
    }

  // Prepare iteration
  vvQueryVideoPlayerPrivate::QueryRegionIter kf[4],
    qrEnd = this->Internal->QueryRegions.end();

  kf[0] = kf[1] = this->Internal->QueryRegions.begin();
  kf[2] = kf[3] = kf[1] + 1;
  if (kf[2] != qrEnd && kf[2] + 1 != qrEnd)
    ++kf[3];

  vvQueryVideoPlayerPrivate::VideoMetaDataIter iter =
    this->Internal->VideoMetaData.upper_bound(kf[1].key());

  // Iterate over video metadata looking for frames with no existing region
  while (iter != this->Internal->VideoMetaData.end() && kf[2] != qrEnd)
    {
    vtkVgTimeStamp frameTimeStamp = iter->first;

    // Check if we need to advance to the next keyframe interval
    if (frameTimeStamp > kf[2].key())
      {
      kf[0] = kf[1];
      kf[1] = kf[2];
      ++kf[2];
      if (kf[3] + 1 != qrEnd) ++kf[3];
      continue;
      }

    // See if we have a region for this frame
    if (!this->Internal->QueryRegions.contains(frameTimeStamp))
      {
      // Insert new interpolated ("tweening") region
      double t = (frameTimeStamp.GetTime() - kf[1].key().GetTime())
                 / (kf[2].key().GetTime() - kf[1].key().GetTime());
      vvQueryVideoPlayerPrivate::RegionInfo r;
      r.IsKeyframe = false;
      r.KeyframeId = kf[1].value().KeyframeId;
      r.Region = interpolate(t, kf[0].value().Region, kf[1].value().Region,
                                kf[2].value().Region, kf[3].value().Region);
      this->Internal->QueryRegions.insert(frameTimeStamp, r);
      }

    // Continue to next video frame
    ++iter;
    }

  // Construct region "events"
  vtkSmartPointer<vtkVgEventBase> kfEventBase =
    this->buildRegionEvent(true);
  vtkSmartPointer<vtkVgEventBase> trEventBase =
    this->buildRegionEvent(false);

  kfEventBase->SetId(EID_KeyframeRegions);
  trEventBase->SetId(EID_TweeningRegions);

  // Add region "events"
  vtkVgEvent* kfEvent = this->Internal->EventModel->AddEvent(kfEventBase);
  vtkVgEvent* trEvent = this->Internal->EventModel->AddEvent(trEventBase);

  // Set "event" colors
  kfEvent->SetDisplayFlags(KeyframeFlag);
  kfEvent->SetCustomColor(0.0, 1.0, 1.0);
  kfEvent->UseCustomColorOn();
  trEvent->SetDisplayFlags(KeyframeFlag);
  trEvent->SetCustomColor(0.0, 0.7, 0.7);
  trEvent->UseCustomColorOn();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgEventBase> vvQueryVideoPlayer::buildRegionEvent(
  bool matchKeyframes)
{
  vtkSmartPointer<vtkVgEventBase> eventBase =
    vtkSmartPointer<vtkVgEventBase>::New();

  double yDim = this->videoHeight();

  vtkVgTimeStamp startFrame(true);      // initialize to MaxTime
  vtkVgTimeStamp endFrame(false);       // initialize to MinTime

  vvQueryVideoPlayerPrivate::QueryRegionIter iter,
    end = this->Internal->QueryRegions.end();
  for (iter = this->Internal->QueryRegions.begin(); iter != end; ++iter)
    {
    if (iter.value().IsKeyframe != matchKeyframes)
      continue;

    if (iter.key() < startFrame)
      startFrame = iter.key();
    if (endFrame < iter.key())
      endFrame = iter.key();

    // clockwise from top left
    double region[8];
    region[0] = iter.value().Region.left();
    region[1] = yDim - iter.value().Region.top() - 1;
    region[2] = iter.value().Region.right();
    region[3] = region[1];
    region[4] = region[2];
    region[5] = yDim - iter.value().Region.bottom() - 1;
    region[6] = region[0];
    region[7] = region[5];

    eventBase->AddRegion(iter.key(), 4, region);
    }

  eventBase->SetStartFrame(startFrame);
  eventBase->SetEndFrame(endFrame);

  return eventBase;
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::eventSelected(vtkIdType id)
{
  if (id < EID_FirstDescriptor)
    {
    // Get current timestamp
    vtkVgTimeStamp vtkTime = this->Internal->EventModel->GetCurrentTimeStamp();

    // Get keyframe ID
    vgTimeStamp time(vtkTime.GetTime(), vtkTime.GetFrameNumber());
    if (!this->Internal->QueryRegions.contains(time))
      return;
    id = this->Internal->QueryRegions[time].KeyframeId;

    emit this->pickedKeyframe(id);
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::setTracks(QList<vvTrack> tracks)
{
  // Load the video; do so first as we need the homographies later
  this->buildVideoModel();

  this->Internal->VideoMetaData =
    this->Internal->VideoModel->GetVideoSource()->GetMetadata();

  this->Internal->ColorTracksByPVO = true;
  if (this->Internal->TrackRepresentation)
    {
    this->Internal->TrackRepresentation->SetColorModeToPVO();
    }

  // Create the track id map and then fill track model
  this->Internal->TrackIdMap.clear();
  this->Internal->Tracks.clear();
  QList<vvTrack>::const_iterator trackIter, end = tracks.end();
  vtkIdType trackId = 0, numPoints = 0;
  for (trackIter = tracks.begin(); trackIter != end; trackIter++, trackId++)
    {
    this->Internal->Tracks[trackId].Id = trackIter->Id;
    this->Internal->TrackIdMap.insert(trackIter->Id, trackId);
    numPoints += static_cast<vtkIdType>(trackIter->Trajectory.size());
    }

  // Now create the track model and necessary allocate memory for vtkPoints
  this->Internal->TrackModel = vtkSmartPointer<vtkVgTrackModel>::New();
  this->Internal->TrackModel->GetPoints()->Allocate(numPoints);

  // Loop the tracks again, create vtkVgTracks, and add to model
  for (trackIter = tracks.begin(); trackIter != end; trackIter++)
    {
    // Create track and add to the internal track map
    vtkVgTrack* track = vtkVgTrack::New();
    vtkIdType trackId = this->Internal->TrackIdMap[trackIter->Id];
    track->SetId(trackId);
    track->SetPoints(this->Internal->TrackModel->GetPoints());
    this->Internal->TrackModel->AddTrack(track);
    track->FastDelete();
    this->Internal->Tracks[trackId].Track = track;

    // Identify track classifications
    // \TODO first, vtkVgTrack should be taking the full map; second, we need
    //       a better way to mangle the ill-defined names from back-ends into
    //       something that vtkVgTrack understands
    const char* keyPerson = "TTPerson", *keyVehicle = "TTVehicle";
    QMap<std::string, double> toc(trackIter->Classification);
    // Alternate accepted form of the triples
    if (toc.count() == 3 && toc.contains("Person")
        && toc.contains("Vehicle") && toc.contains("Other"))
      {
      keyPerson = "Person";
      keyVehicle = "Vehicle";
      }

    // Set track classifications
    const double pp = toc.value(keyPerson,  0.0);
    const double pv = toc.value(keyVehicle, 0.0);
    track->SetPVO(pp, pv, 1.0 - (pp + pv));

    // Add the points to the track; for now don't need (use) the head, so
    // not adding it to the track
    vvTrackTrajectory::const_iterator ptsIter,
                                      ptsEnd = trackIter->Trajectory.end();
    for (ptsIter = trackIter->Trajectory.begin(); ptsIter != ptsEnd; ptsIter++)
      {
      double t1, t2;
      this->Internal->VideoModel->GetVideoSource()->GetTimeRange(t1, t2);

      // Clip the track to the time extents of the video
      if (ptsIter->TimeStamp.Time < t1 || ptsIter->TimeStamp.Time > t2)
        {
        continue;
        }

      vvQueryVideoPlayerPrivate::VideoMetaDataIter hi =
        this->Internal->VideoMetaData.find(ptsIter->TimeStamp);

      if (hi != this->Internal->VideoMetaData.end())
        {
        double outputPt[2];
        vtkVgApplyHomography(ptsIter->ImagePoint.X, ptsIter->ImagePoint.Y,
                             hi->second.Homography, outputPt);
        track->InsertNextPoint(ptsIter->TimeStamp, outputPt, vtkVgGeoCoord());
        }
      else
        {
        std::cerr.flags(ios::fixed);
        std::cerr << "Meta data for corresponding timestamp not found."
                     " Point skipped. Search Time: " << ptsIter->TimeStamp.Time
                  << std::endl;
        }
      }

    // Close the track
    track->Close();
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::setDescriptors(QList<vvDescriptor> descriptors,
                                        bool haveTracks/*=false*/)
{
  this->Internal->Descriptors = descriptors;

  // If we have 'real' tracks, they should have been given to us already, and
  // the video model will already be set up; otherwise, clear any stale tracks
  // and set up the video model now
  if (!haveTracks)
    {
    this->Internal->Tracks.clear();
    this->Internal->TrackIdMap.clear();

    // Load the video; do so first as we need the homographies later
    this->buildVideoModel();
    }

  // Build the track model, which will generate any missing tracks from
  // descriptors using them; must be done before events so that TrackIdMap is
  // populated when we look for the vtkVgTrack to which the event should be
  // added
  this->buildTrackModel(haveTracks);

  this->buildEventModel();

  // Now set up the scene and video player UI
  this->buildScene();
  this->loadVideo();

  // (Re)generate image regions per video frame
  this->buildRegionMap();
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::setKeyframes(
  QHash<vtkIdType, vgRegionKeyframe> keyframes)
{
  this->Internal->Keyframes = keyframes;
  this->buildRegionMap();
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::buildScene()
{
  this->Internal->VideoNode = vtkSmartPointer<vtkVgVideoNode>::New();

  this->Internal->EventRepresentation =
    vtkSmartPointer<vtkVgCompositeEventRepresentation>::New();

  this->Internal->DefaultEventRepresentation =
    vtkSmartPointer<vtkVgEventRegionRepresentation>::New();

  if (this->Internal->DefaultEventColor.isValid())
    {
    this->Internal->DefaultEventRepresentation->SetColor(
      this->Internal->DefaultEventColor.redF(),
      this->Internal->DefaultEventColor.greenF(),
      this->Internal->DefaultEventColor.blueF());
    }

  this->Internal->KeyframeRepresentation =
    vtkSmartPointer<vtkVgEventRegionRepresentation>::New();

  this->Internal->TrackRepresentation =
    vtkSmartPointer<vtkVgTrackRepresentation>::New();

  if (this->Internal->ColorTracksByPVO)
    {
    this->Internal->TrackRepresentation->SetColorModeToPVO();

    double pcolor[] =
      {
      this->Internal->PersonTrackColor.redF(),
      this->Internal->PersonTrackColor.greenF(),
      this->Internal->PersonTrackColor.blueF()
      };
    double vcolor[] =
      {
      this->Internal->VehicleTrackColor.redF(),
      this->Internal->VehicleTrackColor.greenF(),
      this->Internal->VehicleTrackColor.blueF()
      };
    double ocolor[] =
      {
      this->Internal->OtherTrackColor.redF(),
      this->Internal->OtherTrackColor.greenF(),
      this->Internal->OtherTrackColor.blueF()
      };
    double ucolor[] =
      {
      this->Internal->UnclassifiedTrackColor.redF(),
      this->Internal->UnclassifiedTrackColor.greenF(),
      this->Internal->UnclassifiedTrackColor.blueF()
      };

    this->Internal->TrackRepresentation->SetColor(vtkVgTrack::Person,
                                                  pcolor);
    this->Internal->TrackRepresentation->SetColor(vtkVgTrack::Vehicle,
                                                  vcolor);
    this->Internal->TrackRepresentation->SetColor(vtkVgTrack::Other,
                                                  ocolor);
    this->Internal->TrackRepresentation->SetColor(vtkVgTrack::Unclassified,
                                                  ucolor);
    }

  this->Internal->VideoRepresentation =
    vtkSmartPointer<vvClipVideoRepresentation>::New();

  this->Internal->EventRepresentation->AddEventRepresentation(
    this->Internal->DefaultEventRepresentation);
  this->Internal->EventRepresentation->AddEventRepresentation(
    this->Internal->KeyframeRepresentation);

  this->Internal->EventRepresentation->SetEventModel(
    this->Internal->EventModel);
  this->Internal->TrackRepresentation->SetTrackModel(
    this->Internal->TrackModel);
  this->Internal->VideoModel->SetTrackModel(
    this->Internal->TrackModel);
  this->Internal->VideoModel->SetEventModel(
    this->Internal->EventModel);
  this->Internal->VideoRepresentation->SetVideoModel(
    this->Internal->VideoModel);
  this->Internal->VideoRepresentation->SetTrackRepresentation(
    this->Internal->TrackRepresentation);

  // selected events should occlude unselected ones
  this->Internal->DefaultEventRepresentation->SetRegionZOffset(1.0);
  this->Internal->DefaultEventRepresentation->SetDisplayMask(EventFlag);

  this->Internal->KeyframeRepresentation->SetRegionZOffset(2.0);
  this->Internal->KeyframeRepresentation->SetDisplayMask(KeyframeFlag);

  this->Internal->VideoRepresentation->SetEventRepresentation(
    this->Internal->EventRepresentation);

  this->Internal->VideoRepresentation->SetUseModelMatrix(0);

  // make sure tracks are in front of video, and behind event regions
  this->Internal->TrackRepresentation->SetZOffset(0.5);

  this->Internal->VideoNode->SetVideoRepresentation(
    this->Internal->VideoRepresentation);
  this->Internal->VideoNode->SetNodeReferenceFrame(
    vtkVgNodeBase::RELATIVE_REFERENCE);
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::buildEventModel()
{
  this->Internal->EventModel = vtkSmartPointer<vtkVgEventModel>::New();

  // Note: This code is stolen shamelessly from vqCore::addResultNode...
  int count = 0;
  foreach (vvDescriptor descriptor, this->Internal->Descriptors)
    {
    vtkSmartPointer<vtkVgEventBase> eventBase =
      vtkSmartPointer<vtkVgEventBase>::New();

    double yDim = this->videoHeight();

    vtkVgTimeStamp startFrame(true);    // initialize to MaxTime
    vtkVgTimeStamp endFrame(false);     // initialize to MinTime

    vvDescriptorRegionMap::const_iterator regionIter;
    for (regionIter = descriptor.Region.begin();
         regionIter != descriptor.Region.end(); ++regionIter)
      {
      if (regionIter->TimeStamp < startFrame)
        startFrame = regionIter->TimeStamp;
      if (endFrame < regionIter->TimeStamp)
        endFrame = regionIter->TimeStamp;

      // clockwise from top left
      double region[8];
      region[0] = regionIter->ImageRegion.TopLeft.X;
      region[1] = yDim - regionIter->ImageRegion.TopLeft.Y - 1;
      region[2] = regionIter->ImageRegion.BottomRight.X;
      region[3] = region[1];
      region[4] = region[2];
      region[5] = yDim - regionIter->ImageRegion.BottomRight.Y - 1;
      region[6] = region[0];
      region[7] = region[5];

      eventBase->AddRegion(regionIter->TimeStamp, 4, region);
      }

    eventBase->SetId(count++);
    eventBase->SetStartFrame(startFrame);
    eventBase->SetEndFrame(endFrame);
    eventBase->SetDisplayFlags(EventFlag);
    this->Internal->EventModel->AddEvent(eventBase);

    // add event to track map
    for (size_t i = 0, size = descriptor.TrackIds.size(); i < size; ++i)
      {
      vtkIdType trackId = this->Internal->TrackIdMap[descriptor.TrackIds[i]];
      this->Internal->Tracks[trackId].Events.append(eventBase->GetId());
      }
    }
}

//-----------------------------------------------------------------------------
struct TrackPoint
{
  double x, y;
};

typedef QMap<vgTimeStamp, TrackPoint> TrackPointMap;

//-----------------------------------------------------------------------------
void FillTrack(QHash<vtkIdType, TrackPointMap>& tracks, vtkIdType trackId,
               const vvDescriptor* descriptor)
{
  TrackPointMap& track = tracks[trackId];

  vvDescriptorRegionMap::const_iterator regionIter;
  for (regionIter = descriptor->Region.begin();
       regionIter != descriptor->Region.end(); regionIter++)
    {
    // Skip points that are already in the track
    if (track.contains(regionIter->TimeStamp))
      continue;

    // Place track points at bottom center of bounding box
    TrackPoint tp;
    const double left = regionIter->ImageRegion.TopLeft.X,
                 right = regionIter->ImageRegion.BottomRight.X;
    tp.x = 0.5 * (left + right);
    tp.y = regionIter->ImageRegion.BottomRight.Y;

    // Add the point
    track.insert(regionIter->TimeStamp, tp);
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::buildTrackModel(bool incremental)
{
  if (!incremental)
    {
    // Create the track model
    this->Internal->TrackModel = vtkSmartPointer<vtkVgTrackModel>::New();
    }

  // If we have no video, there is nothing to do
  if (!this->Internal->VideoModel)
    return;

  this->Internal->VideoMetaData =
    this->Internal->VideoModel->GetVideoSource()->GetMetadata();

  // Before starting, remember what tracks existed before
  QSet<vvTrackId> existingTracks;
  qtUtil::mapBound(this->Internal->TrackIdMap.keys(),
                   existingTracks, &QSet<vvTrackId>::insert);

  // Divvy up descriptors by their track sources
  QMultiHash<vtkIdType, const vvDescriptor*> trackDescriptors;
  QList<vvDescriptor>::const_iterator
    iter, end = this->Internal->Descriptors.end();
  for (iter = this->Internal->Descriptors.begin(); iter != end; ++iter)
    {
    for (size_t i = 0, count = iter->TrackIds.size(); i < count; ++i)
      {
      // Ignore tracks that existed previously
      if (existingTracks.contains(iter->TrackIds[i]))
        continue;

      // Get internal track ID
      vtkIdType trackId;
      if (this->Internal->TrackIdMap.contains(iter->TrackIds[i]))
        {
        trackId = this->Internal->TrackIdMap[iter->TrackIds[i]];
        }
      else
        {
        const vvTrackId& vtid = iter->TrackIds[i];
        if (incremental)
          {
          const QString idString =
            QString("%1:%2").arg(vtid.Source).arg(vtid.SerialNumber);
          qWarning() << "vvQueryVideoPlayer::buildTrackModel: warning: "
                        "one or more descriptors refer to track"
                     << qPrintable(idString)
                     << "which does not exist in our track set";
          }
        trackId = this->Internal->TrackIdMap.count();
        this->Internal->TrackIdMap.insert(vtid, trackId);
        this->Internal->Tracks[trackId].Id = vtid;
        }
      // Add descriptor to track descriptors list
      trackDescriptors.insert(trackId, &(*iter));
      }
    }

  // Reconstruct tracks
  QHash<vtkIdType, TrackPointMap> tracks;
  foreach (vtkIdType trackId, this->Internal->TrackIdMap)
    {
    // Create track from all available descriptors, dropping them as they
    // become no longer useful, until none are left. We will prefer ones
    // starting before desiredStartTime (the last end time) above those
    // that don't, otherwise we will prefer those that start earliest.
    // Out of those sets, we will pick the one with the latest end time.
    //
    // We will fill in any holes (i.e. if a descriptor skips some frames)
    // using any descriptor available. This is less optimal, but ideally
    // no such gaps will exist.
    vtkVgTimeStamp desiredStartTime(false);
    while (trackDescriptors.contains(trackId))
      {
      // Select the "best" descriptor to use next
      const vvDescriptor* bestDescriptor = 0;
      vtkVgTimeStamp bestStartTime(true), bestEndTime(false);
      foreach (const vvDescriptor* d, trackDescriptors.values(trackId))
        {
        vtkVgTimeStamp start = d->Region.begin()->TimeStamp,
                       end = d->Region.rbegin()->TimeStamp;
        // If the descriptor ends before (or at) the current end of the track,
        // it is only useful for gap filling. Add its points, if any (points we
        // already have are skipped) and remove it from further consideration.
        if (end <= desiredStartTime)
          {
          FillTrack(tracks, trackId, d);
          trackDescriptors.remove(trackId, d);
          continue;
          }

        // Does it start before our desired start time?
        if (start <= desiredStartTime)
          {
          // If our previous best started later, use this one
          if (desiredStartTime < bestStartTime)
            {
            bestDescriptor = d;
            bestStartTime = desiredStartTime;
            }
          // Else, if this one ends later, use it
          else if (bestEndTime < end)
            {
            bestDescriptor = d;
            }
          }
        // Does it start before our previous best start?
        else if (start < bestStartTime)
          {
          bestDescriptor = d;
          bestStartTime = start;
          bestEndTime = end;
          }
        // Is it starting at the same time, but longer than our previous best?
        else if (start == bestStartTime && bestEndTime < end)
          {
          bestDescriptor = d;
          bestEndTime = end;
          }
        }
      // If we found a descriptor to add, add its points, then remove it from
      // our list and update the track end time
      if (bestDescriptor)
        {
        FillTrack(tracks, trackId, bestDescriptor);
        desiredStartTime = bestDescriptor->Region.rbegin()->TimeStamp;
        trackDescriptors.remove(trackId, bestDescriptor);
        }
      }
    }

  if (!incremental)
    {
    // Figure out how many points we'll need in the track model
    vtkIdType numPoints = 0;
    foreach (const TrackPointMap& track, tracks)
      numPoints += track.count();

    this->Internal->TrackModel->GetPoints()->Allocate(numPoints);
    }

  // Loop through the track points we accumulated and create
  // tracks in the model
  foreach (vtkIdType trackId, tracks.keys())
    {
    // Create track and add to the internal track map
    vtkVgTrack* track = vtkVgTrack::New();
    track->SetId(trackId);
    this->Internal->TrackModel->AddTrack(track);
    track->FastDelete();
    track->SetPoints(this->Internal->TrackModel->GetPoints());
    this->Internal->Tracks[trackId].Track = track;

    const TrackPointMap& points = tracks[trackId];
    TrackPointMap::const_iterator iter, end = points.end();
    for (iter = points.begin(); iter != end; ++iter)
      {
      vvQueryVideoPlayerPrivate::VideoMetaDataIter hi =
        this->Internal->VideoMetaData.find(iter.key());

      if (hi != this->Internal->VideoMetaData.end())
        {
        double outputPt[2];
        vtkVgApplyHomography(iter.value().x, iter.value().y,
                             hi->second.Homography, outputPt);
        track->InsertNextPoint(iter.key(), outputPt, vtkVgGeoCoord());
        }
      else
        {
        std::cerr.flags(ios::fixed);
        std::cerr << "Meta data for corresponding timestamp not found."
                     " Point skipped. Search Time: " << iter.key().Time
                  << std::endl;
        }
      }

    // Close the tracks
    track->Close();
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::addEventRepresentation(
  vtkSmartPointer<vtkVgEventRepresentationBase> rep)
{
  this->Internal->EventRepresentation->AddEventRepresentation(rep);
  this->update();
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::setDefaultEventColor(const QColor& color)
{
  this->Internal->DefaultEventColor = color;
  if (this->Internal->DefaultEventRepresentation)
    {
    if (color.isValid())
      {
      this->Internal->DefaultEventRepresentation->SetColor(
        this->Internal->DefaultEventColor.redF(),
        this->Internal->DefaultEventColor.greenF(),
        this->Internal->DefaultEventColor.blueF());
      }
    else
      {
      // \TODO how to clear set color?
      }
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::setTrackTypeColor(
  vtkVgTrack::enumTrackPVOType type, const QColor& color)
{
  switch (type)
    {
    case vtkVgTrack::Person:
      this->Internal->PersonTrackColor = color;
      break;
    case vtkVgTrack::Vehicle:
      this->Internal->VehicleTrackColor = color;
      break;
    case vtkVgTrack::Other:
      this->Internal->OtherTrackColor = color;
      break;
    case vtkVgTrack::Unclassified:
      this->Internal->UnclassifiedTrackColor = color;
      break;
    }

  if (this->Internal->TrackRepresentation)
    {
    if (color.isValid())
      {
      this->Internal->TrackRepresentation->SetColor(
        type, vgColor(color).data().array);
      }
    else
      {
      // \TODO how to clear set color?
      }
    }
}

//-----------------------------------------------------------------------------
bool vvQueryVideoPlayer::timeFromFrameNumber(vgTimeStamp& timeStamp)
{
  // Check that we have a frame number
  if (!timeStamp.HasFrameNumber())
    return false;

  // Search video archive metadata
  // \TODO search video metadata once KWA has frame numbers

  // If that failed (or if we don't have frame numbers in the video archive),
  // search descriptors
  foreach (const vvDescriptor& d, this->Internal->Descriptors)
    {
    vvDescriptorRegionMap::const_iterator iter, end = d.Region.end();
    for (iter = d.Region.begin(); iter != end; ++iter)
      {
      // Skip entries with no time; they aren't useful
      if (!iter->TimeStamp.HasTime())
        continue;
      // Check for a frame number match... if the entry has no frame number,
      // this will fail, so don't need to check it explicitly
      if (iter->TimeStamp.FrameNumber == timeStamp.FrameNumber)
        {
        // Found a match
        timeStamp.Time = iter->TimeStamp.Time;
        return true;
        }
      }
    }

  // No match found
  return false;
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::update()
{
  if (!this->Internal->UpdatePending)
    {
    this->Internal->UpdatePending = true;
    QMetaObject::invokeMethod(this, "updateScene", Qt::QueuedConnection);
    }
  // vvVideoPlayer::update() is deferred to updateScene()
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::reset()
{
  this->Internal->Descriptors.clear();
  this->Internal->Keyframes.clear();

  this->Internal->Tracks.clear();
  this->Internal->TrackIdMap.clear();
  this->Internal->QueryRegions.clear();
  this->Internal->VideoMetaData.clear();

  this->Internal->EventRepresentation = 0;
  this->Internal->DefaultEventRepresentation = 0;
  this->Internal->TrackRepresentation = 0;
  this->Internal->VideoRepresentation = 0;

  this->Internal->EventModel = 0;
  this->Internal->TrackModel = 0;
  this->Internal->VideoModel = 0;
  this->Internal->VideoNode = 0;

  vvVideoPlayer::reset();
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::updateScene()
{
  this->Internal->UpdatePending = false;
  if (this->Internal->EventRepresentation)
    {
    this->Internal->EventRepresentation->Update();
    }
  if (this->Internal->TrackRepresentation)
    {
    this->Internal->TrackRepresentation->Update();
    }
  vvVideoPlayer::update();
}

//-----------------------------------------------------------------------------
QList<vtkIdType> vvQueryVideoPlayer::trackIds()
{
  return this->Internal->Tracks.keys();
}

//-----------------------------------------------------------------------------
vvTrackId vvQueryVideoPlayer::trackId(vtkIdType vtkId)
{
  Q_ASSERT(this->Internal->Tracks.contains(vtkId));
  return this->Internal->Tracks[vtkId].Id;
}

//-----------------------------------------------------------------------------
QList<vtkIdType> vvQueryVideoPlayer::trackEvents(vtkIdType trackId)
{
  Q_ASSERT(this->Internal->Tracks.contains(trackId));
  return this->Internal->Tracks[trackId].Events;
}

//-----------------------------------------------------------------------------
vtkVgEvent* vvQueryVideoPlayer::event(vtkIdType eventId)
{
  return this->Internal->EventModel->GetEvent(eventId);
}

//-----------------------------------------------------------------------------
vvDescriptor vvQueryVideoPlayer::descriptor(vtkIdType eventId)
{
  Q_ASSERT(eventId >= 0 && eventId < this->Internal->Descriptors.count());
  return this->Internal->Descriptors[eventId];
}

//-----------------------------------------------------------------------------
vtkIdType vvQueryVideoPlayer::descriptorEventId(
  const vvDescriptor& descriptor)
{
  return this->Internal->Descriptors.indexOf(descriptor);
}

//-----------------------------------------------------------------------------
QHash<vtkIdType, QSet<vtkIdType> > vvQueryVideoPlayer::regionEvents()
{
  QHash<vtkIdType, QSet<vtkIdType> > matchingEvents;

  // Loop over region boxes looking for matching descriptors
  vvQueryVideoPlayerPrivate::QueryRegionIter qrIter, qrEnd;
  qrEnd = this->Internal->QueryRegions.end();
  for (qrIter = this->Internal->QueryRegions.begin();
       qrIter != qrEnd; ++qrIter)
    {
    vvDescriptorRegionEntry re;
    re.TimeStamp = qrIter.key().GetRawTimeStamp();

    // Find matching descriptors
    for (int i = 0; i < this->Internal->Descriptors.count(); ++i)
      {
      const vvDescriptorRegionMap& drMap =
        this->Internal->Descriptors[i].Region;
      // Look for temporal intersection
      vvDescriptorRegionMap::const_iterator drIter = drMap.find(re);
      if (drIter != drMap.end())
        {
        // Check for spatial intersection
        QRect r(QPoint(drIter->ImageRegion.TopLeft.X,
                       drIter->ImageRegion.TopLeft.Y),
                QPoint(drIter->ImageRegion.BottomRight.X,
                       drIter->ImageRegion.BottomRight.Y));
        if (r.intersects(qrIter->Region))
          {
          // We have a match; add to match map (will be a no-op if this match
          // is already recorded)
          matchingEvents[qrIter->KeyframeId].insert(i);
          }
        }
      }
    }

  return matchingEvents;
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::jumpToTrack(
  vtkIdType trackId, vvQueryVideoPlayer::JumpDirection direction)
{
  vtkVgTrack* track = this->Internal->TrackModel->GetTrack(trackId);
  if (!track)
    return;

  vtkVgTimeStamp time =
    (direction == JumpToEnd ? track->GetEndFrame() : track->GetStartFrame());
  this->jumpToTrack(trackId, time);
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::jumpToTrack(vtkIdType trackId,
                                     const vtkVgTimeStamp& time)
{
  vtkVgTrack* track = this->Internal->TrackModel->GetTrack(trackId);
  if (!track)
    return;

  this->seekTo(time.GetTime());

  this->Internal->JumpToTrackId = trackId;
  this->Internal->JumpToTrackTime = time;
  if (!this->Internal->Seeking)
    {
    this->finishJumpToTrack();
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::jumpToEvent(
  vtkIdType eventId, vvQueryVideoPlayer::JumpDirection direction)
{
  vtkVgEvent* event = this->Internal->EventModel->GetEvent(eventId);
  if (!event)
    return;

  vtkVgTimeStamp time =
    (direction == JumpToEnd ? event->GetEndFrame() : event->GetStartFrame());
  this->jumpToEvent(eventId, time);
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::jumpToEvent(vtkIdType eventId,
                                     const vtkVgTimeStamp& time)
{
  this->seekTo(time.GetTime());

  vtkVgEvent* event = this->Internal->EventModel->GetEvent(eventId);
  if (!event)
    return;

  double goToPoint[2];

  vtkIdType npts;
  vtkIdType* pts;
  event->GetRegion(time, npts, pts);

  if (npts != 0)
    {
    vtkPoints* regionPts = event->GetRegionPoints();

    double upperLeft[3], lowerRight[3];
    regionPts->GetPoint(pts[0], upperLeft);
    regionPts->GetPoint(pts[2], lowerRight);

    // center the camera at the middle of the region
    goToPoint[0] = 0.5 * (upperLeft[0] + lowerRight[0]);
    goToPoint[1] = 0.5 * (upperLeft[1] + lowerRight[1]);
    this->moveCameraTo(goToPoint);
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::jumpToKeyframe(vtkIdType keyframeId)
{
  vtkVgTimeStamp time = this->Internal->Keyframes[keyframeId].Time;
  this->jumpToEvent(EID_KeyframeRegions, time);
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::setTrackVisibility(vtkIdType trackId,
                                            bool visibility)
{
  this->Internal->TrackModel->SetTrackDisplayState(trackId, visibility);
}

//-----------------------------------------------------------------------------
bool vvQueryVideoPlayer::eventVisibility(vtkIdType eventId)
{
  return this->Internal->EventModel->GetEventInfo(eventId).GetDisplayEvent();
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::setEventVisibility(vtkIdType eventId,
                                            bool visibility)
{
  this->Internal->EventModel->SetEventDisplayState(eventId, visibility);
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::setKeyframeVisibility(bool visibility)
{
  if (this->Internal->KeyframeRepresentation)
    {
    this->Internal->KeyframeRepresentation->SetVisible(visibility);
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::refreshEvents()
{
  this->Internal->EventRepresentation->Update();
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vvQueryVideoPlayer::currentTimeStamp()
{
  return this->Internal->EventModel->GetCurrentTimeStamp();
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::updateFrame()
{
  this->vvVideoPlayer::updateFrame();

  if (this->Internal->JumpToTrackId != -1)
    {
    this->finishJumpToTrack();
    }
}

//-----------------------------------------------------------------------------
void vvQueryVideoPlayer::finishJumpToTrack()
{
  vtkVgTrack* track = this->Internal->TrackModel->GetTrack(
                        this->Internal->JumpToTrackId);

  this->Internal->JumpToTrackId = -1;
  if (!track)
    return;

  double goToPoint[2];
  track->GetClosestFramePt(this->Internal->JumpToTrackTime, goToPoint);

  const vtkMatrix4x4* mat =
    this->Internal->TrackRepresentation->GetRepresentationMatrix();
  vtkVgApplyHomography(goToPoint, mat, goToPoint);

  this->moveCameraTo(goToPoint);

  // The initial seek may not have triggered a render (such as when seeking to
  // the same frame already being displayed), so we must force an update here.
  this->Internal->Viewer->ForceRender();
}
