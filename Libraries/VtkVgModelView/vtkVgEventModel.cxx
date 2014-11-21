/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgEventModel.h"

#include "vgEventType.h"
#include "vtkVgContourOperatorManager.h"
#include "vtkVgEvent.h"
#include "vtkVgEventInfo.h"
#include "vtkVgEventTypeRegistry.h"
#include "vtkVgTemporalFilters.h"
#include "vtkVgTrack.h"
#include "vtkVgTrackModel.h"

#include <vtkCommand.h>
#include <vtkIdList.h>
#include <vtkIdListCollection.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkTimeStamp.h>

#include <assert.h>
#include <map>

vtkStandardNewMacro(vtkVgEventModel);

vtkCxxSetObjectMacro(vtkVgEventModel, TrackModel, vtkVgTrackModel);
vtkCxxSetObjectMacro(vtkVgEventModel, SharedRegionPoints, vtkPoints);

typedef std::map<vtkIdType, vtkVgEventInfo>           EventMap;
typedef std::map<vtkIdType, vtkVgEventInfo>::iterator EventMapIterator;
typedef std::map<int, double> EventNormalcyMap;

//----------------------------------------------------------------------------
struct vtkVgEventModel::vtkInternal
{
  EventMap         EventIdMap;
  EventMapIterator EventIter;

  EventNormalcyMap NormalcyMinimum;
  EventNormalcyMap NormalcyMaximum;

  vtkTimeStamp UpdateTime;
  vtkTimeStamp SpatialFilteringUpdateTime;
  vtkTimeStamp TemporalFilteringUpdateTime;

  std::vector<EventLink> EventLinks;
};

//-----------------------------------------------------------------------------
vtkVgEventModel::vtkVgEventModel()
{
  this->TrackModel = 0;
  this->UseSharedRegionPoints = true;
  this->SharedRegionPoints = vtkPoints::New();

  this->Internal = new vtkInternal;

  this->ShowEventsBeforeStart = false;
  this->ShowEventsAfterExpiration = false;
  this->ShowEventsUntilSupportingTracksExpire = false;

  this->UseTrackGroups = false;

  this->EventExpirationOffset.SetFrameNumber(0u);
  this->EventExpirationOffset.SetTime(0.0);

  this->CurrentTimeStamp.SetToMinTime();
}

