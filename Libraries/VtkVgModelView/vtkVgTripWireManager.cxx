// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgTripWireManager.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkImplicitBoolean.h"
#include "vtkImplicitSelectionLoop.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyLine.h"
#include "vtkSmartPointer.h"

#include "vtkVgEvent.h"
#include "vtkVgTrack.h"
#include "vtkVgTrackModel.h"
#include "vtkVgEventModel.h"

#include <map>

vtkCxxSetObjectMacro(vtkVgTripWireManager, TrackModel, vtkVgTrackModel);
vtkCxxSetObjectMacro(vtkVgTripWireManager, EventModel, vtkVgEventModel);

vtkStandardNewMacro(vtkVgTripWireManager);

struct vtkVgTripWireEventInfo
{
  vtkVgEvent* Event;
  std::vector<vtkVgTimeStamp>  EndTimes;

  vtkVgTripWireEventInfo()
    {
    this->Event = 0;
    }

  ~vtkVgTripWireEventInfo()
    {
    if (this->Event)
      {
      this->Event->UnRegister(NULL);
      this->Event = 0;
      }
    }

  // Function ensures proper reference counting.
  void SetEvent(vtkVgEvent* theEvent)
    {
    if (this->Event != theEvent)
      {
      // Before assigning a new one store the last one
      // so that we can decrement the ref count later.
      vtkVgEvent* temp = this->Event;
      this->Event = theEvent;
      // Increment the ref count.
      if (this->Event != NULL)
        {
        this->Event->Register(NULL);
        }
      if (temp != NULL)
        {
        // Decrement the ref count.
        temp->UnRegister(NULL);
        }
      }
    }

  vtkVgTripWireEventInfo(const vtkVgTripWireEventInfo& fromTripWireEventInfo)
    {
    this->Event = 0;
    this->SetEvent(fromTripWireEventInfo.Event);
    }

  vtkVgTripWireEventInfo& operator=(
    const vtkVgTripWireEventInfo& fromTripWireEventInfo)
    {
    this->Event = 0;
    this->SetEvent(fromTripWireEventInfo.Event);

    return *this;
    }
};

typedef std::map<vtkIdType, std::vector<vtkVgTripWireEventInfo> > TrackEventMap;

struct vtkVgTripWireInfo
{
  vtkPolyLine* TripWire;
  vtkImplicitSelectionLoop* TripWireLoop;
  vtkTimeStamp CheckWireTime;
  TrackEventMap TriggeredEventMap;
  bool Enabled;

  vtkVgTripWireInfo()
    {
    this->Enabled = true;
    this->TripWire = 0;
    this->TripWireLoop = 0;
    }

  ~vtkVgTripWireInfo()
    {
    if (this->TripWire)
      {
      this->TripWire->UnRegister(NULL);
      this->TripWire = 0;
      }
    if (this->TripWireLoop)
      {
      this->TripWireLoop->UnRegister(NULL);
      this->TripWireLoop = 0;
      }
    }

  // Function ensures proper reference counting.
  void SetTripWire(vtkPolyLine* tripWire)
    {
    if (this->TripWire != tripWire)
      {
      // Before assigning a new one store the last one
      // so that we can decrement the ref count later.
      vtkPolyLine* temp = this->TripWire;
      this->TripWire = tripWire;
      // Increment the ref count.
      if (this->TripWire != NULL)
        {
        this->TripWire->Register(NULL);
        }
      if (temp != NULL)
        {
        // Decrement the ref count.
        temp->UnRegister(NULL);
        }
      }
    }

  // Function ensures proper reference counting.
  void SetTripWireLoop(vtkImplicitSelectionLoop* tripWireLoop)
    {
    if (this->TripWireLoop != tripWireLoop)
      {
      // Before assigning a new one store the last one
      // so that we can decrement the ref count later.
      vtkImplicitSelectionLoop* temp = this->TripWireLoop;
      this->TripWireLoop = tripWireLoop;
      // Increment the ref count.
      if (this->TripWireLoop != NULL)
        {
        this->TripWireLoop->Register(NULL);
        }
      if (temp != NULL)
        {
        // Decrement the ref count.
        temp->UnRegister(NULL);
        }
      }
    }

