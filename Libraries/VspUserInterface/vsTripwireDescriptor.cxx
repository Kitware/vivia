/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTripwireDescriptor.h"

#include <vtkPoints.h>

#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTripWireManager.h>

#include <vsContour.h>
#include <vsEventInfo.h>
#include <vsTrackState.h>
#include <vtkVsTrackInfo.h>

#include <vsLiveDescriptorPrivate.h>

//BEGIN vsTripwireDescriptorPrivate

//-----------------------------------------------------------------------------
class vsTripwireDescriptorPrivate : public vsLiveDescriptorPrivate
{
public:
  vsTripwireDescriptorPrivate(vsTripwireDescriptor*);
  virtual ~vsTripwireDescriptorPrivate();

protected:
  virtual void run();

  virtual void injectInput(qint64 id, vsDescriptorInputPtr input);
  virtual void revokeInput(qint64 id, bool revokeEvents);
  virtual void revokeAllInput(bool revokeEvents);

  void addContour(qint64 id, const QPolygonF& points);
  void updateTrack(const vvTrackId* id, const vsTrackState* state);
  void processIntersections(
    std::vector<vtkVgTripWireManager::IntersectionInfo>& intersections,
    vtkVgTrack* track, const vvTrackId* id);
  void emitEvent(
    const vvTrackId& id, const vtkVgTimeStamp& startTime,
    const vtkVgTimeStamp& endTime, double (&tripLocation)[3],
    const vtkVgTimeStamp& tripTime, int classifierType);

  vtkIdType NextTrackId;
  vtkIdType NextEventId;
  vtkVgTripWireManager* TripWireManager;
  QHash<vvTrackId, vtkIdType> TrackModelIdMap;

private:
  QTE_DECLARE_PUBLIC(vsTripwireDescriptor)
};

//-----------------------------------------------------------------------------
vsTripwireDescriptorPrivate::vsTripwireDescriptorPrivate(
  vsTripwireDescriptor* q) :
  vsLiveDescriptorPrivate(q), NextTrackId(0), NextEventId(0)
{
}

//-----------------------------------------------------------------------------
vsTripwireDescriptorPrivate::~vsTripwireDescriptorPrivate()
{
}

//-----------------------------------------------------------------------------
void vsTripwireDescriptorPrivate::run()
{
  QTE_Q(vsTripwireDescriptor);

  // Initialization
  this->TripWireManager = vtkVgTripWireManager::New();
  vtkVgTrackModel* trackModel = vtkVgTrackModel::New();
  this->TripWireManager->SetTrackModel(trackModel);
  trackModel->FastDelete();
  this->TripWireManager->SetTripWireId(vsEventInfo::Tripwire);
  this->TripWireManager->SetEnteringRegionId(vsEventInfo::EnteringRegion);
  this->TripWireManager->SetExitingRegionId(vsEventInfo::ExitingRegion);

  emit q->readyForInput(q);
  vsLiveDescriptorPrivate::run();

  // Cleanup
  this->TripWireManager->Delete();
}

//-----------------------------------------------------------------------------
void vsTripwireDescriptorPrivate::injectInput(
  qint64 id, vsDescriptorInputPtr input)
{
  if (input->type() == vsDescriptorInput::TrackUpdate)
    {
    this->updateTrack(input->trackId(), input->trackState());
    }
  else
    {
    const vsContour* contour = input->contour();
    if (contour && (contour->type() == vsContour::Tripwire))
      {
      this->addContour(id, contour->points());
      }
    }
}

//-----------------------------------------------------------------------------
void vsTripwireDescriptorPrivate::revokeInput(qint64 id, bool revokeEvents)
{
  vtkIdType vtkId = static_cast<vtkIdType>(id);
  if (this->TripWireManager->IsTripWire(vtkId))
    {
    if (revokeEvents)
      {
      // \TODO revoke events associated with the specified contour
      }

    this->TripWireManager->RemoveTripWire(vtkId);
    if (!this->TripWireManager->GetNumberOfTripWires())
      this->setSuicideTimer(30000);
    }
}

