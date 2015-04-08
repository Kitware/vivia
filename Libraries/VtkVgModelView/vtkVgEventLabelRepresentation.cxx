/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgEventLabelRepresentation.h"

#include <vtkActor2D.h>
#include <vtkIdListCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkProperty2D.h>
#include <vtkPropPicker.h>
#include <vtkTextProperty.h>
#include <vtkTimeStamp.h>

#include <vgEventType.h>
#include <vgUtil.h>

#include <vtkVgAnnotationActor.h>
#include <vtkVgEvent.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgTrack.h>
#include <vtkVgUtil.h>

#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgPickData.h"

#include <map>
#include <sstream>
#include <algorithm>

vtkStandardNewMacro(vtkVgEventLabelRepresentation);

typedef std::map<vtkIdType, vtkVgAnnotationActor*> ActorMap;

//----------------------------------------------------------------------------
struct vtkVgEventLabelRepresentation::vtkInternal
{
  ~vtkInternal();

  ActorMap Actors;
  vtkSmartPointer<vtkPropPicker> Picker;
  vtkSmartPointer<vtkMatrix4x4> InvRepresentationMatrix;
};

vtkVgEventLabelRepresentation::vtkInternal::~vtkInternal()
{
  for (ActorMap::iterator iter = this->Actors.begin(), end = this->Actors.end();
       iter != end; ++iter)
    {
    if (iter->second)
      {
      iter->second->Delete();
      }
    }
}

//-----------------------------------------------------------------------------
vtkVgEventLabelRepresentation::vtkVgEventLabelRepresentation()
{
  this->Visible = 1;
  this->ShowId = true;
  this->ShowClassifiers = 1;
  this->ShowProbability = false;
  this->ShowNote = false;
  this->LabelPrefix = 0;
  this->LocationSourceMode = LSM_Track;
  this->Internal = new vtkInternal;

  this->Internal->Picker = vtkSmartPointer<vtkPropPicker>::New();
  this->Internal->Picker->PickFromListOn();

  this->Internal->InvRepresentationMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();

  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();
}