  vtkVgTripWireInfo(const vtkVgTripWireInfo& fromTripWireInfo)
    {
    this->Enabled = fromTripWireInfo.Enabled;
    this->TripWire = 0;
    this->TripWireLoop = 0;
    this->SetTripWire(fromTripWireInfo.TripWire);
    this->SetTripWireLoop(fromTripWireInfo.TripWireLoop);
    this->TriggeredEventMap = fromTripWireInfo.TriggeredEventMap;
    }

  vtkVgTripWireInfo& operator=(const vtkVgTripWireInfo& fromTripWireInfo)
    {
    this->Enabled = fromTripWireInfo.Enabled;
    this->TripWire = 0;
    this->TripWireLoop = 0;
    this->SetTripWire(fromTripWireInfo.TripWire);
    this->SetTripWireLoop(fromTripWireInfo.TripWireLoop);
    this->TriggeredEventMap = fromTripWireInfo.TriggeredEventMap;

    return *this;
    }
};

//----------------------------------------------------------------------------
class vtkVgTripWireManager::vtkInternal
{
public:
  vtkInternal(vtkVgTripWireManager* tripWireManager)
    {
    this->TripWireManager = tripWireManager;
    }

  ~vtkInternal()
    {
    }

  void RemoveTripWireEvents(vtkIdType tripWireId);
  void RemoveEventsFromModel(std::vector<vtkVgTripWireEventInfo>& events);

  void GetTripWireVsTrackIntersections(
    vtkVgTrack* track, vtkVgTripWireInfo& tripWireInfo,
    std::vector<vtkVgTripWireManager::IntersectionInfo>& intersections);
  bool GetTripWireVsSegmentIntersections(
    double trackPts[2][3], const vtkVgTimeStamp (&timeStamps)[2],
    vtkVgTripWireInfo& tripWireInfo,
    vtkVgTripWireManager::IntersectionInfo& intersectionInfo);

  void AddEvent(
    vtkVgTrack* track, vtkVgTimeStamp& startTime,
    vtkVgTripWireManager::IntersectionInfo& intersectionInfo,
    int classifierType, int nextEventId,
    TrackEventMap::iterator& trackEventMap);

  vtkVgTripWireManager* TripWireManager;
  vtkTimeStamp CheckTripWiresTime;

  std::map<vtkIdType, vtkVgTripWireInfo>              TripWires;
};

//----------------------------------------------------------------------------
void vtkVgTripWireManager::vtkInternal::
RemoveTripWireEvents(vtkIdType tripWireId)
{
  std::map<vtkIdType, vtkVgTripWireInfo>::iterator tripWireIter =
    this->TripWires.find(tripWireId);
  if (tripWireIter == this->TripWires.end())
    {
    return;  // trip wire not found
    }

  if (!this->TripWireManager->EventModel)
    {
    vtkGenericWarningMacro("EventModel must be set!");
    return;
    }

  TrackEventMap::iterator tripWireTrackIter;
  for (tripWireTrackIter = tripWireIter->second.TriggeredEventMap.begin();
       tripWireTrackIter != tripWireIter->second.TriggeredEventMap.end(); tripWireTrackIter++)
    {
    this->RemoveEventsFromModel(tripWireTrackIter->second);
    }

  tripWireIter->second.TriggeredEventMap.clear();
}

