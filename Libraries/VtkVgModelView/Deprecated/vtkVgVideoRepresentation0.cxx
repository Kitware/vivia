/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgVideoRepresentation0.h"

// VG includes.
#include "vtkVgEventRepresentationBase.h"
#include "vtkVgTrackRepresentationBase.h"
#include "vtkVgVideoModel0.h"

// VTK includes.
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineFilter.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropCollection.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkVertex.h>

vtkStandardNewMacro(vtkVgVideoRepresentation0);

//-----------------------------------------------------------------------------
vtkVgVideoRepresentation0::vtkVgVideoRepresentation0() :
  vtkVgVideoRepresentationBase0()
{
  this->VideoActor = vtkImageActorRef::New();
  this->OutlineActor = vtkActorRef::New();
  this->MarkerActor = vtkActorRef::New();
  this->MarkerMapper = 0;

  this->VideoTransform = vtkTransformRef::New();
  this->OutlineTransform = vtkTransformRef::New();
  this->MarkerTransform = vtkTransformRef::New();

  this->VideoActor->SetUserTransform(this->VideoTransform);
  this->OutlineActor->SetUserTransform(this->OutlineTransform);
  this->MarkerActor->SetUserTransform(this->MarkerTransform);

  this->EventVisible = 1;
  this->TrackVisible = 1;
  this->OutlineVisible = 0;
  this->VideoVisible = 1;
  this->SelectionVisible = 0;
  this->MarkerVisible = 1;

  this->OutlineFilter = 0;
  this->OutlineStippled = false;

  this->DummyImageData = vtkImageDataRef::New();

  this->MarkerPointId = 0;
  this->MarkerLocation[0] = 0.0;
  this->MarkerLocation[1] = 0.0;
  this->MarkerLocation[2] = 0.0;

  this->OutlineActor->VisibilityOff();
  this->OutlineColor[0] = 1.0;
  this->OutlineColor[1] = 1.0;
  this->OutlineColor[2] = 0.0;
  this->OutlineActor->GetProperty()->SetColor(this->OutlineColor);
  this->OutlineActor->GetProperty()->SetLineWidth(4.0f);

  this->MarkerActor->GetProperty()->SetPointSize(10.0f);
  this->MarkerActor->GetProperty()->SetColor(this->OutlineColor);

  this->SelectionColor[0] = 1.0;
  this->SelectionColor[1] = 0.08;
  this->SelectionColor[2] = 0.58;

  // \NOTE: Apply some offset to avoid Z fighting.
  this->VideoZOffset = 0.1;
  this->OutlineZOffset = 0.2;
  this->MarkerZOffset = 0.3;
  this->SelectionZOffset = 0.2;

  this->NewPropCollection    = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();

  this->NewPropCollection->AddItem(this->VideoActor);
  this->NewPropCollection->AddItem(this->OutlineActor);
  this->NewPropCollection->AddItem(this->MarkerActor);

  this->ActivePropCollection->AddItem(this->VideoActor);
  this->ActivePropCollection->AddItem(this->OutlineActor);
  this->ActivePropCollection->AddItem(this->MarkerActor);

  this->SetLayerIndex(0);

  this->ModelError = false;

  UpdateRenderObjectsTime.Modified();
}

