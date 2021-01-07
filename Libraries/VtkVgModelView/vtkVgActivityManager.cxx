// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgActivityManager.h"

// VTK includes.
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCollection.h>
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPropPicker.h>
#include <vtkRenderer.h>

#include "vgActivityType.h"

#include "vtkVgActivity.h"
#include "vtkVgActivityTypeRegistry.h"
#include "vtkVgEvent.h"
#include "vtkVgEventModel.h"
#include "vtkVgIcon.h"
#include "vtkVgIconManager.h"

#include "vtkVgLabeledRegion.h"

#include <vector>
#include <map>
#include <set>

vtkStandardNewMacro(vtkVgActivityManager);
vtkCxxSetObjectMacro(vtkVgActivityManager, IconManager, vtkVgIconManager);
vtkCxxSetObjectMacro(vtkVgActivityManager, EventModel, vtkVgEventModel);
vtkCxxSetObjectMacro(vtkVgActivityManager, Renderer, vtkRenderer);
vtkCxxSetObjectMacro(vtkVgActivityManager, ActivityTypeRegistry,
                     vtkVgActivityTypeRegistry);

struct ActivityInfo
{
  vtkVgActivity* Activity;
  bool Active;
  bool DisplayActivity;

  ActivityInfo()
    {
    this->Activity = 0;
    this->Active = false;
    this->DisplayActivity = false;
    }

  ~ActivityInfo()
    {
    if (this->Activity)
      {
      this->Activity->UnRegister(NULL);
      this->Activity = 0;
      }
    }

  // Function ensures proper reference counting.
  void SetActivity(vtkVgActivity* activity)
    {
    // Proceed only if the incoming event is not
    // this->Event.
    if (this->Activity != activity)
      {
      // Before assigning a new one store the last one
      // so that we can decrement the ref count later.
      vtkVgActivity* temp = this->Activity;
      this->Activity = activity;
      // Increment the ref count.
      if (this->Activity != NULL)
        {
        this->Activity->Register(NULL);
        }
      if (temp != NULL)
        {
        // Decrement the ref count.
        temp->UnRegister(NULL);
        }
      }
    }

  ActivityInfo(const ActivityInfo& fromActivityInfo)
    {
    this->Activity = 0;
    this->SetActivity(fromActivityInfo.Activity);
    this->Active = fromActivityInfo.Active;
    this->DisplayActivity = fromActivityInfo.DisplayActivity;
    }

  ActivityInfo& operator=(const ActivityInfo& fromActivityInfo)
    {
    this->Activity = 0;
    this->SetActivity(fromActivityInfo.Activity);
    this->Active = fromActivityInfo.Active;
    this->DisplayActivity = fromActivityInfo.DisplayActivity;
    return *this;
    }
};

//----------------------------------------------------------------------------
class vtkVgActivityManager::vtkInternal
{
public:
  std::vector<ActivityInfo> Activities;

  std::vector<bool> DisplayActivityType;
  std::vector<double> SaliencyThreshold;

  std::multimap<vtkVgTrack*, vtkVgActivity*> TrackToActivityMap;

  typedef std::multimap<vtkVgTrack*, vtkVgActivity*>::const_iterator TrackActivityConstIterator;

  std::multimap<vtkVgTrack*, vtkVgActivity*>::const_iterator AdjudicationIterator;

  vtkInternal(vtkVgActivityManager* activityManager) : ActivityManager(activityManager)
    {
    }

  ~vtkInternal()
    {
    }

  vtkVgActivityManager* ActivityManager;
};

//-----------------------------------------------------------------------------
vtkVgActivityManager::vtkVgActivityManager()
{
  this->IconManager = 0;
  this->EventModel = 0;
  this->Renderer = 0;
  this->Internal = new vtkInternal(this);
  this->Visibility = true; // default to visible
  this->OverlayOpacity = 0.3;
  this->ColorMultiplier = 1.0;

  // Support picking
  this->Picker = vtkSmartPointer<vtkPropPicker>::New();
  this->Picker->PickFromListOn();
  this->PickPosition[0] = this->PickPosition[1] = this->PickPosition[2] = 0.0;

  this->ActivityTypeRegistry = vtkVgActivityTypeRegistry::New();
}

//-----------------------------------------------------------------------------
vtkVgActivityManager::~vtkVgActivityManager()
{
  this->SetIconManager(0);
  this->SetEventModel(0);
  this->SetRenderer(0);

  this->ActivityTypeRegistry->Delete();
  delete this->Internal;
}

