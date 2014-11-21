/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTrackLabelRepresentation.h"

#include <vtkActor2D.h>
#include <vtkIdListCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkProperty2D.h>
#include <vtkPropPicker.h>
#include <vtkTextProperty.h>
#include <vtkTimeStamp.h>
#include <vtkVgUtil.h>

#include <vgUtil.h>

#include <vtkVgAnnotationActor.h>
#include <vtkVgTrack.h>

#include "vtkVgPickData.h"
#include "vtkVgTrackFilter.h"
#include "vtkVgTrackModel.h"

#include <map>
#include <sstream>

vtkStandardNewMacro(vtkVgTrackLabelRepresentation);

typedef std::map<vtkIdType, vtkVgAnnotationActor*> ActorMap;

//----------------------------------------------------------------------------
struct vtkVgTrackLabelRepresentation::vtkInternal
{
  ~vtkInternal();

  struct TrackInfoItem
    {
    double BackgroundColor[3];
    double ForegroundColor[3];
    };

  ActorMap Actors;
  std::map<int, TrackInfoItem> TrackInfo;

  vtkSmartPointer<vtkPropPicker> Picker;

  vtkSmartPointer<vtkMatrix4x4> InvRepresentationMatrix;

  typedef std::map<int, TrackInfoItem>::iterator       TrackInfoIterator;
  typedef std::map<int, TrackInfoItem>::const_iterator TrackInfoConstIterator;
};

