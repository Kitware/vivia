/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgBorderRepresentation.h"

#include "vtkActor2D.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkRenderer.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkWindow.h"

vtkStandardNewMacro(vtkVgBorderRepresentation);

//------------------------------------------------------------------------
vtkVgBorderRepresentation::vtkVgBorderRepresentation()
{
  this->PositionCoordinate->SetCoordinateSystemToWorld();
  this->Position2Coordinate->SetCoordinateSystemToWorld();
  this->Position2Coordinate->SetReferenceCoordinate(0);

  // using our own transform
  vtkCoordinate* xform = vtkCoordinate::New();
  xform->SetCoordinateSystemToWorld();
  this->BWMapper->SetTransformCoordinate(xform);
  this->BWMapper->SetInputData(this->BWPolyData);
  this->BWTransformFilter->SetInputData(0);
  xform->FastDelete();
}

//------------------------------------------------------------------------
vtkVgBorderRepresentation::~vtkVgBorderRepresentation()
{
}

//-------------------------------------------------------------------------
void vtkVgBorderRepresentation::StartWidgetInteraction(double eventPos[2])
{
  // Convert from normalized viewport coords to world.
  double x = eventPos[0];
  double y = eventPos[1];
  double z = 0.0;
  this->Renderer->NormalizedViewportToView(x, y, z);
  this->Renderer->SetViewPoint(x, y, z);
  this->Renderer->ViewToWorld();

  double* wp = this->Renderer->GetWorldPoint();
  this->StartEventPosition[0] = vtkMath::Round(wp[0]);
  this->StartEventPosition[1] = vtkMath::Round(wp[1]);
}

//-------------------------------------------------------------------------
// Note: Most of this is copied wholesale from vtkBorderRepresentation.
//-------------------------------------------------------------------------
void vtkVgBorderRepresentation::WidgetInteraction(double eventPos[2])
{
  double XF = eventPos[0];
  double YF = eventPos[1];

  // convert to world coordinates
  this->Renderer->SetDisplayPoint(XF, YF, 0.0);
  this->Renderer->DisplayToWorld();
  XF = vtkMath::Round(this->Renderer->GetWorldPoint()[0]);
  YF = vtkMath::Round(this->Renderer->GetWorldPoint()[1]);

  // there are four parameters that can be adjusted
  double* fpos1 = this->PositionCoordinate->GetValue();
  double* fpos2 = this->Position2Coordinate->GetValue();
  double par1[2];
  double par2[2];
  par1[0] = fpos1[0];
  par1[1] = fpos1[1];
  par2[0] = fpos2[0];
  par2[1] = fpos2[1];

  double delX = XF - this->StartEventPosition[0];
  double delY = YF - this->StartEventPosition[1];
  double delX2 = 0.0, delY2 = 0.0;

  // Based on the state, adjust the representation. Note that we force a
  // uniform scaling of the widget when tugging on the corner points (and
  // when proportional resize is on). This is done by finding the maximum
  // movement in the x-y directions and using this to scale the widget.
  if (this->ProportionalResize && !this->Moving)
    {
    double sx = (fpos2[0] - fpos1[0]) / (fpos2[1] - fpos1[1]);
    double sy = (fpos2[1] - fpos1[1]) / (fpos2[0] - fpos1[0]);
    if (fabs(delX) > fabs(delY))
      {
      delY = sy * delX;
      delX2 = delX;
      delY2 = -delY;
      }
    else
      {
      delX = sx * delY;
      delY2 = delY;
      delX2 = -delX;
      }
    }
  else
    {
    delX2 = delX;
    delY2 = delY;
    }

  // The previous "if" statement has taken care of the proportional resize
  // for the most part. However, tugging on edges has special behavior, which
  // is to scale the box about its center.
  switch (this->InteractionState)
    {
    case vtkBorderRepresentation::AdjustingP0:
      par1[0] = par1[0] + delX;
      par1[1] = par1[1] + delY;
      break;
    case vtkBorderRepresentation::AdjustingP1:
      par2[0] = par2[0] + delX2;
      par1[1] = par1[1] + delY2;
      break;
    case vtkBorderRepresentation::AdjustingP2:
      par2[0] = par2[0] + delX;
      par2[1] = par2[1] + delY;
      break;
    case vtkBorderRepresentation::AdjustingP3:
      par1[0] = par1[0] + delX2;
      par2[1] = par2[1] + delY2;
      break;
    case vtkBorderRepresentation::AdjustingE0:
      par1[1] = par1[1] + delY;
      if (this->ProportionalResize)
        {
        par2[1] = par2[1] - delY;
        par1[0] = par1[0] + delX;
        par2[0] = par2[0] - delX;
        }
      break;
    case vtkBorderRepresentation::AdjustingE1:
      par2[0] = par2[0] + delX;
      if (this->ProportionalResize)
        {
        par1[0] = par1[0] - delX;
        par1[1] = par1[1] - delY;
        par2[1] = par2[1] + delY;
        }
      break;
    case vtkBorderRepresentation::AdjustingE2:
      par2[1] = par2[1] + delY;
      if (this->ProportionalResize)
        {
        par1[1] = par1[1] - delY;
        par1[0] = par1[0] - delX;
        par2[0] = par2[0] + delX;
        }
      break;
    case vtkBorderRepresentation::AdjustingE3:
      par1[0] = par1[0] + delX;
      if (this->ProportionalResize)
        {
        par2[0] = par2[0] - delX;
        par1[1] = par1[1] + delY;
        par2[1] = par2[1] - delY;
        }
      break;
    case vtkBorderRepresentation::Inside:
      if (this->Moving)
        {
        par1[0] = par1[0] + delX;
        par1[1] = par1[1] + delY;
        par2[0] = par2[0] + delX;
        par2[1] = par2[1] + delY;
        }
      break;
    }

  // Modify the representation
  if (par2[0] > par1[0] && par2[1] > par1[1])
    {
    this->SetCoordValue(par1, this->PositionCoordinate);
    this->SetCoordValue(par2, this->Position2Coordinate);
    this->StartEventPosition[0] = XF;
    this->StartEventPosition[1] = YF;
    }

  this->Modified();
  this->BuildRepresentation();
}