//-----------------------------------------------------------------------------
void vsTripwireDescriptorPrivate::revokeAllInput(bool revokeEvents)
{
  if (revokeEvents)
    {
    // \TODO revoke all events
    }
  this->TripWireManager->RemoveAllTripWires();
  this->setSuicideTimer(30000);
}

//-----------------------------------------------------------------------------
void vsTripwireDescriptorPrivate::addContour(qint64 id, const QPolygonF& r)
{
  this->cancelSuicideTimer();
  vtkIdType vtkId = static_cast<vtkIdType>(id);

  // Convert (back) from QPolygonF... maybe not the most efficient way, but
  // for thread safety, the vsDescriptorInput *must* have its own copy of the
  // points
  const bool closedLoop = r.isClosed();
  vtkPoints* loopPts = vtkPoints::New();
  loopPts->SetNumberOfPoints(r.count() - (closedLoop ? 1 : 0));
  vtkIdType ptIndex = 0;
  foreach (const QPointF& point, r)
    {
    loopPts->SetPoint(ptIndex++, point.x(), point.y(), 0.0);
    if (ptIndex == loopPts->GetNumberOfPoints())
      {
      // If a closed loop, we don't want to add last point; we already handled
      // setting number of points correctly, so stop if we've added last
      // desired point
      break;
      }
    }

  // Add contour to the TripWireManager
  vtkIdType tripWireId =
    this->TripWireManager->AddTripWire(loopPts, closedLoop, vtkId);
  loopPts->Delete();

  // Process all known track information against the new region
  std::vector<vtkVgTripWireManager::IntersectionInfo> intersections;
  this->TripWireManager->CheckTripWire(tripWireId, intersections);
  this->processIntersections(intersections, 0, 0);
}

//-----------------------------------------------------------------------------
void vsTripwireDescriptorPrivate::updateTrack(
  const vvTrackId* id, const vsTrackState* state)
{
  if (id && state)
    {
    // Get the relevant track (or start a new one)
    vtkVgTrack* track;
    vtkVgTrackModel* trackModel = this->TripWireManager->GetTrackModel();
    if (this->TrackModelIdMap.contains(*id))
      {
      track = trackModel->GetTrack(this->TrackModelIdMap.value(*id));
      }
    else
      {
      // Track does not exist; assign a new model ID...
      vtkIdType trackModelId = this->NextTrackId++;
      this->TrackModelIdMap.insert(*id, trackModelId);

      // Create a new track
      track = vtkVgTrack::New();
      track->SetId(trackModelId);
      track->SetPoints(trackModel->GetPoints());
      trackModel->AddTrack(track);
      track->FastDelete();
      }

    // Add the new point to the track
    double pt[3] = { state->point.x(), state->point.y(), 0.0 };
    track->SetPoint(state->time, pt, vtkVgGeoCoord());

    // Get previous point and use with current to check for a trip event
    double previousPt[3];
    vtkVgTimeStamp previousTimeStamp;
    if (!track->GetPriorFramePt(state->time, previousPt, previousTimeStamp))
      {
      // No previous point; just return for now
      // \TODO This doesn't handle out-of-order updates properly; should also
      //       check against point after
      return;
      }

    previousPt[2] = 0;
    std::vector<vtkVgTripWireManager::IntersectionInfo> intersections;
    this->TripWireManager->CheckTrackSegment(
      previousPt, pt, previousTimeStamp, state->time, intersections);
    this->processIntersections(intersections, track, id);

    // \NOTE For performance, it would be best to calculate a bounding
    //       box for the track segment, and compare that to the bounding box
    //       (precomputed!) of each known region before progressing to a full
    //       intersection test.
    // \NOTE DO NOT assume that states will arrive in order; this should be
    //       true at the moment, but is expected to change in the future. This
    //       does mean that the arrival of a new segment could mean than an
    //       event that was emitted previously should be revoked :-(. Possibly
    //       this should be left as a FIXME in the initial implementation.
    }
}