//-----------------------------------------------------------------------------
vtkVgVideoRepresentation0::~vtkVgVideoRepresentation0()
{
  if (this->OutlineFilter)
    {
    this->OutlineFilter->Delete();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgVideoRepresentation0::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgVideoRepresentation0::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vtkVgVideoRepresentation0::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgVideoRepresentation0::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgVideoRepresentation0::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vtkVgVideoRepresentation0::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetVisible(int flag)
{
  if (this->Visible == flag || this->ModelError)
    {
    return;
    }

  this->Visible = flag;

  this->VideoActor->SetVisibility(flag && this->VideoVisible);
  this->OutlineActor->SetVisibility(flag && this->OutlineVisible);
  this->MarkerActor->SetVisibility(flag && this->MarkerVisible);

  // Turn off other related representations.
  // \NOTE: This might change in future.
  if (this->TrackRepresentation)
    {
    this->TrackRepresentation->SetVisible(flag && this->TrackVisible);
    }

  if (this->EventRepresentation)
    {
    this->EventRepresentation->SetVisible(flag && this->EventVisible);
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetOutlineVisible(int flag)
{
  if (this->OutlineVisible != flag && this->GetVisible())
    {
    this->OutlineVisible = flag;
    this->OutlineActor->SetVisibility(this->OutlineVisible);
    this->Modified();
    }
  // Postpone actually setting the visibility until later when the representation
  // will become visible.
  else if (this->OutlineVisible != flag)
    {
    this->OutlineVisible = flag;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
int vtkVgVideoRepresentation0::GetOutlineVisible() const
{
  return (this->OutlineVisible && this->GetVisible());
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetSelectionVisible(int flag)
{
  if (flag)
    {
    this->OutlineActor->GetProperty()->SetColor(this->SelectionColor);
    this->OutlineActor->GetProperty()->SetLineWidth(8.0f);
    }
  else
    {
    this->OutlineActor->GetProperty()->SetColor(this->OutlineColor);
    this->OutlineActor->GetProperty()->SetLineWidth(4.0f);
    }

  // In sync with outline color
  this->MarkerActor->GetProperty()->SetColor(
    this->OutlineActor->GetProperty()->GetColor());

  this->SelectionVisible = flag;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetMarkerVisible(int flag)
{
  if (this->MarkerVisible != flag && this->GetVisible())
    {
    this->MarkerVisible = flag;
    this->MarkerActor->SetVisibility(this->MarkerVisible);
    this->Modified();
    }
  // Postpone actually setting the visibility until later when the representation
  // will become visible.
  else if (this->MarkerVisible != flag)
    {
    this->MarkerVisible = flag;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetVideoVisible(int flag)
{
  if (this->VideoVisible != flag && this->GetVisible())
    {
    this->VideoVisible = flag;
    this->VideoActor->SetVisibility(this->VideoVisible);
    this->Modified();
    }
  // Postpone actually setting the visibility until later when the representation
  // will become visible.
  else if (this->VideoVisible != flag)
    {
    this->VideoVisible = flag;
    this->Modified();
    }

  // Pass visibility flag to event as well
  this->SetEventVisible(flag);
}

//-----------------------------------------------------------------------------
int vtkVgVideoRepresentation0::GetVideoVisible() const
{
  return (this->VideoVisible && this->GetVisible());
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetTrackVisible(int flag)
{
  if (this->TrackVisible != flag)
    {
    this->TrackVisible = flag;
    if (this->TrackRepresentation)
      {
      this->TrackRepresentation->SetVisible(this->GetVisible() &&
                                            this->TrackVisible);
      }

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetEventVisible(int flag)
{
  // Do not turn visibility ON if we are not playing and video clip
  // is not visible.
  if (flag && this->VideoModel && !this->VideoModel->IsPlaying()
      && !this->GetVideoVisible())
    {
    return;
    }

  if (this->EventVisible != flag)
    {
    this->EventVisible = flag;
    if (this->EventRepresentation)
      {
      this->EventRepresentation->SetVisible(this->GetVisible() &&
                                            this->EventVisible);
      }

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
int vtkVgVideoRepresentation0::GetVisible() const
{
  return this->Visible;
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetOutlineColor(double r, double g, double b)
{
  if (this->SelectionVisible)
    {
    return;
    }

  if (this->OutlineColor[0] != r || this->OutlineColor[1] != g ||
      this->OutlineColor[2] != b)
    {
    this->OutlineColor[0] = r;
    this->OutlineColor[1] = g;
    this->OutlineColor[2] = b;
    this->OutlineActor->GetProperty()->SetColor(this->OutlineColor);

    // Outline color and marker color should be in sync
    this->MarkerActor->GetProperty()->SetColor(this->OutlineColor);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetOutlineColor(double rgb[3])
{
  this->SetOutlineColor(rgb[0], rgb[1], rgb[2]);
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetOutlineStippled(bool enable)
{
  if (enable != this->OutlineStippled)
    {
    this->OutlineStippled = enable;
    this->OutlineActor->GetProperty()->SetLineStipplePattern(enable ?
      0xf0f0 : 0xffff);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetSelectionColor(double r, double g, double b)
{
  if (this->SelectionColor[0] != r || this->SelectionColor[1] != g ||
      this->SelectionColor[2] != b)
    {
    this->SelectionColor[0] = r;
    this->SelectionColor[1] = g;
    this->SelectionColor[2] = b;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::SetSelectionColor(double rgb[3])
{
  this->SetSelectionColor(rgb[0], rgb[1], rgb[2]);
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::Update()
{
  if (this->VideoModel->GetUpdateDataRequestTime() >
      this->UpdateTime.GetMTime() || this->GetUpdateRequest())
    {
    const vtkVgVideoFrameData* frameData = this->VideoModel->GetFrameData();

    if (frameData->VideoImage)
      {
      if (!this->OutlineFilter)
        {
        this->OutlineFilter = vtkOutlineFilter::New();
        this->OutlineFilter->SetInputData(frameData->VideoImage);
        vtkPolyDataMapper* PDM = vtkPolyDataMapper::New();
        PDM->SetInputConnection(this->OutlineFilter->GetOutputPort());
        this->OutlineActor->SetMapper(PDM);
        PDM->FastDelete();
        }
      else
        {
        this->OutlineFilter->SetInputData(frameData->VideoImage);
        }
      this->VideoActor->SetInputData(frameData->VideoImage);

      if (!this->MarkerMapper)
        {
        this->CreateMarker(frameData->VideoImage);
        }
      else
        {
        this->UpdateMarker(frameData->VideoImage);
        }
      }

    // Calculate the render matrix.
    this->RenderMatrix->Identity();

    if (frameData->VideoMatrix && this->UseModelMatrix)
      {
      vtkMatrix4x4::Multiply4x4(
        this->VideoModel->GetModelMatrix(), frameData->VideoMatrix,
        this->RenderMatrix);
      }
    else
      {
      this->RenderMatrix = this->VideoModel->GetModelMatrix();
      }

    vtkMatrix4x4::Multiply4x4(
      this->RepresentationMatrix, this->RenderMatrix,
      this->RenderMatrix);

    // HACK - This is needed to make picking work correctly.
    this->MakeLinear(this->RenderMatrix);

    // prepare transformations... add in z offset
    this->VideoTransform->PostMultiply();  // just to be sure
    this->VideoTransform->SetMatrix(this->RenderMatrix);
    this->VideoTransform->Translate(0.0, 0.0, this->VideoZOffset  +
      vtkVgRepresentationBase::GetZOffset(this->VideoActor));

    this->OutlineTransform->PostMultiply();  // just to be sure
    this->OutlineTransform->SetMatrix(this->RenderMatrix);
    this->OutlineTransform->Translate(0.0, 0.0, this->OutlineZOffset  +
      vtkVgRepresentationBase::GetZOffset(this->OutlineActor));

    this->MarkerTransform->PostMultiply();  // just to be sure
    this->MarkerTransform->SetMatrix(this->RenderMatrix);
    this->MarkerTransform->Translate(0.0, 0.0, this->MarkerZOffset  +
      vtkVgRepresentationBase::GetZOffset(this->MarkerActor));

    if (this->EventRepresentation)
      {
      this->EventRepresentation->SetRepresentationMatrix(this->RenderMatrix);
      this->EventRepresentation->Update();
      }

    if (this->TrackRepresentation)
      {
      this->TrackRepresentation->SetRepresentationMatrix(this->RenderMatrix);
      this->TrackRepresentation->Update();
      }

    this->UpdateTime.Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::HandleModelError()
{
  this->VideoActor->SetUseBounds(0);
  this->OutlineActor->SetUseBounds(0);
  this->MarkerActor->SetUseBounds(0);

  // \NOTE It is important that we call \c SetVisible(0) before
  // we make the \c ModelError set to \c true since \c ModelError
  // prevents changing visibility state of all the drawable
  // entries in this representation.
  this->SetVisible(0);
  this->ModelError = true;
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::HandleAnimationCueTickEvent()
{
  if (this->VideoModel->IsPlaying() && !this->GetEventVisible())
    {
    this->SetEventVisible(1);
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::HandleEndAnimationCueEvent()
{
  // Set event visibility to false only when event representation is visible.
  // Why? This is to diffrentiate between states when we receive this
  // event while playing a video without video image shown vs when
  // we receive this event when other video started playing. We have this
  // requirement where we need to turn on the visibility of even region
  // when we play a video (video image not shown) and turn off when we are stopped.
  if (this->EventRepresentation->GetVisible() && !this->GetVideoVisible())
    {
    this->SetEventVisible(0);
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::CreateMarker(vtkImageData* imageData)
{
  if (!imageData)
    {
    return;
    }

  this->MarkerMapper = vtkPolyDataMapperRef::New();
  this->MarkerPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->MarkerPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> mVerts(vtkSmartPointer<vtkCellArray>::New());

  this->MarkerActor->SetMapper(this->MarkerMapper);
  this->MarkerPolyData->SetPoints(this->MarkerPoints);
  this->MarkerPolyData->SetVerts(mVerts);
  this->MarkerPointId = this->MarkerPoints->InsertNextPoint(
    this->MarkerLocation);

  vtkIdType pts[1] = {this->MarkerPointId};
  mVerts->InsertNextCell(1, pts);

  this->MarkerMapper->SetInputData(this->MarkerPolyData);


  this->UpdateMarker(imageData);
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::UpdateMarker(vtkImageData* imageData)
{
  if (!imageData)
    {
    return;
    }

  int dims[3];
  imageData->GetDimensions(dims);

  this->MarkerLocation[0] = static_cast<double>(dims[0]) * 0.5;
  this->MarkerLocation[1] = static_cast<double>(dims[1]) * 0.5;
  this->MarkerLocation[2] = static_cast<double>(dims[2]);

  this->MarkerPoints->SetPoint(this->MarkerPointId, this->MarkerLocation);
  this->MarkerPolyData->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgVideoRepresentation0::MakeLinear(vtkMatrix4x4* mat)
{
  double w = mat->GetElement(3, 3);
  for (int i = 0; i < 4; ++i)
    {
    for (int j = 0; j < 4; ++j)
      {
      mat->SetElement(i, j, mat->GetElement(i, j) / w);
      }
    }
  mat->SetElement(2, 2, 1.0);
}