//-------------------------------------------------------------------------
void vtkVgBorderRepresentation::BuildRepresentation()
{
  if (this->GetMTime() > this->BuildTime ||
      (this->Renderer && this->Renderer->GetVTKWindow() &&
       this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime))
    {
    // Set things up
    double* pos1 = this->PositionCoordinate->GetComputedValue(this->Renderer);
    double* pos2 = this->Position2Coordinate->GetComputedValue(this->Renderer);

    this->BWPoints->SetPoint(0, pos1[0], pos1[1], 0.0);
    this->BWPoints->SetPoint(1, pos2[0], pos1[1], 0.0);
    this->BWPoints->SetPoint(2, pos2[0], pos2[1], 0.0);
    this->BWPoints->SetPoint(3, pos1[0], pos2[1], 0.0);

    this->BWPolyData->Modified();
    this->BuildTime.Modified();
    }
}

//-------------------------------------------------------------------------
void vtkVgBorderRepresentation::SetDisplayPoint1(int p1[2])
{
  this->SetDisplayPoint(p1, this->Point1);
  this->SetCoordValue(this->Point1, this->PositionCoordinate);
  this->Modified();
}

//-------------------------------------------------------------------------
void vtkVgBorderRepresentation::SetDisplayPoint2(int p2[2])
{
  this->SetDisplayPoint(p2, this->Point2);

  // The coordinates need to be "normalized" so that position 2 is always above
  // and to the right of position 1. vtkBorderWidget relies on this.
  double p1coord[3] = { this->Point1[0], this->Point1[1] };
  double p2coord[3] = { this->Point2[0], this->Point2[1] };
  if (p2coord[0] < p1coord[0])
    {
    std::swap(p2coord[0], p1coord[0]);
    }
  if (p2coord[1] < p1coord[1])
    {
    std::swap(p2coord[1], p1coord[1]);
    }
  this->SetCoordValue(p1coord, this->PositionCoordinate);
  this->SetCoordValue(p2coord, this->Position2Coordinate);
  this->Modified();
}

//-------------------------------------------------------------------------
void vtkVgBorderRepresentation::SetDisplayPoint(int p[2], double world[2])
{
  this->Renderer->SetDisplayPoint(p[0], p[1], 0.0);
  this->Renderer->DisplayToWorld();
  world[0] = this->Renderer->GetWorldPoint()[0];
  world[1] = this->Renderer->GetWorldPoint()[1];
}

//-------------------------------------------------------------------------
void vtkVgBorderRepresentation::SetCoordValue(double p[2], vtkCoordinate* coord)
{
  // snap to integral world (image) coordinates
  coord->SetValue(vtkMath::Round(p[0]), vtkMath::Round(p[1]), 0.0);
}