//-----------------------------------------------------------------------------
void vsTripwireDescriptorPrivate::processIntersections(
  std::vector<vtkVgTripWireManager::IntersectionInfo>& intersections,
  vtkVgTrack* track, const vvTrackId* id)
{
  if (!intersections.size())
    {
    return;
    }

  // Give event 3 second duration (frames assumes 30 fps)
  vtkVgTimeStamp preTripDuration(3e6, 90);

  vvTrackId tempId;
  if (id)
    {
    tempId = *id;
    }

  // Process (emit events) any intersections
  typedef std::vector<vtkVgTripWireManager::IntersectionInfo>::const_iterator
    IntersectionIter;
  foreach_iter (IntersectionIter, intersectionIter, intersections)
    {
    if (!id)
      {
      // If vvTrackId wasn't passed in, then need to set/get tempId and track
      vtkIdType modelId = intersectionIter->TrackId;
      tempId = this->TrackModelIdMap.key(modelId);
      track = this->TripWireManager->GetTrackModel()->GetTrack(modelId);
      }

    // Right now, computing start time for the event regardless of whether
    // entering/exiting or open trip wire
    vtkVgTimeStamp startTime = intersectionIter->PostFrame;

    // For now, preTripDuration measured from the point that caused the
    // crossing, NOT where the intersection occurred (which we should maybe do
    // at some point)
    startTime.ShiftBackward(preTripDuration);
    if (intersectionIter->PreFrame < startTime)
      {
      // If previous track point is more than preTripDuration "old", use it for
      // the start
      startTime = intersectionIter->PreFrame;
      }
    else if (startTime < track->GetStartFrame())
      {
      // If preTripDuration puts us before start of track, use start of
      // track as start
      startTime = track->GetStartFrame();
      }

    double tripLocation[3] = { intersectionIter->IntersectionPt[0],
                               intersectionIter->IntersectionPt[1], 0.0 };

    // If ClassifierType == -1, the segment that created the intersection has
    // end points either both inside or both outside the trip wire, and thus we
    // should create events for both
    if (intersectionIter->ClassifierType == -1)
      {
      this->emitEvent(tempId, startTime, intersectionIter->PostFrame,
                      tripLocation, intersectionIter->IntersectionTime,
                      vsEventInfo::EnteringRegion);
      this->emitEvent(tempId, startTime, intersectionIter->PostFrame,
                      tripLocation, intersectionIter->IntersectionTime,
                      vsEventInfo::ExitingRegion);
      }
    else
      {
      this->emitEvent(tempId, startTime, intersectionIter->PostFrame,
                      tripLocation, intersectionIter->IntersectionTime,
                      intersectionIter->ClassifierType);
      }
    }
}

//-----------------------------------------------------------------------------
void vsTripwireDescriptorPrivate::emitEvent(
  const vvTrackId& trackId, const vtkVgTimeStamp& startTime,
  const vtkVgTimeStamp& endTime, double (&tripLocation)[3],
  const vtkVgTimeStamp& tripTime, int classifierType)
{
  vsEvent eventBase(QUuid::createUuid());
  eventBase->SetId(this->NextEventId++);
  eventBase->AddClassifier(classifierType, 1.0, 0.0);
  vtkVsTrackInfo* ti = new vtkVsTrackInfo(trackId, startTime, endTime);
  eventBase->AddTrack(ti);
  eventBase->SetTripEventInfo(tripLocation, tripTime);
  emit this->eventAvailable(eventBase);
}


//END vsTripwireDescriptorPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsTripwireDescriptor

//-----------------------------------------------------------------------------
vsTripwireDescriptor::vsTripwireDescriptor()
  : vsLiveDescriptor(new vsTripwireDescriptorPrivate(this))
{
}

//-----------------------------------------------------------------------------
vsTripwireDescriptor::~vsTripwireDescriptor()
{
}

//-----------------------------------------------------------------------------
QString vsTripwireDescriptor::name() const
{
  return "tripwire";
}

//-----------------------------------------------------------------------------
vsDescriptorInput::Types vsTripwireDescriptor::inputAccepted() const
{
  return (vsDescriptorInput::TrackUpdate | vsDescriptorInput::Contour |
          vsDescriptorSource::inputAccepted());
}

//END vsTripwireDescriptor