vtkVgTrackLabelRepresentation::vtkInternal::~vtkInternal()
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
vtkVgTrackLabelRepresentation::vtkVgTrackLabelRepresentation()
{
  this->Visible = 1;
  this->ShowName = true;
  this->ShowProbability = false;
  this->LabelPrefix = 0;
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
vtkVgTrackLabelRepresentation::~vtkVgTrackLabelRepresentation()
{
  this->SetLabelPrefix(0);
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgTrackLabelRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkVgTrackLabelRepresentation::SetTrackTypeColors(
  const int id, const double bcolor[3]/*=0*/,
  const double fcolor[3]/*=0*/)
{
  vtkInternal::TrackInfoItem ei;

  if (bcolor)
    {
    ei.BackgroundColor[0] = bcolor[0];
    ei.BackgroundColor[1] = bcolor[1];
    ei.BackgroundColor[2] = bcolor[2];
    }
  else
    {
    ei.BackgroundColor[0] = 0.0;
    ei.BackgroundColor[1] = 0.4;
    ei.BackgroundColor[2] = 0.0;
    }

  if (fcolor)
    {
    ei.ForegroundColor[0] = fcolor[0];
    ei.ForegroundColor[1] = fcolor[1];
    ei.ForegroundColor[2] = fcolor[2];
    }
  else
    {
    ei.ForegroundColor[0] = 1.0;
    ei.ForegroundColor[1] = 1.0;
    ei.ForegroundColor[2] = 1.0;
    }

  this->Internal->TrackInfo[id] = ei;
  this->Modified();
  return VTK_OK;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackLabelRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackLabelRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgTrackLabelRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackLabelRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackLabelRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgTrackLabelRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgTrackLabelRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vtkVgTrackLabelRepresentation::SetVisible(int flag)
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
void vtkVgTrackLabelRepresentation::SetLabelColorHelper(
  vtkVgTrackLabelColorHelper* labelColorHelper)
{
  if (this->LabelColorHelper != labelColorHelper)
    {
    this->LabelColorHelper = labelColorHelper;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackLabelRepresentation::ShowTrackAnnotation(vtkVgTrack* track,
  bool rebuild)
{
  vtkVgAnnotationActor*& actor = this->Internal->Actors[track->GetId()];

  // @TODO: Maintain a pool of actors that can be reused in order to
  // avoid some of this reallocation.
  if (!actor)
    {
    actor = vtkVgAnnotationActor::New();
    actor->SetClampToViewport(false);

    this->NewPropCollection->AddItem(actor);
    this->ActivePropCollection->AddItem(actor);
    rebuild = true;
    }

  actor->SetUserMatrix(this->RepresentationMatrix);

  if (rebuild)
    {
    const double* backgroundColor = 0;
    const double* foregroundColor = 0;

    if (this->LabelColorHelper)
      {
      backgroundColor =
        this->LabelColorHelper->GetTrackLabelBackgroundColor(track);
      foregroundColor =
        this->LabelColorHelper->GetTrackLabelForegroundColor(track);
      }

    if (!backgroundColor || !foregroundColor)
      {
      const vtkInternal::TrackInfoItem* info;
      if (this->TrackFilter)
        {
        info = &this->Internal->TrackInfo[
                this->TrackFilter->GetBestClassifier(track)];
        }
      else
        {
        info = &this->Internal->TrackInfo[track->GetBestPVOClassifier()];
        }

      if (!backgroundColor)
        {
        backgroundColor = info->BackgroundColor;
        }
      if (!foregroundColor)
        {
        foregroundColor = info->ForegroundColor;
        }
      }

    actor->GetFrameProperty()->SetColor(backgroundColor[0],
                                        backgroundColor[1],
                                        backgroundColor[2]);

    actor->GetTextProperty()->SetColor(foregroundColor[0],
                                       foregroundColor[1],
                                       foregroundColor[2]);

    actor->SetOffset(0, -15);
    actor->AutoCenterXOn();

    std::ostringstream ostr;

    if (this->LabelPrefix)
      {
      ostr << this->LabelPrefix;
      }

    if (this->ShowName)
      {
      const char* name = track->GetName();
      if (name)
        {
        ostr << name;
        }
      else
        {
        ostr << 'T' << track->GetId();
        }
      }

    if (this->ShowProbability)
      {
      double PVO[3];
      track->GetPVO(PVO);

      // show PVO from top to bottom if the track isn't unclassified
      if (PVO[0] != 0.0 || PVO[1] != 0.0 || PVO[2] != 0.0)
        {
        if (this->ShowName)
          {
          ostr << ' ';
          }

        ostr.setf(ios::fixed, ios::floatfield);
        ostr.precision(2);
        ostr << "(P:" << PVO[0] << ", V:" << PVO[1] << ", O:" << PVO[2] << ')';
        }
      }

    actor->SetText(ostr.str().c_str());
    }

  vtkIdType npts, *pts, trackPointId;
  double labelPosition[3];
  vtkVgTimeStamp now = this->TrackModel->GetCurrentTimeStamp();
  track->GetHeadIdentifier(now, npts, pts, trackPointId);

  if (npts > 0)
    {
    vtkPoints* points = track->GetPoints();

    // Loop through points, transforming to get the minimum "y" (v) point. If
    // there are several points close to the minimum y, average their min and
    // max x values together.
    // TODO: This is identical to the code used to position to event labels,
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
    const vgPoint2d point = { 0.5 * (minX + maxX), minY };
    vtkVgApplyHomography(point, this->Internal->InvRepresentationMatrix,
                         labelPosition);
    labelPosition[2] = 0.0;
    }
  else
    {
    // position the annotation at the head of the track
    vtkVgTrackDisplayData tdd = this->TrackModel->GetTrackDisplayData(track);
    vtkIdType headId = tdd.IdsStart[tdd.NumIds - 1];

    track->GetPoints()->GetPoint(headId, labelPosition);
    }
  actor->SetPosition(labelPosition);
}

//-----------------------------------------------------------------------------
void vtkVgTrackLabelRepresentation::HideTrackAnnotation(vtkVgTrack* track)
{
  ActorMap::iterator iter = this->Internal->Actors.find(track->GetId());
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
void vtkVgTrackLabelRepresentation::Update()
{
  if (!this->GetVisible())
    {
    return;
    }

  bool forceRebuild =
    this->GetMTime() > this->UpdateTime ||
    this->TrackFilter->GetMTime() > this->UpdateTime;

  if (!forceRebuild && this->TrackModel->GetMTime() < this->UpdateTime &&
      this->TrackModel->GetUpdateTime() < this->UpdateTime)
    {
    return;
    }

  // Recompute the cached inverse matrix if necessary
  if (this->Internal->InvRepresentationMatrix->GetMTime() <
      this->RepresentationMatrix->GetMTime())
    {
    vtkMatrix4x4::Invert(this->RepresentationMatrix,
                         this->Internal->InvRepresentationMatrix);
    }

  vtkVgTrackInfo trackInfo;
  this->TrackModel->InitTrackTraversal();
  while ((trackInfo = this->TrackModel->GetNextTrack()).GetTrack())
    {
    vtkVgTrack* track = trackInfo.GetTrack();
    bool visible = track != this->ExcludedTrack &&
                   trackInfo.GetDisplayTrack() &&
                   trackInfo.GetPassesFilters() &&
                   trackInfo.GetHeadVisible();

    if (!visible || this->TrackFilter->GetBestClassifier(track) == -1)
      {
      this->HideTrackAnnotation(track);
      continue;
      }
    else if (this->TrackModel->GetTrackDisplayData(track).NumIds == 0)
      {
      // Only do the work of getting the HeadIdentifier (which is preferred
      // display vehicle) if we know we don't have the trail as backup
      vtkIdType npts, *pts, trackPointId;
      track->GetHeadIdentifier(this->TrackModel->GetCurrentTimeStamp(), npts,
                               pts, trackPointId);
      if (npts == 0)
        {
        this->HideTrackAnnotation(track);
        continue;
        }
      }
    this->ShowTrackAnnotation(track, forceRebuild);
    }

  this->UpdateTime.Modified();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTrackLabelRepresentation::Pick(double renX,
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
