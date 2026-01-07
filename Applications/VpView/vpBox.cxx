// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpBox.h"

#include <vtkBorderWidget.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindowInteractor.h>

#include "vtkVgBorderRepresentation.h"

//-----------------------------------------------------------------------------
vpBox::vpBox(vtkRenderWindowInteractor* iren)
  : Visible(1), Interactor(iren)
{
  this->BorderRepresentation =
    vtkSmartPointer<vtkVgBorderRepresentation>::New();
  this->BorderWidget = vtkSmartPointer<vtkBorderWidget>::New();
  this->BorderWidget->SetRepresentation(this->BorderRepresentation);
  this->BorderWidget->SetInteractor(this->Interactor);

  this->SetLineColor(1.0, 1.0, 0.0);

  this->PolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints* pts = vtkPoints::New();
  vtkCellArray* ca = vtkCellArray::New();
  pts->SetNumberOfPoints(4);

  vtkIdType cell[] = { 0, 1, 2, 3, 0 };
  ca->InsertNextCell(5, cell);
  this->PolyData->SetPoints(pts);
  this->PolyData->SetLines(ca);
  pts->FastDelete();
  ca->FastDelete();
}

//-----------------------------------------------------------------------------
vpBox::~vpBox()
{
}

//-----------------------------------------------------------------------------
void vpBox::Initialize(vtkPolyData* pd)
{
  double bounds[6];
  pd->GetBounds(bounds);

  this->BorderRepresentation->SetPosition(bounds[0], bounds[2]);
  this->BorderRepresentation->SetPosition2(bounds[1], bounds[3]);
  this->BorderRepresentation->Modified();

  this->BorderWidget->On();
}

//-----------------------------------------------------------------------------
void vpBox::SetVisible(int visible)
{
  if (visible != this->Visible)
    {
    this->Visible = visible;
    this->BorderWidget->SetProcessEvents(visible);
    this->BorderRepresentation->SetVisibility(visible);
    }
}

//-----------------------------------------------------------------------------
bool vpBox::CanInteract(int X, int Y)
{
  return this->BorderRepresentation->ComputeInteractionState(X, Y) != 0;
}

//-----------------------------------------------------------------------------
void vpBox::SetLineColor(double r, double g, double b)
{
  this->BorderRepresentation->GetBorderProperty()->SetColor(r, g, b);
}

//-----------------------------------------------------------------------------
vtkPolyData* vpBox::GetPolyData()
{
  double ll[3];
  double ur[3];
  this->BorderRepresentation->GetPositionCoordinate()->GetValue(ll);
  this->BorderRepresentation->GetPosition2Coordinate()->GetValue(ur);

  vtkPoints* pts = this->PolyData->GetPoints();
  pts->SetPoint(0, ll[0], ur[1], 0.0);
  pts->SetPoint(1, ll[0], ll[1], 0.0);
  pts->SetPoint(2, ur[0], ll[1], 0.0);
  pts->SetPoint(3, ur[0], ur[1], 0.0);
  pts->Modified();

  return this->PolyData;
}

//-----------------------------------------------------------------------------
vtkAbstractWidget* vpBox::GetWidget()
{
  return this->BorderWidget;
}