//----------------------------------------------------------------------------
void vtkVgTripWireManager::vtkInternal::
RemoveEventsFromModel(std::vector<vtkVgTripWireEventInfo>& events)
{
  if (!this->TripWireManager->EventModel)
    {
    vtkGenericWarningMacro("EventModel must be set!");
    return;
    }
  std::vector<vtkVgTripWireEventInfo>::const_iterator eventInfoIter;
  for (eventInfoIter = events.begin(); eventInfoIter != events.end(); eventInfoIter++)
    {
    this->TripWireManager->EventModel->RemoveEvent(eventInfoIter->Event->GetId());
    }
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::vtkInternal::
GetTripWireVsTrackIntersections(
  vtkVgTrack* track, vtkVgTripWireInfo& tripWireInfo,
  std::vector<vtkVgTripWireManager::IntersectionInfo>& intersections)
{
  vtkVgTripWireManager::IntersectionInfo intersectionInfo;
  vtkIdType ptId[2];
  vtkVgTimeStamp timeStamps[2];
  track->InitPathTraversal();
  ptId[0] = track->GetNextPathPt(timeStamps[0]);
  // check each line segment for intersection with the trip wire
  double trackPoints[2][3];
  track->GetPoints()->GetPoint(ptId[0], trackPoints[0]);
  for (vtkIdType i = 1; i < track->GetNumberOfPathPoints(); i++)
    {
    ptId[i % 2] = track->GetNextPathPt(timeStamps[i % 2]);
    track->GetPoints()->GetPoint(ptId[i % 2], trackPoints[i % 2]);
    if (this->GetTripWireVsSegmentIntersections(
      trackPoints, timeStamps, tripWireInfo, intersectionInfo))
      {
      intersectionInfo.TrackId = track->GetId();
      intersections.push_back(intersectionInfo);
      }
    }
}

//-----------------------------------------------------------------------------
bool vtkVgTripWireManager::vtkInternal::GetTripWireVsSegmentIntersections(
  double trackPts[2][3], const vtkVgTimeStamp (&timeStamps)[2],
  vtkVgTripWireInfo& tripWireInfo,
  vtkVgTripWireManager::IntersectionInfo& intersectionInfo)
{
  double t, x[3], pCoords[3], tol = 1e-7;
  int subId;
  int classifierType = this->TripWireManager->TripWireId;
  if (tripWireInfo.TripWire->IntersectWithLine(trackPts[0], trackPts[1], tol, t,
                                               x, pCoords, subId))
    {

    // interpolate to get "time" at intersection
    if (timeStamps[0].HasTime())
      {
      intersectionInfo.IntersectionTime.SetTime(
        timeStamps[0].GetTime() +
        t * (timeStamps[1].GetTime() - timeStamps[0].GetTime()));
      }
    if (timeStamps[0].HasFrameNumber())
      {
      unsigned int frameNumber = 0.5 +
        static_cast<double>(timeStamps[0].GetFrameNumber()) +
        t * (timeStamps[1].GetFrameNumber() - timeStamps[0].GetFrameNumber());
      intersectionInfo.IntersectionTime.SetFrameNumber(frameNumber);
      }

    int earlierPtIndex = 0;  // index (of array of 2 input) of earlier Pt
    if (timeStamps[0] > timeStamps[1])
      {
      earlierPtIndex = 1;
      }

    // if closed loop, additional test to see if entering or exiting
    if (tripWireInfo.TripWireLoop)
      {
      // if> 0, then must have exited
      if (tripWireInfo.TripWireLoop->FunctionValue(trackPts[!earlierPtIndex]) > 0)
        {
        classifierType = this->TripWireManager->ExitingRegionId;
        // both points outside, but crossed a boundary, thus enter AND exit
        if (tripWireInfo.TripWireLoop->FunctionValue(trackPts[earlierPtIndex]) > 0)
          {
          classifierType = -1;
          }
        }
      else
        {
        classifierType = this->TripWireManager->EnteringRegionId;
        // both points inside, but crossed a boundary, thus enter AND exit
        if (tripWireInfo.TripWireLoop->FunctionValue(trackPts[earlierPtIndex]) < 0)
          {
          classifierType = -1;
          }
        }
      }

    intersectionInfo.PreFrame = timeStamps[earlierPtIndex];
    intersectionInfo.PostFrame = timeStamps[!earlierPtIndex];
    intersectionInfo.ClassifierType = classifierType;
    intersectionInfo.IntersectionPt[0] = x[0];
    intersectionInfo.IntersectionPt[1] = x[1];
    return true;
    }
  return false; // does NOT intersect
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::vtkInternal::
AddEvent(vtkVgTrack* track, vtkVgTimeStamp& startTime,
         vtkVgTripWireManager::IntersectionInfo& intersectionInfo,
         int classifierType,
         int nextEventId,
         TrackEventMap::iterator& trackEventMap)
{
  vtkVgEvent* newEvent = vtkVgEvent::New();
  newEvent->SetStartFrame(startTime);
  newEvent->SetEndFrame(intersectionInfo.PostFrame);
  newEvent->AddTrack(track, startTime, intersectionInfo.PostFrame);
  newEvent->AddClassifier(classifierType, 1.0);
  newEvent->SetId(nextEventId);
  double tripLocation[3] = {intersectionInfo.IntersectionPt[0],
                            intersectionInfo.IntersectionPt[1],
                            0.0
                           };
  newEvent->SetTripEventInfo(tripLocation,
                             intersectionInfo.IntersectionTime);

  vtkVgTripWireEventInfo eventInfo;
  eventInfo.SetEvent(newEvent);
  newEvent->FastDelete();

  trackEventMap->second.push_back(eventInfo);
}

//----------------------------------------------------------------------------
vtkVgTripWireManager::vtkVgTripWireManager()
  : EnteringRegionId(-1),  ExitingRegionId(-1), TripWireId(-1),
    EventIdCounter(1000000)
{
  this->TrackModel = 0;
  this->EventModel = 0;
  this->PreTripDuration.SetTime(3e6);   // 3 seconds
  this->PreTripDuration.SetFrameNumber(90);    // 3 seconds, assuming 30 fps
  this->Internals = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkVgTripWireManager::~vtkVgTripWireManager()
{
  delete this->Internals;
  this->SetTrackModel(0);
  this->SetEventModel(0);
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::SetPreTripDuration(const vtkVgTimeStamp& timeStamp)
{
  if (this->PreTripDuration == timeStamp)
    {
    return;
    }

  this->PreTripDuration = timeStamp;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTripWireManager::AddTripWire(vtkPoints* loopPoints,
                                            bool closedLoop,
                                            vtkIdType tripWireId/* = -1*/)
{
  if (tripWireId != -1 &&
      this->Internals->TripWires.find(tripWireId) !=
      this->Internals->TripWires.end())
    {
    return -1;
    }
  else if (tripWireId == -1)
    {
    // if already entries in our trip-wire map, just use one greater (which does mean that
    // we might reuse trip-wire ids).  If no entries yet, start with id = 0
    if (this->Internals->TripWires.size())
      {
      tripWireId = 1 + this->Internals->TripWires.rbegin()->first;
      if (tripWireId == -1)
        {
        // id -1 is not allowed... skip to next one;  only possible if
        // externally specified a negative (<-1) id
        tripWireId = 0;
        }
      }
    else // no id yet, start with 0
      {
      tripWireId = 0;
      }
    }

  vtkVgTripWireInfo tripWireInfo;

  vtkPolyLine* polyLine = vtkPolyLine::New();
  vtkIdType npts = loopPoints->GetNumberOfPoints();
  vtkIdType* pts = new vtkIdType [npts];
  for (vtkIdType i = 0; i < npts; i++)
    {
    pts[i] = i;
    }
  polyLine->Initialize(npts, pts, loopPoints);
  tripWireInfo.SetTripWire(polyLine);
  polyLine->FastDelete();
  delete [] pts;

  if (closedLoop)
    {
    vtkImplicitSelectionLoop* loop = vtkImplicitSelectionLoop::New();
    loop->SetLoop(loopPoints);
    tripWireInfo.SetTripWireLoop(loop);
    loop->FastDelete();
    }
  this->Internals->TripWires[tripWireId] = tripWireInfo;

  this->Modified();
  return tripWireId;
}

//-----------------------------------------------------------------------------
bool vtkVgTripWireManager::IsTripWire(vtkIdType tripWireId)
{
  return this->Internals->TripWires.find(tripWireId) !=
         this->Internals->TripWires.end();
}

//-----------------------------------------------------------------------------
bool vtkVgTripWireManager::RemoveTripWire(vtkIdType tripWireId)
{
  std::map<vtkIdType, vtkVgTripWireInfo>::iterator iter =
    this->Internals->TripWires.find(tripWireId);
  if (iter == this->Internals->TripWires.end())
    {
    return false;
    }

  // Remove the events from the EventModel
  if (this->EventModel)
    {
    this->Internals->RemoveTripWireEvents(tripWireId);
    }
  this->Internals->TripWires.erase(iter);
  this->Modified();
  return true;
}

//-----------------------------------------------------------------------------
bool vtkVgTripWireManager::RemoveAllTripWires()
{
  if (this->Internals->TripWires.size())
    {
    if (this->EventModel)
      {
      // Make sure the events associated with the trip wires are removed
      // from the EventModel
      std::map<vtkIdType, vtkVgTripWireInfo>::iterator iter;
      for (iter = this->Internals->TripWires.begin();
           iter != this->Internals->TripWires.end(); iter++)
        {
        this->Internals->RemoveTripWireEvents(iter->first);
        }
      }
    this->Internals->TripWires.clear();
    this->Modified();
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
int vtkVgTripWireManager::GetNumberOfTripWires()
{
  return static_cast<int>(this->Internals->TripWires.size());
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::CheckTripWires()
{
  if (!this->TrackModel || !this->EventModel)
    {
    vtkErrorMacro("Both the TrackModel and EventModel must be set!");
    return;
    }

  if (this->TripWireId == -1 || this->EnteringRegionId == -1 ||
      this->ExitingRegionId == -1)
    {
    vtkErrorMacro("Must set TripWire, EnteringRegion, and ExitingRegion identifiers!");
    return;
    }

  // if neither the track model nor this object (should just care about ADDing
  // a trip wire, but currently will catch both adding and removing) have
  // changed since lst "check time", there is nothing to do here.
  if (this->TrackModel->GetMTime() < this->Internals->CheckTripWiresTime &&
      this->GetMTime() < this->Internals->CheckTripWiresTime)
    {
    // we've
    return;
    }

//  // only reexecute trip wire if trip wire changed (assumed not to happen for now)
//  // or track data changes (also not assumed to happen for now)
//
//  // thus, we only execute a trip wire when we haven't seen it before (well, that's the plan)

  std::map<vtkIdType, vtkVgTripWireInfo>::iterator iter;
  for (iter = this->Internals->TripWires.begin();
       iter != this->Internals->TripWires.end(); iter++)
    {
    vtkVgTrack* track;

    if (!iter->second.Enabled ||
        this->TrackModel->GetMTime() < iter->second.CheckWireTime)
      {
      // nothing has changed (because we can't yet modify a trip wire)
      continue;
      }

    // for now, always clear all (if any) for this trip wire, and recompute
    this->Internals->RemoveTripWireEvents(iter->first);
    iter->second.CheckWireTime.Modified();

    this->TrackModel->InitTrackTraversal();
    while ((track = this->TrackModel->GetNextTrack().GetTrack()))
      {
      if (track->GetNumberOfPathPoints() < 2)
        {
        continue;
        }

      std::vector<IntersectionInfo> intersections;
      this->Internals->GetTripWireVsTrackIntersections(
        track, iter->second, intersections);

      if (intersections.size() > 0)
        {
        TrackEventMap::iterator trackEventMap =
          iter->second.TriggeredEventMap.find(track->GetId());
        if (trackEventMap == iter->second.TriggeredEventMap.end())
          {
          // no entry yet for this track; create one
          std::vector<vtkVgTripWireEventInfo> tripWireEventInfo;
          iter->second.TriggeredEventMap[track->GetId()] = tripWireEventInfo;
          trackEventMap =
            iter->second.TriggeredEventMap.find(track->GetId());
          }

        std::vector<IntersectionInfo>::iterator intersectionIter;
        for (intersectionIter = intersections.begin();
             intersectionIter != intersections.end(); intersectionIter++)
          {
          // right now, computing start time for the event regardless of whether
          // entering/exiting or open trip wire

          vtkVgTimeStamp startTime = intersectionIter->PostFrame;
          // for now, PreTripDuration measured from the point that
          // caused the crossing, NOT where the interesetion occurred (which
          // we should maybe do at some point)
          startTime.ShiftBackward(this->PreTripDuration);
          if (intersectionIter->PreFrame < startTime)
            {
            // if previous track point is more than PreTripDuration "old",
            // use it for the start
            startTime = intersectionIter->PreFrame;
            }
          else if (startTime < track->GetStartFrame())
            {
            // if PreTripDuration puts us before start of track, use start of
            // track as start
            startTime = track->GetStartFrame();
            }

          // create events for both entering and exiting
          if (intersectionIter->ClassifierType == -1)
            {
            this->Internals->AddEvent(
              track, startTime, *intersectionIter,
              this->EnteringRegionId, this->EventIdCounter++, trackEventMap);
            this->Internals->AddEvent(
              track, startTime, *intersectionIter,
              this->ExitingRegionId, this->EventIdCounter++, trackEventMap);
            }
          else
            {
            this->Internals->AddEvent(
              track, startTime, *intersectionIter,
              intersectionIter->ClassifierType, this->EventIdCounter++,
              trackEventMap);
            }
          }
        }
      }
    }

  this->Internals->CheckTripWiresTime.Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::CheckTripWire(
  vtkIdType tripWireId, std::vector<IntersectionInfo>& intersections)
{
  intersections.clear(); // just to be sure

  std::map<vtkIdType, vtkVgTripWireInfo>::iterator tripWire =
    this->Internals->TripWires.find(tripWireId);
  if (tripWire == this->Internals->TripWires.end())
    {
    vtkWarningMacro("TripWire not found! Id = " << tripWireId);
    return;
    }

  // should we NOT test if the indicated trip-wire is disabled????

  vtkVgTrack* track;
  this->TrackModel->InitTrackTraversal();
  while ((track = this->TrackModel->GetNextTrack().GetTrack()))
    {
    if (track->GetNumberOfPathPoints() < 2)
      {
      continue;
      }

    // continue appending any intersections to the output vector
    this->Internals->GetTripWireVsTrackIntersections(
      track, tripWire->second, intersections);
    }
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::CheckTrackSegment(double pt1[2], double pt2[2],
  const vtkVgTimeStamp& timeStamp1, const vtkVgTimeStamp& timeStamp2,
  std::vector<IntersectionInfo>& intersections)
{
  if (this->TripWireId == -1 || this->EnteringRegionId == -1 ||
      this->ExitingRegionId == -1)
    {
    vtkErrorMacro("Must set TripWire, EnteringRegion, and ExitingRegion identifiers!");
    return;
    }

  intersections.clear();  // just to be sure

  double pts[2][3] = { {pt1[0], pt1[1], 0.0}, {pt2[0], pt2[1], 0.0} };
  const vtkVgTimeStamp timeStamps[2] = {timeStamp1, timeStamp2};

  std::map<vtkIdType, vtkVgTripWireInfo>::iterator iter;
  for (iter = this->Internals->TripWires.begin();
       iter != this->Internals->TripWires.end(); iter++)
    {
    if (!iter->second.Enabled)
      {
      continue;
      }
    IntersectionInfo intersectionInfo;
    if (this->Internals->GetTripWireVsSegmentIntersections(pts, timeStamps,
        iter->second, intersectionInfo))
      {
      intersections.push_back(intersectionInfo);
      }

    }
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::UpdateEventModel(
  const vtkVgTimeStamp& vtkNotUsed(timestamp))
{
  if (!this->TrackModel || !this->EventModel)
    {
    vtkErrorMacro("Both the TrackModel and EventModel must be set!");
    return;
    }

  std::map<vtkIdType, vtkVgTripWireInfo>::const_iterator tripWireIter;
  // for each trip wire
  for (tripWireIter = this->Internals->TripWires.begin();
       tripWireIter != this->Internals->TripWires.end(); tripWireIter++)
    {
    // for each track that intersects the trip wire
    TrackEventMap::const_iterator trackEventMapIter;
    for (trackEventMapIter = tripWireIter->second.TriggeredEventMap.begin();
         trackEventMapIter != tripWireIter->second.TriggeredEventMap.end();
         trackEventMapIter++)
      {
      std::vector<vtkVgTripWireEventInfo>::const_iterator eventsIter;
      for (eventsIter = trackEventMapIter->second.begin();
           eventsIter != trackEventMapIter->second.end(); eventsIter++)
        {
        // if not already added to the model, do so now
        if (tripWireIter->second.Enabled &&
            !this->EventModel->GetEvent(eventsIter->Event->GetId()))
          {
          this->EventModel->AddEvent(eventsIter->Event);
          }
        else if (!tripWireIter->second.Enabled)
          {
          this->EventModel->RemoveEvent(eventsIter->Event->GetId());
          }
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::CheckTrackForTripWireEvents(
  vtkVgTrack* track, bool& tripEvent, bool& enterEvent,
  bool& exitEvent)
{
  // setup variables for results we are looking for
  bool checkForTrip = tripEvent;
  bool checkForEnter = enterEvent;
  bool checkForExit = exitEvent;

  // and then reset, bcasue we use same variables for output
  tripEvent = enterEvent = exitEvent = false;

  if (this->TripWireId == -1 || this->EnteringRegionId == -1 ||
      this->ExitingRegionId == -1)
    {
    vtkErrorMacro("Must set TripWire, EnteringRegion, and ExitingRegion identifiers!");
    return;
    }

  std::map<vtkIdType, vtkVgTripWireInfo>::iterator iter;
  for (iter = this->Internals->TripWires.begin();
       iter != this->Internals->TripWires.end(); iter++)
    {
    if (!iter->second.Enabled)
      {
      continue;
      }
    std::vector<IntersectionInfo> intersections;
    this->Internals->GetTripWireVsTrackIntersections(track, iter->second, intersections);

    std::vector<IntersectionInfo>::iterator intersectionIter;
    for (intersectionIter = intersections.begin();
         intersectionIter != intersections.end(); intersectionIter++)
      {
      if (checkForTrip && intersectionIter->ClassifierType == this->TripWireId)
        {
        tripEvent = true;
        }
      if (checkForEnter &&
          (intersectionIter->ClassifierType == this->EnteringRegionId ||
           intersectionIter->ClassifierType == -1))
        {
        enterEvent = true;
        }
      if (checkForExit &&
          (intersectionIter->ClassifierType == this->ExitingRegionId ||
           intersectionIter->ClassifierType == -1))
        {
        exitEvent = true;
        }
      }

    // have we already found true for everything we're looking for?  If so, return
    if ((!checkForTrip || tripEvent) &&
        (!checkForEnter || enterEvent) &&
        (!checkForExit || exitEvent))
      {
      return;
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgTripWireManager::SetTripWireEnabled(vtkIdType tripWireId, bool state)
{
  std::map<vtkIdType, vtkVgTripWireInfo>::iterator iter =
    this->Internals->TripWires.find(tripWireId);
  if (iter == this->Internals->TripWires.end() ||
      iter->second.Enabled == state)
    {
    return;
    }

  // perhaps should remove from event model NOW, but let UpdateEventModel do
  // the work

  iter->second.Enabled = state;
  this->Modified();
}

//-----------------------------------------------------------------------------
bool vtkVgTripWireManager::GetTripWireEnabled(vtkIdType tripWireId)
{
  std::map<vtkIdType, vtkVgTripWireInfo>::const_iterator iter =
    this->Internals->TripWires.find(tripWireId);
  if (iter == this->Internals->TripWires.end())
    {
    return false;
    }

  return iter->second.Enabled;
}