//-----------------------------------------------------------------------------
vtkVgEventModel::~vtkVgEventModel()
{
  this->SetTrackModel(0);
  this->SetSharedRegionPoints(0);

  for (EventMapIterator itr = this->Internal->EventIdMap.begin(),
       end = this->Internal->EventIdMap.end(); itr != end; ++itr)
    {
    itr->second.GetEvent()->UnRegister(this);
    }

  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
vtkVgEvent* vtkVgEventModel::GetEvent(vtkIdType eventId)
{
  EventMapIterator eventIter = this->Internal->EventIdMap.find(eventId);
  if (eventIter != this->Internal->EventIdMap.end())
    {
    return eventIter->second.GetEvent();
    }
  return 0;
}

//-----------------------------------------------------------------------------
vtkVgEventInfo vtkVgEventModel::GetEventInfo(vtkIdType eventId)
{
  EventMapIterator eventIter = this->Internal->EventIdMap.find(eventId);
  if (eventIter != this->Internal->EventIdMap.end())
    {
    return eventIter->second;
    }
  return vtkVgEventInfo();
}

//-----------------------------------------------------------------------------
vtkVgTrackDisplayData vtkVgEventModel::GetTrackDisplayData(vtkVgEvent* event,
  int trackIndex)
{
  vtkVgTimeStamp eventStart = event->GetStartFrame();
  vtkVgTimeStamp eventExpiration = event->GetEndFrame();
  eventExpiration.ShiftForward(this->EventExpirationOffset);

  // Never display the event track if its start occurs before the reference time.
  if (this->ReferenceTimeStamp.IsValid() &&
      eventStart < this->ReferenceTimeStamp)
    {
    return vtkVgTrackDisplayData();
    }

  if (!this->ShowEventsBeforeStart &&
      this->CurrentTimeStamp < eventStart)
    {
    return vtkVgTrackDisplayData();
    }

  if (!this->ShowEventsAfterExpiration &&
      this->CurrentTimeStamp > eventExpiration)
    {
    return vtkVgTrackDisplayData();
    }

  // Display the whole event if showing before the start of the event.
  if (this->CurrentTimeStamp < eventStart)
    {
    return event->GetTrackDisplayData(trackIndex, this->UseTrackGroups,
                                      vtkVgTimeStamp(false),
                                      vtkVgTimeStamp(true));
    }

  // Waiting for track expiration?
  if (this->ShowEventsUntilSupportingTracksExpire)
    {
    vtkVgTimeStamp maxTime(false);
    int end = this->UseTrackGroups ?
              event->GetNumberOfTrackGroups() : event->GetNumberOfTracks();
    for (int i = 0; i < end; ++i)
      {
      vtkVgTimeStamp endFrame = this->TrackModel->GetTrackEndDisplayFrame(
        this->UseTrackGroups ? event->GetTrackGroupTrack(i) :
          event->GetTrack(i));
      if (endFrame > maxTime)
        {
        maxTime = endFrame;
        }
      }
    if (this->CurrentTimeStamp > maxTime)
      {
      return vtkVgTrackDisplayData();
      }
    }

  // Display from the first frame of the event track up to the current frame.
  return event->GetTrackDisplayData(trackIndex, this->UseTrackGroups,
                                    vtkVgTimeStamp(false),
                                    this->CurrentTimeStamp);
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::GetEvents(vtkIdType trackId,
  std::vector<vtkVgEvent*>& events)
{
  EventMapIterator eventIter = this->Internal->EventIdMap.begin();
  while (eventIter != this->Internal->EventIdMap.end())
    {
    vtkVgEvent* event = eventIter->second.GetEvent();
    if (event->HasTrack(trackId))
      {
      events.push_back(event);
      }
    ++eventIter;
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::GetEvents(vtkVgTrack* track,
  std::vector<vtkVgEvent*>& events)
{
  if (!track)
    {
    vtkErrorMacro("ERROR: NULL or invalid track\n");
    return;
    }

  this->GetEvents(track->GetId(), events);
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::InitEventTraversal()
{
  this->Internal->EventIter = this->Internal->EventIdMap.begin();
}

//-----------------------------------------------------------------------------
vtkVgEventInfo vtkVgEventModel::GetNextEvent()
{
  if (this->Internal->EventIter != this->Internal->EventIdMap.end())
    {
    return this->Internal->EventIter++->second;
    }

  return vtkVgEventInfo();
}

//-----------------------------------------------------------------------------
vtkVgEventInfo vtkVgEventModel::GetNextDisplayedEvent()
{
  while (this->Internal->EventIter != this->Internal->EventIdMap.end())
    {
    if (this->Internal->EventIter->second.GetDisplayEvent() &&
        this->Internal->EventIter->second.GetPassesFilters())
      {
      return this->Internal->EventIter++->second;
      }
    ++this->Internal->EventIter;
    }

  return vtkVgEventInfo();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgEventModel::GetNumberOfEvents()
{
  return static_cast<vtkIdType>(this->Internal->EventIdMap.size());
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::SetEventExpirationOffset(const vtkVgTimeStamp& offset)
{
  if (offset != this->EventExpirationOffset)
    {
    this->EventExpirationOffset = offset;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::Initialize()
{
  this->Internal->EventIdMap.clear();
  this->Internal->NormalcyMinimum.clear();
  this->Internal->NormalcyMaximum.clear();
}

//-----------------------------------------------------------------------------
vtkVgEvent* vtkVgEventModel::AddEvent(vtkVgEventBase* vgEventBase)
{
  vtkVgEventInfo eventInfo;
  vtkVgEvent* vgEvent = vtkVgEvent::SafeDownCast(vgEventBase);
  if (!vgEvent)
    {
    vgEvent = vtkVgEvent::New();
    if (this->UseSharedRegionPoints)
      {
      vgEvent->SetRegionPoints(this->SharedRegionPoints);
      }
    vgEvent->DeepCopy(vgEventBase, true);
    if (this->TrackModel)
      {
      // if we have a track model, try to resolve the track ptr

      for (unsigned int i = 0; i < vgEvent->GetNumberOfTracks(); i++)
        {
        if (!vgEvent->SetTrackPtr(i,
            this->TrackModel->GetTrack(vgEvent->GetTrackId(i))))
          {
          // not sure how to handle this yet... since now have events that
          // might not have resolved tracks (display something of interest
          // via region), we don't necessarily need to throw away events
          // that we can't link up.  However, we will throw them away if
          // were able to find some of the tracks for the events, but not al.
          if (i != 0)
            {
            vtkErrorMacro("Resolved some, but not all, track ptrs in event... thus NOT added!");
            vgEvent->Delete();
            return 0;
            }
          else
            {
            vtkErrorMacro("Failed to resolve track ptrs in event... but event added anyway!");
            break;
            }
          }
        }
      }
    eventInfo.SetEvent(vgEvent);
    vgEvent->Register(this);
    vgEvent->FastDelete();
    }
  else
    {
    eventInfo.SetEvent(vgEvent);
    vgEvent->Register(this);
    }
  this->Modified();

  eventInfo.SetDisplayEventOn();
  this->Internal->EventIdMap[vgEvent->GetId()] = eventInfo;

  // Update minimum and maximum normalcy for this event's classifiers
  EventNormalcyMap::iterator itr;
  for (bool valid = vgEvent->InitClassifierTraversal(); valid;
       valid = vgEvent->NextClassifier())
    {
    int type = vgEvent->GetClassifierType();
    double normalcy = vgEvent->GetClassifierNormalcy();

    itr = this->Internal->NormalcyMinimum.find(type);
    if (itr == this->Internal->NormalcyMinimum.end() ||
        itr->second > normalcy)
      {
      this->Internal->NormalcyMinimum[type] = normalcy;
      }

    itr = this->Internal->NormalcyMaximum.find(type);
    if (itr == this->Internal->NormalcyMaximum.end() ||
        itr->second < normalcy)
      {
      this->Internal->NormalcyMaximum[type] = normalcy;
      }
    }
  return vgEvent;
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::AddEventLink(const EventLink& link)
{
  this->Internal->EventLinks.push_back(link);
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::GetEventLink(int index, EventLink& link)
{
  link = this->Internal->EventLinks[index];
}

//-----------------------------------------------------------------------------
int vtkVgEventModel::GetNumberOfEventLinks()
{
  return static_cast<int>(this->Internal->EventLinks.size());
}

//-----------------------------------------------------------------------------
vtkVgEvent* vtkVgEventModel::CreateAndAddEvent(int type, vtkIdList* trackIds)
{
  if (!this->TrackModel)
    {
    return 0;
    }
  vtkVgEvent* event = vtkVgEvent::New();
  if (this->UseSharedRegionPoints)
    {
    event->SetRegionPoints(this->SharedRegionPoints);
    }

  vtkIdType id = this->GetNextAvailableId();

  event->SetId(id);
  event->AddClassifier(type, 0.0, 1.0);

  event->SetFlags(vtkVgEvent::EF_UserCreated |
                  vtkVgEvent::EF_Dirty |
                  vtkVgEvent::EF_Modifiable);

  vtkVgTimeStamp minFrame(true);
  vtkVgTimeStamp maxFrame(false);

  for (vtkIdType i = 0, end = trackIds->GetNumberOfIds(); i < end; ++i)
    {
    vtkVgTrack* track = this->TrackModel->GetTrack(trackIds->GetId(i));

    vtkVgTimeStamp startFrame = track->GetStartFrame();
    vtkVgTimeStamp endFrame = track->GetEndFrame();

    event->AddTrack(track, startFrame, endFrame);

    minFrame = std::min(minFrame, startFrame);
    maxFrame = std::max(maxFrame, endFrame);
    }

  // for now, have the event span the entire duration of all child tracks
  event->SetStartFrame(minFrame);
  event->SetEndFrame(maxFrame);

  this->AddEvent(event);
  event->FastDelete();

  this->Modified();
  return event;
}

//-----------------------------------------------------------------------------
vtkVgEvent* vtkVgEventModel::CloneEvent(vtkIdType eventId)
{
  vtkVgEvent* srcEvent = this->GetEvent(eventId);
  if (!srcEvent)
    {
    return 0;
    }

  vtkVgEvent* event = vtkVgEvent::New();
  event->CloneEvent(srcEvent);

  vtkIdType id = this->Internal->EventIdMap.empty()
                   ? 0
                   : (--this->Internal->EventIdMap.end())->first + 1;

  event->SetId(id);
  this->AddEvent(event);
  event->FastDelete();

  this->Modified();
  return event;
}

//-----------------------------------------------------------------------------
bool vtkVgEventModel::RemoveEvent(vtkIdType eventId)
{
  EventMapIterator iter = this->Internal->EventIdMap.find(eventId);
  if (iter != this->Internal->EventIdMap.end())
    {
    vtkVgEvent* event = iter->second.GetEvent();
    this->InvokeEvent(vtkVgEventModel::EventRemoved, event);
    this->Internal->EventIdMap.erase(iter);
    event->UnRegister(this);
    this->Modified();
    return true;
    }
  return false;  // not removed (not present)
}

//-----------------------------------------------------------------------------
double vtkVgEventModel::GetNormalcyMinForType(int type)
{
  EventNormalcyMap::iterator itr = this->Internal->NormalcyMinimum.find(type);
  if (itr == this->Internal->NormalcyMinimum.end())
    {
    return VTK_DOUBLE_MAX;
    }
  return itr->second;
}

//-----------------------------------------------------------------------------
double vtkVgEventModel::GetNormalcyMaxForType(int type)
{
  EventNormalcyMap::iterator itr = this->Internal->NormalcyMaximum.find(type);
  if (itr == this->Internal->NormalcyMaximum.end())
    {
    return VTK_DOUBLE_MIN;
    }
  return itr->second;
}

//-----------------------------------------------------------------------------
int vtkVgEventModel::Update(const vtkVgTimeStamp& timeStamp,
                            const vtkVgTimeStamp* referenceFrameTimeStamp/*=0*/)
{
  bool updateSpatial = this->ContourOperatorManager &&
                       (this->GetMTime() >
                        this->Internal->SpatialFilteringUpdateTime ||
                        this->ContourOperatorManager->GetMTime() >
                        this->Internal->SpatialFilteringUpdateTime);

  bool updateTemporal = this->TemporalFilters &&
                        (this->GetMTime() >
                         this->Internal->TemporalFilteringUpdateTime ||
                         this->TemporalFilters->GetMTime() >
                         this->Internal->TemporalFilteringUpdateTime);

  if (this->CurrentTimeStamp == timeStamp &&
      this->Internal->UpdateTime > this->GetMTime() &&
      !(updateSpatial || updateTemporal))
    {
    return VTK_OK;
    }

  this->CurrentTimeStamp = timeStamp;
  this->ReferenceTimeStamp = referenceFrameTimeStamp ? *referenceFrameTimeStamp
                             : vtkVgTimeStamp();

  this->Internal->UpdateTime.Modified();
  if (updateSpatial)
    {
    this->Internal->SpatialFilteringUpdateTime.Modified();
    }
  if (updateTemporal)
    {
    this->Internal->TemporalFilteringUpdateTime.Modified();
    }

  EventMapIterator eventIter;
  for (eventIter = this->Internal->EventIdMap.begin();
       eventIter != this->Internal->EventIdMap.end(); eventIter++)
    {
    vtkVgEventInfo& info = eventIter->second;

    // hide this event if it is not displayed
    if (!info.GetDisplayEvent())
      {
      continue;
      }

    if (updateTemporal)
      {
      this->UpdateTemporalFiltering(info);
      }

    if (updateSpatial)
      {
      this->UpdateSpatialFiltering(info);
      }
    }

  // Let the representation (if listening) know it needs to update
  //this->UpdateTime.ModifieDataRequestOn();
  this->InvokeEvent(vtkCommand::UpdateDataEvent);

  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::SetEventDisplayState(vtkIdType eventId, bool displayEvent)
{
  EventMapIterator eventIter = this->Internal->EventIdMap.find(eventId);
  if (eventIter != this->Internal->EventIdMap.end() &&
      eventIter->second.GetDisplayEvent() != displayEvent)
    {
    displayEvent ? eventIter->second.SetDisplayEventOn()
    : eventIter->second.SetDisplayEventOff();
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
unsigned long vtkVgEventModel::GetUpdateTime()
{
  return this->Internal->UpdateTime.GetMTime();
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::SetAllEventsDisplayState(bool state)
{
  for (EventMapIterator itr = this->Internal->EventIdMap.begin(),
       end = this->Internal->EventIdMap.end(); itr != end; ++itr)
    {
    if (state)
      {
      itr->second.SetDisplayEventOn();
      }
    else
      {
      itr->second.SetDisplayEventOff();
      }
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgEventModel::GetNextAvailableId()
{
  if (this->Internal->EventIdMap.empty())
    {
    return 0;
    }
  return (--this->Internal->EventIdMap.end())->first + 1;
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::UpdateTemporalFiltering(vtkVgEventInfo& info)
{
  vtkVgEvent* event = info.GetEvent();

  bool pass =
    this->TemporalFilters->EvaluateInterval(event->GetStartFrame(),
                                            event->GetEndFrame());

  pass ? info.SetPassesTemporalFiltersOn() : info.SetPassesTemporalFiltersOff();
}

//-----------------------------------------------------------------------------
void vtkVgEventModel::UpdateSpatialFiltering(vtkVgEventInfo& info)
{
  vtkVgEvent* event = info.GetEvent();

  // if no track (in which case we should really test the regions,
  // but not right now) or track isn't started, don't mark the event as
  // having failed filtering / selecting
  if (event->GetNumberOfTracks() == 0 || !event->GetTrack(0) ||
      !event->GetTrack(0)->IsStarted())
    {
    info.SetPassesSpatialFiltersOn();
    return;
    }

  if (event->IsTripEvent())
    {
    if (this->ContourOperatorManager->EvaluatePoint(
          event->GetTripEventPosition()))
      {
      info.SetPassesSpatialFiltersOn();
      return;
      }
    }
  else
    {
    vtkIdListCollection* idLists = event->GetFullEventIdCollection();
    for (int i = 0; i < idLists->GetNumberOfItems(); ++i)
      {
      // does any of the event "pass" the filters and selectors
      if (this->ContourOperatorManager->EvaluatePath(
            event->GetPoints(), idLists->GetItem(i)))
        {
        info.SetPassesSpatialFiltersOn();
        return;
        }
      }
    }

  info.SetPassesSpatialFiltersOff();
}
