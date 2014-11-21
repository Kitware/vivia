/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgActivityManager_h
#define __vtkVgActivityManager_h

#include "vtkObject.h"
#include "vtkSmartPointer.h" //to support picker object

// VisGUI includes.
#include "vtkVgTimeStamp.h"
#include "vtkVgTypeDefs.h"

// C++ includes.
#include <vector> // STL required.

#include <vgExport.h>

// Forward declarations.
class vtkVgActivity;
class vtkVgActivityTypeRegistry;
class vtkVgEvent;
class vtkVgEventModel;
class vtkVgIconManager;
class vtkVgTrack;

class vgActivityType;

class vtkActor;
class vtkPropPicker;
class vtkRenderer;

class VTKVG_MODELVIEW_EXPORT vtkVgActivityManager : public vtkObject
{
public:

  // Description:
  // Standard VTK functions.
  static vtkVgActivityManager* New();
  vtkTypeMacro(vtkVgActivityManager, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  void Initialize();
  void AddActivity(vtkVgActivity* activity);

  int GetNumberOfActivities();
  vtkVgActivity* GetActivity(int activityIndex);

  bool GetActivityDisplayState(int activityIndex);
  bool GetActivityFilteredDisplayState(int activityIndex);

  // Description:
  // Return whether a track is used by any activities
  bool IsTrackUsedByActivity(vtkVgTrack* track);

  // Description:
  // Return whether a event is used by any activities
  bool IsEventUsedByActivity(vtkIdType eventId);

  // Description:
  // Specify a renderer that individual activity actors will use.
  void SetRenderer(vtkRenderer* ren);
  vtkGetObjectMacro(Renderer, vtkRenderer);

  // Description:
  // Specify an icon manager to coordinate with.
  void SetIconManager(vtkVgIconManager* iconManager);
  vtkGetObjectMacro(IconManager, vtkVgIconManager);

  // Description:
  // Specify an event model to coordinate with.
  void SetEventModel(vtkVgEventModel* EventModel);
  vtkGetObjectMacro(EventModel, vtkVgEventModel);

  // Description:
  // Update all the activity actor(s). Generally called by the application layer.
  // Returns true if a new actor was added.
  bool UpdateActivityActors(vtkVgTimeStamp& timeStamp);

  // Description:
  // Method for controlling visibility of all events.  Note, Visibility is
  // different from the Display state.  For a particular event to be
  // rendered, it must be "displayed" and visibility must be on.
  void SetVisibility(int visibility);
  vtkGetMacro(Visibility, int);
  vtkBooleanMacro(Visibility, int);

  void TurnOnAllActivities()  { this->SetAllActivitiesDisplayState(true); }
  void TurnOffAllActivities() { this->SetAllActivitiesDisplayState(false); }

  void UpdateActivityDisplayStates();

  void InitializeAdjudication(vtkVgTrack* track);
  vtkVgActivity* GetNextAdjudicationActivity();
  void SetActivityState(vtkVgActivity* activity, bool state);

  void SetDisplayActivityType(int activityType, bool state);
  bool GetDisplayActivityType(int activityType);

  void SetOverlayOpacity(double opacity);
  vtkGetMacro(OverlayOpacity, double);

  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked activityId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account
  // whether the activity is currently visible or not.
  vtkIdType Pick(double renX, double renY, vtkRenderer* ren);

  // Description:
  // After a successful pick occurs, return the position of the pick (in
  // world coordinates).
  vtkGetVector3Macro(PickPosition, double);

  // Description:
  // Don't show activities of this type with a saliency below the threshold value.
  void SetSaliencyThreshold(int type, double threshold);
  double GetSaliencyThreshold(int type);

  void SetActivityTypeRegistry(vtkVgActivityTypeRegistry* registry);
  vtkGetObjectMacro(ActivityTypeRegistry, vtkVgActivityTypeRegistry);

  void InitializeTypeInfo();

  int GetNumberOfActivityTypes();

  const char* GetActivityName(int activityType);

  void UpdateActivityColors();

  bool ActivityIsFiltered(vtkVgActivity* a);

  // Description:
  // Given a track pointer return list of activities linked.
  void GetActivities(vtkVgTrack* track, std::vector<vtkVgActivity*>& activities);

  // Description:
  // Show all activities regardless of current frame.
  void SetShowFullVolume(bool show);

  // Description:
  // Additional amount of time to display activities past their end.
  void SetActivityExpirationOffset(const vtkVgTimeStamp& ts);

  vtkSetClampMacro(ColorMultiplier, double, 0.0, 1.0);
  vtkGetMacro(ColorMultiplier, double);

private:
  vtkVgActivityManager(const vtkVgActivityManager&); // Not implemented.
  void operator=(const vtkVgActivityManager&);  // Not implemented.

  // Description:
  // Constructor / Destructor.
  vtkVgActivityManager();
  ~vtkVgActivityManager();

  void SetActivityColors(vtkVgActivity* vgActivity);

  void SetAllActivitiesDisplayState(bool state);

  vtkVgActivityTypeRegistry* ActivityTypeRegistry;

  vtkVgEventModel* EventModel;
  vtkVgIconManager* IconManager;

  vtkRenderer* Renderer;

  // Internal data members to support picking
  vtkSmartPointer<vtkPropPicker> Picker;
  double PickPosition[3];

  int Visibility;

  double OverlayOpacity;
  double ColorMultiplier;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgActivityManager_h
