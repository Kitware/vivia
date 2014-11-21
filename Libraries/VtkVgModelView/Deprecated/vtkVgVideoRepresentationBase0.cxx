/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgVideoRepresentationBase0.h"

#include "vtkVgEventRepresentationBase.h"
#include "vtkVgTrackRepresentationBase.h"
#include "vtkVgVideoModel0.h"

// VTK includes.
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>

//-----------------------------------------------------------------------------
vtkVgVideoRepresentationBase0::vtkVgVideoRepresentationBase0() :
  vtkVgRepresentationBase(),
  Visible(1),
  VideoModel(NULL),
  EventRepresentation(NULL),
  TrackRepresentation(NULL)
{
  this->SetLayerIndex(0);
}

//-----------------------------------------------------------------------------
vtkVgVideoRepresentationBase0::~vtkVgVideoRepresentationBase0()
{
  this->SetEventRepresentation(0);
  this->SetTrackRepresentation(0);
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentationBase0::SetTrackRepresentation(
  vtkVgTrackRepresentationBase* trackRep)
{
  if (this->TrackRepresentation != trackRep)
    {
    vtkVgTrackRepresentationBase* temp = this->TrackRepresentation;
    this->TrackRepresentation = trackRep;
    if (this->TrackRepresentation)
      {
      this->TrackRepresentation->Register(this);
      this->TrackRepresentation->SetVisible(
        this->GetVisible() && this->GetTrackVisible());
      }
    if (temp)
      {
      temp->UnRegister(this);
      }

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentationBase0::SetEventRepresentation(
  vtkVgEventRepresentationBase* eventRep)
{
  if (this->EventRepresentation != eventRep)
    {
    vtkVgEventRepresentationBase* temp = this->EventRepresentation;
    this->EventRepresentation = eventRep;
    if (this->EventRepresentation)
      {
      this->EventRepresentation->Register(this);
      this->EventRepresentation->SetVisible(
        this->GetVisible() && this->GetEventVisible());
      }
    if (temp)
      {
      temp->UnRegister(this);
      }

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentationBase0::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentationBase0::SetVideoModel(vtkVgVideoModel0* videoModel)
{
  if (videoModel && (this->VideoModel.GetPointer() != videoModel))
    {
    this->VideoModel = videoModel;

    if (this->UseAutoUpdate)
      {
      this->VideoModel->AddObserver(vtkCommand::UpdateDataEvent, this,
                                    &vtkVgVideoRepresentationBase0::Update);
      }

    this->VideoModel->AddObserver(vtkCommand::ErrorEvent, this,
      &vtkVgVideoRepresentationBase0::HandleModelError);

    this->VideoModel->AddObserver(vtkCommand::StartAnimationCueEvent, this,
      &vtkVgVideoRepresentationBase0::HandleStartAnimationCueEvent);

    this->VideoModel->AddObserver(vtkCommand::AnimationCueTickEvent, this,
      &vtkVgVideoRepresentationBase0::HandleAnimationCueTickEvent);

    this->VideoModel->AddObserver(vtkCommand::EndAnimationCueEvent, this,
      &vtkVgVideoRepresentationBase0::HandleEndAnimationCueEvent);

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vtkVgVideoModel0* vtkVgVideoRepresentationBase0::GetVideoModel()
{
  return this->VideoModel;
}

//-----------------------------------------------------------------------------
const vtkVgVideoModel0* vtkVgVideoRepresentationBase0::GetVideoModel() const
{
  return this->VideoModel;
}