//-----------------------------------------------------------------------------
bool vtkVgActivityManager::IsTrackUsedByActivity(vtkVgTrack* track)
{
  return this->Internal->TrackToActivityMap.find(track) !=
         this->Internal->TrackToActivityMap.end();
}

//-----------------------------------------------------------------------------
bool vtkVgActivityManager::IsEventUsedByActivity(vtkIdType eventId)
{
  for (size_t i = 0, size = this->Internal->Activities.size(); i < size; ++i)
    {
    vtkVgActivity* activity = this->Internal->Activities[i].Activity;
    for (unsigned int i = 0, size = activity->GetNumberOfEvents(); i < size;
         ++i)
      {
      if (activity->GetEvent(i)->GetId() == eventId)
        {
        return true;
        }
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
int vtkVgActivityManager::GetNumberOfActivities()
{
  return static_cast<int>(this->Internal->Activities.size());
}

//-----------------------------------------------------------------------------
vtkVgActivity* vtkVgActivityManager::GetActivity(int activityIndex)
{
  if (activityIndex < 0 || activityIndex >=
      static_cast<int>(this->Internal->Activities.size()))
    {
    vtkErrorMacro("Invalid index: " << activityIndex);
    return 0;
    }
  return this->Internal->Activities[activityIndex].Activity;
}

//-----------------------------------------------------------------------------
bool vtkVgActivityManager::GetActivityDisplayState(int activityIndex)
{
  if (activityIndex < 0 || activityIndex >=
      static_cast<int>(this->Internal->Activities.size()))
    {
    vtkErrorMacro("Invalid index: " << activityIndex);
    return false;
    }
  return this->Internal->Activities[activityIndex].DisplayActivity;
}

//-----------------------------------------------------------------------------
bool vtkVgActivityManager::GetActivityFilteredDisplayState(int activityIndex)
{
  if (activityIndex < 0 || activityIndex >=
      static_cast<int>(this->Internal->Activities.size()))
    {
    vtkErrorMacro("Invalid index: " << activityIndex);
    return false;
    }

  assert(this->EventModel &&
         "Event model is needed to support activity spatial filtering");

  // If at least one supporting event passes the spatial filter, the activity
  // passes as well.
  vtkVgActivity* a = this->Internal->Activities[activityIndex].Activity;
  for (int i = 0, end = a->GetNumberOfEvents(); i < end; ++i)
    {
    vtkIdType id = a->GetEvent(i)->GetId();
    vtkVgEventInfo info = this->EventModel->GetEventInfo(id);
    if (info.GetPassesFilters())
      {
      return true;
      }
    }

  // All supporting events are filtered out.
  return false;
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::Initialize()
{
  this->Internal->Activities.clear();
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::AddActivity(vtkVgActivity* vgActivity)
{
  ActivityInfo activityInfo;
  vgActivity->SetRenderer(this->Renderer);
  vgActivity->SetIconManager(this->IconManager);
  vgActivity->SetOverlayOpacity(this->OverlayOpacity);
  activityInfo.SetActivity(vgActivity);
  activityInfo.DisplayActivity = true;
  this->Internal->Activities.push_back(activityInfo);

  this->SetActivityColors(vgActivity);

  // add map from track to activity for each of the tracks in this activity
  std::set<vtkVgTrack*> tempSet;  // quick "hack" to prevent adding the
                                  // activity multiple times for a track
  for (unsigned int i = 0; i < vgActivity->GetNumberOfEvents(); i++)
    {
    tempSet.clear();
    vtkVgEvent* theEvent = vgActivity->GetEvent(i);
    for (unsigned int j = 0; j < theEvent->GetNumberOfTracks(); j++)
      {
      vtkVgTrack* track = theEvent->GetTrack(j);
      if (tempSet.count(track) == 0)
        {
        this->Internal->TrackToActivityMap.insert(
          std::make_pair(track, vgActivity));
        tempSet.insert(track);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::UpdateActivityDisplayStates()
{
  std::vector<ActivityInfo>::iterator activityIter;
  for (activityIter = this->Internal->Activities.begin();
       activityIter != this->Internal->Activities.end(); activityIter++)
    {
    activityIter->DisplayActivity =
      this->Internal->DisplayActivityType[activityIter->Activity->GetType()];
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::InitializeAdjudication(vtkVgTrack* track)
{
  this->Internal->AdjudicationIterator =
    this->Internal->TrackToActivityMap.lower_bound(track);
}

//-----------------------------------------------------------------------------
vtkVgActivity* vtkVgActivityManager::GetNextAdjudicationActivity()
{
  vtkVgTrack* track = this->Internal->AdjudicationIterator->first;
  vtkVgActivity* activity = this->Internal->AdjudicationIterator->second;
  this->Internal->AdjudicationIterator++;
  if (this->Internal->AdjudicationIterator ==
      this->Internal->TrackToActivityMap.upper_bound(track))
    {
    this->Internal->AdjudicationIterator =
      this->Internal->TrackToActivityMap.lower_bound(track);
    }
  return activity;
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetActivityState(vtkVgActivity* activity, bool state)
{
  std::vector<ActivityInfo>::iterator activityIter;
  for (activityIter = this->Internal->Activities.begin();
       activityIter != this->Internal->Activities.end(); activityIter++)
    {
    if (activityIter->Activity == activity)
      {
      activityIter->DisplayActivity = state;
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetAllActivitiesDisplayState(bool state)
{
  std::vector<ActivityInfo>::iterator activityIter;
  for (activityIter = this->Internal->Activities.begin();
       activityIter != this->Internal->Activities.end(); activityIter++)
    {
    activityIter->DisplayActivity = state;
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetDisplayActivityType(int activityType, bool state)
{
  this->Internal->DisplayActivityType[activityType] = state;
}

//-----------------------------------------------------------------------------
bool vtkVgActivityManager::GetDisplayActivityType(int activityType)
{
  return this->Internal->DisplayActivityType[activityType];
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetVisibility(int visibility)
{
  if (this->Visibility == visibility)
    {
    return;
    }
  this->Visibility = visibility;

  // even if setting visibility on, the activity actor may not be setup since
  // only do so when Visibility is on in call to SetCurrentDisplayFrame.  Thus
  // if turning on, should call UpdateActivityActors after this call
  bool visible = visibility ? true : false;
  std::vector<ActivityInfo>::iterator activityIter;
  for (activityIter = this->Internal->Activities.begin();
       activityIter != this->Internal->Activities.end(); activityIter++)
    {
    activityIter->Activity->SetVisibility(
      activityIter->DisplayActivity ? visible : false);
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetOverlayOpacity(double opacity)
{
  this->OverlayOpacity = opacity;

  std::vector<ActivityInfo>::iterator iter;
  for (iter = this->Internal->Activities.begin();
       iter != this->Internal->Activities.end(); ++iter)
    {
    iter->Activity->SetOverlayOpacity(opacity);
    }
}

//-----------------------------------------------------------------------------
bool vtkVgActivityManager::UpdateActivityActors(vtkVgTimeStamp& timeStamp)
{
  if (!this->Visibility)
    {
    return false;
    }

  bool actorAdded = false;

  std::vector<ActivityInfo>::iterator activityIter;
  int index = 0;
  for (activityIter = this->Internal->Activities.begin();
       activityIter != this->Internal->Activities.end(); activityIter++, ++index)
    {
    if (activityIter->DisplayActivity  &&
        !this->ActivityIsFiltered(activityIter->Activity) &&
        this->GetActivityFilteredDisplayState(index))
      {
      if (activityIter->Activity->SetCurrentDisplayFrame(timeStamp))
        {
        actorAdded = true;
        }
      activityIter->Activity->SetVisibility(true);   // making sure
      }
    else
      {
      activityIter->Activity->SetVisibility(false);
      }
    }
  return actorAdded;
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgActivityManager::
Pick(double renX, double renY, vtkRenderer* ren)
{
  if (!this->Visibility)
    {
    return -1;
    }

  this->Picker->InitializePickList();

  // add actors to pick list
  std::vector<ActivityInfo>::iterator activityIter;
  for (activityIter = this->Internal->Activities.begin();
       activityIter != this->Internal->Activities.end(); activityIter++)
    {
    if (activityIter->DisplayActivity)
      {
      vtkVgLabeledRegion* activityActor = activityIter->Activity->GetActor();
      if (activityActor)
        {
        this->Picker->AddPickList(activityActor->GetFrameActor());
        this->Picker->AddPickList(activityActor->GetImageActor());
        }
      }
    }

  if (this->Picker->Pick(renX, renY, 0.0, ren))
    {
    vtkProp* pickedActor = this->Picker->GetViewProp();
    if (!pickedActor)
      {
      return -1;
      }

    // look up the picked actor
    for (size_t i = 0, end = this->Internal->Activities.size(); i < end; ++i)
      {
      const ActivityInfo& info = this->Internal->Activities[i];
      if (info.DisplayActivity)
        {
        vtkVgLabeledRegion* activityActor = info.Activity->GetActor();
        if (activityActor &&
            (pickedActor == activityActor->GetFrameActor() ||
             pickedActor == activityActor->GetImageActor()))
          {
          return static_cast<vtkIdType>(i);
          }
        }
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
bool vtkVgActivityManager::ActivityIsFiltered(vtkVgActivity* a)
{
  return !this->Internal->DisplayActivityType[a->GetType()] ||
         a->GetSaliency() < this->Internal->SaliencyThreshold[a->GetType()];
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::GetActivities(vtkVgTrack* track,
  std::vector<vtkVgActivity*>& activities)
{
  if (!track)
    {
    vtkErrorMacro("ERROR: Invalid track pointer\n");
    return;
    }

  std::pair < vtkInternal::TrackActivityConstIterator,
      vtkInternal::TrackActivityConstIterator > ret;

  ret = this->Internal->TrackToActivityMap.equal_range(track);

  vtkInternal::TrackActivityConstIterator constItr;
  for (constItr = ret.first; constItr != ret.second; ++constItr)
    {
    activities.push_back(constItr->second);
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::InitializeTypeInfo()
{
  int numTypes = this->ActivityTypeRegistry->GetNumberOfTypes();
  this->Internal->DisplayActivityType.resize(numTypes);
  this->Internal->SaliencyThreshold.resize(numTypes);

  std::fill(this->Internal->DisplayActivityType.begin(),
            this->Internal->DisplayActivityType.end(), true);
  std::fill(this->Internal->SaliencyThreshold.begin(),
            this->Internal->SaliencyThreshold.end(), 0.0);
}

//-----------------------------------------------------------------------------
int vtkVgActivityManager::GetNumberOfActivityTypes()
{
  return this->ActivityTypeRegistry->GetNumberOfTypes();
}

//-----------------------------------------------------------------------------
const char* vtkVgActivityManager::GetActivityName(int activityType)
{
  return this->ActivityTypeRegistry->GetType(activityType).GetName();
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetSaliencyThreshold(int type, double threshold)
{
  this->Internal->SaliencyThreshold[type] = threshold;
}

//-----------------------------------------------------------------------------
double vtkVgActivityManager::GetSaliencyThreshold(int type)
{
  return this->Internal->SaliencyThreshold[type];
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetActivityColors(vtkVgActivity* vgActivity)
{
  int type = vgActivity->GetType();
  const vgActivityType& at = this->ActivityTypeRegistry->GetType(type);

  vgActivity->SetMultiColorEnabled(false);

  if (at.GetUseRandomColors())
    {
    // Color the activity "randomly" based on its id.
    vtkMath::RandomSeed(vgActivity->GetId());
    double rgb[] =
      {
      vtkMath::Random(0.2, 1.0)* this->ColorMultiplier,
      vtkMath::Random(0.2, 1.0)* this->ColorMultiplier,
      vtkMath::Random(0.2, 1.0)* this->ColorMultiplier
      };
    vgActivity->SetColor(rgb);
    }
  else
    {
    // Color using the configured color values.
    double rgb[3];
    at.GetColor(rgb[0], rgb[1], rgb[2]);
    rgb[0] *= this->ColorMultiplier;
    rgb[1] *= this->ColorMultiplier;
    rgb[2] *= this->ColorMultiplier;
    vgActivity->SetColor(rgb);

    if (at.GetHasSecondaryColor())
      {
      double rgb2[3];
      at.GetSecondaryColor(rgb2[0], rgb2[1], rgb2[2]);
      rgb2[0] *= this->ColorMultiplier;
      rgb2[1] *= this->ColorMultiplier;
      rgb2[2] *= this->ColorMultiplier;
      vgActivity->SetSecondaryColor(rgb2);
      vgActivity->SetMultiColorEnabled(true);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::UpdateActivityColors()
{
  // Update the colors of all activities.
  for (std::vector<ActivityInfo>::iterator iter =
         this->Internal->Activities.begin();
       iter != this->Internal->Activities.end(); ++iter)
    {
    this->SetActivityColors(iter->Activity);
    iter->Activity->ApplyColors();
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetShowFullVolume(bool show)
{
  for (std::vector<ActivityInfo>::iterator iter =
         this->Internal->Activities.begin();
       iter != this->Internal->Activities.end(); ++iter)
    {
    iter->Activity->SetShowAlways(show);
    }
}

//-----------------------------------------------------------------------------
void vtkVgActivityManager::SetActivityExpirationOffset(const vtkVgTimeStamp& ts)
{
  for (std::vector<ActivityInfo>::iterator iter =
         this->Internal->Activities.begin();
       iter != this->Internal->Activities.end(); ++iter)
    {
    iter->Activity->SetExpirationOffset(ts);
    }
}
