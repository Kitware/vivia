// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpContour.h"

#include <vtkLinearContourLineInterpolator.h>
#include <vtkMatrix4x4.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>

#include "vtkVgContourRepresentation.h"
#include "vtkVgContourWidget.h"

//-----------------------------------------------------------------------------
vpContour::vpContour(vtkRenderWindowInteractor* iren)
  : Visible(1), InteractionDisabled(false), Interactor(iren)
{
  this->ContourRepresentation =
    vtkSmartPointer<vtkVgContourRepresentation>::New();
  this->ContourWidget = vtkSmartPointer<vtkVgContourWidget>::New();
  this->ContourWidget->SetRepresentation(this->ContourRepresentation);
  this->ContourWidget->SetInteractor(this->Interactor);

  vtkLinearContourLineInterpolator* interpolator =
    vtkLinearContourLineInterpolator::New();

  this->FinalLineColor[0] = 0.8;
  this->FinalLineColor[1] = 0.8;
  this->FinalLineColor[2] = 0.8;

  this->ContourRepresentation->SetLineColor(1.0, 1.0, 1.0);
  this->ContourRepresentation->AlwaysOnTopOn();
  this->ContourRepresentation->SetLineInterpolator(interpolator);

  this->WorldToImageMatrix = vtkMatrix4x4::New();

  interpolator->FastDelete();
}

//-----------------------------------------------------------------------------
vpContour::~vpContour()
{
  this->WorldToImageMatrix->Delete();
}

//-----------------------------------------------------------------------------
void vpContour::Initialize()
{
  this->ContourWidget->On();
  this->ContourWidget->Initialize();
}

//-----------------------------------------------------------------------------
void vpContour::Initialize(vtkPolyData* pd)
{
  this->ContourWidget->On();
  this->ContourWidget->Initialize(pd);
}

//-----------------------------------------------------------------------------
void vpContour::Begin()
{
  this->ContourWidget->FollowCursorOn();
  this->ContourWidget->Render();
}

//-----------------------------------------------------------------------------
void vpContour::End()
{
  // don't auto-add a 'dangling' mouse cursor node
  if (!this->ContourRepresentation->GetClosedLoop())
    {
    this->ContourRepresentation->DeleteLastNode();
    }
  this->ContourWidget->CloseLoop();
  this->DisableInteraction();
}

//-----------------------------------------------------------------------------
void vpContour::Finalize()
{
  this->ContourRepresentation->SetLineColor(this->FinalLineColor[0],
                                            this->FinalLineColor[1],
                                            this->FinalLineColor[2]);
}

//-----------------------------------------------------------------------------
void vpContour::SetVisible(int visible)
{
  if (visible != this->Visible)
    {
    this->Visible = visible;
    this->ContourWidget->SetProcessEvents(visible &&
                                          !this->InteractionDisabled);
    this->ContourRepresentation->SetVisibility(visible);
    }
}

//-----------------------------------------------------------------------------
bool vpContour::CanInteract(int X, int Y)
{
  double pos[3];
  return this->ContourRepresentation->FindClosestPoint(X, Y, pos) != 0;
}

//-----------------------------------------------------------------------------
void vpContour::SetPointSize(float pointSize)
{
  if (pointSize > 0)
    {
    this->ContourRepresentation->GetProperty()->SetPointSize(pointSize);
    }
}

//-----------------------------------------------------------------------------
float vpContour::GetPointSize()
{
  return this->ContourRepresentation->GetProperty()->GetPointSize();
}

//-----------------------------------------------------------------------------
void vpContour::SetActivePointSize(float pointSize)
{
  if (pointSize > 0)
    {
    this->ContourRepresentation->GetActiveProperty()->SetPointSize(pointSize);
    }
}

//-----------------------------------------------------------------------------
float vpContour::GetActivePointSize()
{
  return this->ContourRepresentation->GetActiveProperty()->GetPointSize();
}

//-----------------------------------------------------------------------------
void vpContour::SetLineColor(double r, double g, double b)
{
  this->ContourRepresentation->SetLineColor(r, g, b);
}

//-----------------------------------------------------------------------------
void vpContour::SetFinalLineColor(double r, double g, double b)
{
  this->FinalLineColor[0] = r;
  this->FinalLineColor[1] = g;
  this->FinalLineColor[2] = b;
}

//-----------------------------------------------------------------------------
vtkPolyData* vpContour::GetPolyData()
{
  return this->ContourRepresentation->GetContourRepresentationAsPolyData();
}

//-----------------------------------------------------------------------------
vtkAbstractWidget* vpContour::GetWidget()
{
  return this->ContourWidget;
}

//-----------------------------------------------------------------------------
void vpContour::DisableInteraction()
{
  this->InteractionDisabled = true;
  this->ContourWidget->ProcessEventsOff();

  this->ContourRepresentation->GetProperty()->SetOpacity(0.0);
  this->ContourRepresentation->GetActiveProperty()->SetOpacity(0.0);
}

//-----------------------------------------------------------------------------
void vpContour::SetWorldToImageMatrix(vtkMatrix4x4* matrix)
{
  if (matrix)
    {
    this->WorldToImageMatrix->DeepCopy(matrix);
    }
  else
    {
    this->WorldToImageMatrix->Identity();
    }
}
