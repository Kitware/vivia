// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgEventModel_h
#define __vtkVgEventModel_h

#include "vtkVgModelBase.h"

#include "vtkVgEventInfo.h"

#include <vector>

#include <vgExport.h>

#include <vtkVgTrack.h>

class vgEventType;

class vtkIdList;
class vtkPoints;
class vtkVgEvent;
class vtkVgEventBase;
class vtkVgEventTypeRegistry;
class vtkVgTrack;
class vtkVgTrackModel;

struct EventLink
{
  vtkIdType Source;
  vtkIdType Destination;
  double Probability;
};

class VTKVG_MODELVIEW_EXPORT vtkVgEventModel : public vtkVgModelBase
{
public:
  // Description:
  // Easy to use.
  vtkVgClassMacro(vtkVgEventModel);

  vtkTypeMacro(vtkVgEventModel, vtkVgModelBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgEventModel* New();

  enum EventIds
    {
    EventRemoved = 1000000
    };

  void Initialize();

  // Description:
  // Set/Get whether or not to have event share the same set of region points.
  // Not required, but if shared, much easier to use one actor. default = true.
  // Disabling (commenting out) about until the vtkVgEventRegionRepresentation
  // is corrected to handle this being "false", or there is some other need/use
  // for this not to be "true".
  //vtkSetMacro(UseSharedRegionPoints, bool);
  //vtkGetMacro(UseSharedRegionPoints, bool);

  // Description:
  // Set/Get the points to be shared by events CREATED when doing AddEvent
  // from a EventBase.  Those that are already vtkVgEvents are not affected.
  void SetSharedRegionPoints(vtkPoints* points);
  vtkGetObjectMacro(SharedRegionPoints, vtkPoints);

  vtkVgEvent* AddEvent(vtkVgEventBase* vgEventBase);

  void AddEventLink(const EventLink& link);
  void GetEventLink(int index, EventLink& link);
  int  GetNumberOfEventLinks();

  vtkVgEvent* CreateAndAddEvent(int type, vtkIdList* trackIds);
  vtkVgEvent* CloneEvent(vtkIdType eventId);

  bool RemoveEvent(vtkIdType eventId);

  vtkVgEvent* GetEvent(vtkIdType eventId);
  vtkVgEventInfo GetEventInfo(vtkIdType eventId);

  vtkVgTrackDisplayData GetTrackDisplayData(vtkVgEvent* event, int trackIndex);

  // Description:
  // Given a track id return a list of events associated.
  void GetEvents(vtkIdType trackId, std::vector<vtkVgEvent*>& events);

  // Description:
  // Given a track pointer return a list of events associated.
  void GetEvents(vtkVgTrack* track, std::vector<vtkVgEvent*>& events);

  double GetNormalcyMinForType(int type);
  double GetNormalcyMaxForType(int type);

  void InitEventTraversal();
  vtkVgEventInfo GetNextEvent();
  vtkVgEventInfo GetNextDisplayedEvent();

  vtkIdType GetNumberOfEvents();

  virtual int Update(const vtkVgTimeStamp& timeStamp,
                     const vtkVgTimeStamp* referenceFrameTimeStamp);
  using Superclass::Update;

  // Description:
  // Specify a track manager to coordinate with.
  void SetTrackModel(vtkVgTrackModel* TrackModel);
  vtkGetObjectMacro(TrackModel, vtkVgTrackModel);

  void SetEventExpirationOffset(const vtkVgTimeStamp& offset);

  vtkSetMacro(ShowEventsBeforeStart, bool);
  vtkGetMacro(ShowEventsBeforeStart, bool);
  vtkBooleanMacro(ShowEventsBeforeStart, bool);

  vtkSetMacro(ShowEventsAfterExpiration, bool);
  vtkGetMacro(ShowEventsAfterExpiration, bool);
  vtkBooleanMacro(ShowEventsAfterExpiration, bool);

  vtkSetMacro(ShowEventsUntilSupportingTracksExpire, bool);
  vtkGetMacro(ShowEventsUntilSupportingTracksExpire, bool);
  vtkBooleanMacro(ShowEventsUntilSupportingTracksExpire, bool);

  vtkSetMacro(UseTrackGroups, bool);
  vtkGetMacro(UseTrackGroups, bool);
  vtkBooleanMacro(UseTrackGroups, bool);

  // Description:
  // Set/Get whether the event should be displayed.
  void SetEventDisplayState(vtkIdType eventId, bool displayEvent);

  // Description:
  // Return the MTime for the last Update (that did something).
  virtual unsigned long GetUpdateTime();

  // Description
  // Set/Get whether all events are to be displayed.  If true, the event model
  // will turn off the events disregarding any other parameters and vice versa.
  void SetAllEventsDisplayState(bool state);

  void TurnOnAllEvents()  { this->SetAllEventsDisplayState(true); }
  void TurnOffAllEvents() { this->SetAllEventsDisplayState(false); }

  vtkIdType GetNextAvailableId();

private:
  vtkVgEventModel(const vtkVgEventModel&); // Not implemented.
  void operator=(const vtkVgEventModel&);  // Not implemented.

  void UpdateTemporalFiltering(vtkVgEventInfo& info);
  void UpdateSpatialFiltering(vtkVgEventInfo& info);

  // Description:
  // Constructor / Destructor.
  vtkVgEventModel();
  ~vtkVgEventModel();

  vtkVgTrackModel* TrackModel;

  bool ShowEventsBeforeStart;
  bool ShowEventsAfterExpiration;
  bool ShowEventsUntilSupportingTracksExpire;

  bool UseTrackGroups;

  vtkVgTimeStamp ReferenceTimeStamp;
  vtkVgTimeStamp EventExpirationOffset;

  bool UseSharedRegionPoints;
  vtkPoints* SharedRegionPoints;

  struct vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgEventModel_h