//-----------------------------------------------------------------------------
vtkVgEventLabelRepresentation::~vtkVgEventLabelRepresentation()
{
  this->SetLabelPrefix(0);
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgEventLabelRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventLabelRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventLabelRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgEventLabelRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventLabelRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventLabelRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgEventLabelRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgEventLabelRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vtkVgEventLabelRepresentation::SetVisible(int flag)
{
  if (flag == this->Visible)
    {
    return;
    }

  this->Visible = flag;
  this->ActivePropCollection->InitTraversal();
  while (vtkProp* prop = this->ActivePropCollection->GetNextProp())
    {
    prop->SetVisibility(flag);
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
bool CompareClassifiers(const vtkVgEventFilter::ScoredClassifier& a,
                        const vtkVgEventFilter::ScoredClassifier& b)
{
  return a.second > b.second;
}

//-----------------------------------------------------------------------------
bool vtkVgEventLabelRepresentation::GetLabelPosition(vtkVgEvent* event,
    double labelPosition[3])
{
  // determine location for label, returning false if none available

  bool havePosition = false;

  // 1st see if Track preferred mode, and can get from track
  if (this->LocationSourceMode == LSM_Track ||
      this->LocationSourceMode == LSM_TrackThenRegion)
    {
    havePosition = this->GetPositionBasedOnTrack(event, labelPosition);
    }

  // then, if no position yet, see if mode includes position from region
  if (!havePosition && this->LocationSourceMode != LSM_Track)
    {
    havePosition = this->GetPositionBasedOnRegion(event, labelPosition);
    }

  // finally, if no position yet, see if considered region 1st THEN track
  if (!havePosition && this->LocationSourceMode == LSM_RegionThenTrack)
    {
    havePosition = this->GetPositionBasedOnTrack(event, labelPosition);
    }

  return havePosition;
}

//-----------------------------------------------------------------------------
bool vtkVgEventLabelRepresentation::GetPositionBasedOnRegion(vtkVgEvent* event,
  double labelPosition[3])
{
  vtkVgTimeStamp now = this->EventModel->GetCurrentTimeStamp();
  if (!event->IsTimeWithinExtents(now))
    {
    // Display time not within event's time range; nothing to do
    return false;
    }

  vtkIdType npts, *pts;
  event->GetClosestDisplayRegion(now, npts, pts);

  if (npts > 0)
    {
    vtkPoints* points = event->GetRegionPoints();

    // Loop through points, transforming to get the minimum "y" (v) point. If
    // there are several points close to the minimum y, average their min and
    // max x values together.
    // TODO: This is identical to the code used to position to track labels,
    //       factor this out into a common base?
    double minX = VTK_DOUBLE_MAX;
    double maxX = VTK_DOUBLE_MIN;
    double minY = VTK_DOUBLE_MAX;
    for (int i = 0; i < npts; ++i)
      {
      double point[3];
      points->GetPoint(pts[i], point);
      const vgPoint2d tpoint =
        vtkVgApplyHomography(point, this->RepresentationMatrix);
      if (fabs(tpoint.Y - minY) < 1.0) // Expect always true for first point
        {
        vgExpandBoundaries(minX, maxX, tpoint.X);
        vgExpandLowerBoundary(minY, tpoint.Y);
        }
      else if (tpoint.Y < minY)
        {
        // Found a new distinct min point, so reset x limits
        minX = maxX = tpoint.X;
        minY = tpoint.Y;
        }
      }

    // Now that we have a good anchor point in world space, transform it back
    // to model space before setting the actor position.
    const vgPoint2d point(0.5 * (minX + maxX), minY);
    vtkVgApplyHomography(point, this->Internal->InvRepresentationMatrix,
                         labelPosition);
    labelPosition[2] = 0.0;
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgEventLabelRepresentation::GetPositionBasedOnTrack(vtkVgEvent* event,
  double labelPosition[3])
{
  // position the annotation at the head of the first track, if there is one
  vtkVgTrackDisplayData tdd = this->EventModel->GetTrackDisplayData(event, 0);
  if (tdd.NumIds > 0)   // if at least one id, we know we have a track
    {
    vtkIdType headId = tdd.IdsStart[tdd.NumIds - 1];
    event->GetTrack(0)->GetPoints()->GetPoint(headId, labelPosition);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkVgEventLabelRepresentation::ShowEventAnnotation(vtkVgEvent* event,
  double labelPosition[3], bool rebuild)
{
  vtkVgAnnotationActor*& actor = this->Internal->Actors[event->GetId()];

  // @TODO: Maintain a pool of actors that can be reused in order to
  // avoid some of this reallocation.
  if (!actor)
    {
    actor = vtkVgAnnotationActor::New();
    actor->SetClampToViewport(false);
    actor->SetOffset(0, -15);
    actor->AutoCenterXOn();

    this->NewPropCollection->AddItem(actor);
    this->ActivePropCollection->AddItem(actor);
    rebuild = true;
    }

  actor->SetUserMatrix(this->RepresentationMatrix);
  actor->SetPosition(labelPosition);

  if (rebuild)
    {
    std::ostringstream ostr;

    if (this->LabelPrefix)
      {
      ostr << this->LabelPrefix;
      }

    if (this->ShowId)
      {
      ostr << 'E' << event->GetId();
      }

    if (this->ShowClassifiers)
      {
      // Get active classifiers, and sort by score
      std::vector<vtkVgEventFilter::ScoredClassifier> classifiers =
        this->EventFilter->GetActiveClassifiers(event);
      std::sort(classifiers.begin(), classifiers.end(), CompareClassifiers);

      // Show the top classifiers
      for (unsigned n = 0; n < classifiers.size() && n < this->ShowClassifiers; ++n)
        {
        if (this->ShowId || n > 0)
          {
          ostr << '\n';
          }

        int idx = this->EventTypeRegistry->GetTypeIndex(classifiers[n].first);
        if (idx >= 0)
          {
          const vgEventType& type = this->EventTypeRegistry->GetType(idx);

          if (n == 0)
            {
            double bg[3];
            double fg[3];
            type.GetLabelBackgroundColor(bg[0], bg[1], bg[2]);
            type.GetLabelForegroundColor(fg[0], fg[1], fg[2]);

            actor->GetFrameProperty()->SetColor(bg[0], bg[1], bg[2]);
            actor->GetTextProperty()->SetColor(fg[0], fg[1], fg[2]);
            }

          ostr << type.GetName();
          }
        else
          {
          ostr << classifiers[n].first;
          }

        // Add probability score, if requested
        if (this->ShowProbability)
          {
          ostr.setf(ios::fixed, ios::floatfield);
          ostr.precision(3);
          ostr << " (" << event->GetProbability(classifiers[n].first) << ")";
          }
        }
      }

  if (this->ShowNote)
    {
    const char* const note = event->GetNote();
    if (note && *note)
      {
      ostr << '\n' << note;
      }
    }

    actor->SetText(ostr.str().c_str());
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventLabelRepresentation::HideEventAnnotation(vtkVgEvent* event)
{
  ActorMap::iterator iter = this->Internal->Actors.find(event->GetId());
  if (iter != this->Internal->Actors.end() && iter->second)
    {
    this->ExpirePropCollection->AddItem(iter->second);

    // linear search here could get expensive...
    this->ActivePropCollection->RemoveItem(iter->second);

    iter->second->Delete();
    iter->second = 0;
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventLabelRepresentation::Update()
{
  if (!this->GetVisible())
    {
    return;
    }

  bool forceRebuild = this->EventFilter->GetMTime() > this->UpdateTime ||
                      this->EventTypeRegistry->GetMTime() > this->UpdateTime ||
                      this->GetMTime() > this->UpdateTime;

  if (!forceRebuild && this->EventModel->GetMTime() < this->UpdateTime &&
      this->EventModel->GetUpdateTime() < this->UpdateTime)
    {
    return;
    }

  if (this->Internal->InvRepresentationMatrix->GetMTime() <
      this->RepresentationMatrix->GetMTime())
    {
    vtkMatrix4x4::Invert(this->RepresentationMatrix,
                         this->Internal->InvRepresentationMatrix);
    }

  double labelPosition[3];
  vtkVgEventInfo info;
  this->EventModel->InitEventTraversal();
  while ((info = this->EventModel->GetNextEvent()).GetEvent())
    {
    vtkVgEvent* event = info.GetEvent();
    if (info.GetDisplayEvent() &&
        info.GetPassesFilters() &&
        this->GetLabelPosition(event, labelPosition) &&
        this->EventFilter->GetBestClassifier(event) != -1)
      {
      this->ShowEventAnnotation(event, labelPosition, forceRebuild);
      }
    else
      {
      this->HideEventAnnotation(event);
      }
    }

  this->UpdateTime.Modified();
}


//-----------------------------------------------------------------------------
void vtkVgEventLabelRepresentation::EventRemoved(
  vtkObject* vtkNotUsed(caller),
  unsigned long vtkNotUsed(invokedEventId),
  void* theEvent)
{
  this->HideEventAnnotation(static_cast<vtkVgEvent*>(theEvent));
  this->Modified();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgEventLabelRepresentation::Pick(double renX,
                                              double renY,
                                              vtkRenderer* ren,
                                              vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;

  if (!this->GetVisible())
    {
    return -1;
    }

  // Set up the pick list and add the actors to it
  this->Internal->Picker->InitializePickList();

  for (ActorMap::iterator iter = this->Internal->Actors.begin(),
       end = this->Internal->Actors.end();
       iter != end; ++iter)
    {
    if (iter->second)
      {
      this->Internal->Picker->AddPickList(iter->second->GetFrameActor());
      }
    }

  if (this->Internal->Picker->Pick(renX, renY, 0.0, ren))
    {
    vtkProp* pickedActor = this->Internal->Picker->GetViewProp();
    if (!pickedActor)
      {
      return -1;
      }

    // Look up the picked actor
    for (ActorMap::iterator iter = this->Internal->Actors.begin(),
         end = this->Internal->Actors.end();
         iter != end; ++iter)
      {
      if (iter->second && pickedActor == iter->second->GetFrameActor())
        {
        pickType = vtkVgPickData::PickedEvent;
        return iter->first;
        }
      }
    }

  return -1;
}
