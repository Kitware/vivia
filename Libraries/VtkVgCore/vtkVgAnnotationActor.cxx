/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgAnnotationActor.h"

#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPropCollection.h>
#include <vtkProperty2D.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>

#define MaxArrowLength     75.0   // pixels
#define MaxArrowLengthDist 500.0  // pixels

vtkStandardNewMacro(vtkVgAnnotationActor);

//----------------------------------------------------------------------
vtkVgAnnotationActor::vtkVgAnnotationActor()
  : Text(0), ClampToViewport(true), ClampedToBorder(false)
{
  this->Offset[0] = this->Offset[1] = 0;

  this->AutoCenterX = this->AutoCenterY = false;

  this->TextMapper = vtkTextMapper::New();
  this->TextActor = vtkActor2D::New();
  this->TextActor->SetMapper(this->TextMapper);

  vtkTextProperty* property = this->TextMapper->GetTextProperty();
  property->SetVerticalJustificationToTop();
  property->SetJustificationToCentered();
  property->SetFontSize(14);

  // contruct the frame
  this->FramePoints = vtkPoints::New();
  vtkPolyData* polydata = vtkPolyData::New();
  vtkCellArray* polys = vtkCellArray::New();

  this->FramePoints->SetNumberOfPoints(8);

  // central quad
    {
    vtkIdType ids[] = { 0, 1, 2, 3 };
    polys->InsertNextCell(4, ids);
    }

  // direction arrow triangles
    {
    vtkIdType left[]   = { 0, 3, 4 };
    vtkIdType bottom[] = { 1, 0, 5 };
    vtkIdType right[]  = { 2, 1, 6 };
    vtkIdType top[]    = { 3, 2, 7 };

    polys->InsertNextCell(3, left);
    polys->InsertNextCell(3, bottom);
    polys->InsertNextCell(3, right);
    polys->InsertNextCell(3, top);
    }

  polydata->SetPoints(this->FramePoints);
  polydata->SetPolys(polys);
  polys->FastDelete();

  vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::New();
  mapper->SetInputData(polydata);
  polydata->FastDelete();

  this->FrameActor = vtkActor2D::New();
  this->FrameActor->SetMapper(mapper);
  this->FrameActor->GetProperty()->SetColor(0.0, 0.4, 0.0);
  this->FrameActor->GetProperty()->SetOpacity(0.7);
  mapper->FastDelete();

  // default positioning is relative to world, not display
  this->FrameActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();

  // need a reference coordinate for text to allow display position offset
  // while maintaining a world anchor point
  vtkCoordinate* coord = vtkCoordinate::New();
  coord->SetCoordinateSystemToWorld();
  this->TextActor->GetPositionCoordinate()->SetReferenceCoordinate(coord);
  coord->FastDelete();
}

//----------------------------------------------------------------------
vtkVgAnnotationActor::~vtkVgAnnotationActor()
{
  delete [] Text;

  this->TextActor->Delete();
  this->TextMapper->Delete();
  this->FrameActor->Delete();
  this->FramePoints->Delete();
}

//-----------------------------------------------------------------------------
vtkTextProperty* vtkVgAnnotationActor::GetTextProperty()
{
  return this->TextMapper->GetTextProperty();
}

//-----------------------------------------------------------------------------
vtkProperty2D* vtkVgAnnotationActor::GetFrameProperty()
{
  return this->FrameActor->GetProperty();
}

//-----------------------------------------------------------------------------
void vtkVgAnnotationActor::GetTransformedPosition(double pos[3])
{
  vtkMatrix4x4* mat = this->GetMatrix();

  double w = mat->Element[3][3];
  pos[0] = mat->Element[0][3] / w;
  pos[1] = mat->Element[1][3] / w;
  pos[2] = mat->Element[2][3] / w;
}

//-----------------------------------------------------------------------------
void vtkVgAnnotationActor::Update(vtkViewport* v)
{
  this->TextMapper->SetInput(this->Text);

  int textSize[2];
  this->TextMapper->GetSize(v, textSize);

  int padding[] = { 4, 4, 1, 4 }; // left, right, bottom, top

  // size the frame in display coordinates relative to the text
  if (this->AutoCenterX)
    {
    // position the center of the frame over the anchor position
    this->Left = -(textSize[0] / 2) - padding[0];
    this->Right = (textSize[0] / 2) + padding[1];
    }
  else
    {
    // position the left corner of the frame at the anchor position
    this->Left = 0;
    this->Right = textSize[0] + padding[0] + padding[1];
    }

  if (this->AutoCenterY)
    {
    // position the center of the frame over the anchor position
    this->Top = (textSize[1] / 2) + padding[3];
    this->Bottom = -(textSize[1] / 2) - padding[2];
    }
  else
    {
    // position the top of the frame at the anchor position
    this->Top = 0;
    this->Bottom = -textSize[1] - (padding[2] + padding[3]);
    }

  // add offset
  this->Left += this->Offset[0];
  this->Right += this->Offset[0];
  this->Bottom += this->Offset[1];
  this->Top += this->Offset[1];

  this->FramePoints->SetPoint(0, this->Left, this->Bottom, 0.0);
  this->FramePoints->SetPoint(1, this->Right, this->Bottom, 0.0);
  this->FramePoints->SetPoint(2, this->Right, this->Top, 0.0);
  this->FramePoints->SetPoint(3, this->Left, this->Top, 0.0);

  // hide the direction triangles by making them degenerate
  this->FramePoints->SetPoint(4, this->FramePoints->GetPoint(0));
  this->FramePoints->SetPoint(5, this->FramePoints->GetPoint(1));
  this->FramePoints->SetPoint(6, this->FramePoints->GetPoint(2));
  this->FramePoints->SetPoint(7, this->FramePoints->GetPoint(3));

  // draw 'pointer' to anchor position when not in clamp mode
  if (!this->ClampToViewport)
    {
    double anchor[] = { 0.0, 0.0, 0.0 };

    if (this->Left < 0 && this->Right > 0)
      {
      // anchor between the left and right edges of the frame
      if (this->Bottom < 0 && this->Top > 0)
        {
        // anchor is between top and bottom edges
        }
      else if (this->Top < 0)
        {
        // anchor is above top edge
        this->FramePoints->SetPoint(7, anchor);
        }
      else if (this->Bottom > 0)
        {
        // anchor is below bottom edge
        this->FramePoints->SetPoint(5, anchor);
        }
      }
    else if (this->Left > 0)
      {
      // anchor is to the left of the frame
      if (this->Bottom < 0 && this->Top > 0)
        {
        // anchor is between top and bottom edges
        this->FramePoints->SetPoint(4, anchor);
        }
      else if (this->Top < 0)
        {
        // anchor is above top edge
        this->FramePoints->SetPoint(3, anchor);
        }
      else if (this->Bottom > 0)
        {
        // anchor is below bottom edge
        this->FramePoints->SetPoint(0, anchor);
        }
      }
    else if (this->Right < 0)
      {
      // anchor is to the right of the frame
      if (this->Bottom < 0 && this->Top > 0)
        {
        // anchor is between top and bottom edges
        this->FramePoints->SetPoint(6, anchor);
        }
      else if (this->Top < 0)
        {
        // anchor is above top edge
        this->FramePoints->SetPoint(2, anchor);
        }
      else if (this->Bottom > 0)
        {
        // anchor is below bottom edge
        this->FramePoints->SetPoint(1, anchor);
        }
      }
    }

  vtkCoordinate* textCoord = this->TextActor->GetPositionCoordinate();

  // set world position of frame and text
  double position[3];
  this->GetTransformedPosition(position);
  this->FrameActor->SetPosition(position);
  textCoord->GetReferenceCoordinate()->SetValue(position);

  // add text offset
  textCoord->SetValue(0.5 * (this->Left + this->Right), this->Top - padding[3]);

  this->UpdateTime.Modified();
}

//----------------------------------------------------------------------
void vtkVgAnnotationActor::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);
  this->TextActor->ReleaseGraphicsResources(w);
  this->FrameActor->ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------
int vtkVgAnnotationActor::RenderOverlay(vtkViewport* v)
{
  if (!this->Text)
    {
    return 0;
    }

  int count = 0;
  count += this->FrameActor->RenderOverlay(v);
  count += this->TextActor->RenderOverlay(v);
  return count;
}

//----------------------------------------------------------------------
inline int vtkVgAnnotationActor::CalcArrowLength(int dist)
{
  const double limits[] = { 0.0, MaxArrowLengthDist };
  return MaxArrowLength * pow(vtkMath::ClampAndNormalizeValue(dist, limits), 0.75);
}

//----------------------------------------------------------------------
int vtkVgAnnotationActor::RenderOpaqueGeometry(vtkViewport* v)
{
  if (!this->Text)
    {
    return 0;
    }

  // update if text or position has changed
  if (this->GetMTime() > this->UpdateTime)
    {
    this->Update(v);
    }

  // if we're not in clamp mode, no further magic is required.
  if (!this->ClampToViewport)
    {
    int count = 0;
    count += this->FrameActor->RenderOpaqueGeometry(v);
    count += this->TextActor->RenderOpaqueGeometry(v);
    return count;
    }

  double position[3];
  this->GetTransformedPosition(position);
  v->SetWorldPoint(position);
  v->WorldToDisplay();
  double pos[] =
    {
    floor(v->GetDisplayPoint()[0]),
    floor(v->GetDisplayPoint()[1])
    };

  int* viewport = v->GetSize();

  int xmin = pos[0] + this->Left;
  int xmax = pos[0] + this->Right;
  int ymin = pos[1] + this->Bottom;
  int ymax = pos[1] + this->Top;

  int xmid = (this->Left + this->Right) / 2;
  int ymid = (this->Bottom + this->Top) / 2;

  double xOffset = 0;
  double yOffset = 0;

  // update the points for the left and right arrows
  if (xmin < 0)
    {
    xOffset = this->CalcArrowLength(-xmin);
    this->FramePoints->SetPoint(4, this->Left - xOffset, ymid, 0.0);
    }
  else if (xmax > viewport[0])
    {
    xOffset = -this->CalcArrowLength(xmax - viewport[0]);
    this->FramePoints->SetPoint(6, this->Right - xOffset, ymid, 0.0);
    }
  else if (this->ClampedToBorder)
    {
    // the x-extents of the frame are within the viewport - hide the arrows
    this->FramePoints->SetPoint(4, this->FramePoints->GetPoint(0));
    this->FramePoints->SetPoint(6, this->FramePoints->GetPoint(2));
    }

  // update the points for the top and bottom arrows
  if (ymin < 0)
    {
    yOffset = this->CalcArrowLength(-ymin);
    this->FramePoints->SetPoint(5, xmid, this->Bottom - yOffset, 0.0);
    }
  else if (ymax > viewport[1])
    {
    yOffset = -this->CalcArrowLength(ymax - viewport[1]);
    this->FramePoints->SetPoint(7, xmid, this->Top - yOffset, 0.0);
    }
  else if (this->ClampedToBorder)
    {
    // the y-extents of the frame are within the viewport - hide the arrows
    this->FramePoints->SetPoint(5, this->FramePoints->GetPoint(1));
    this->FramePoints->SetPoint(7, this->FramePoints->GetPoint(3));
    }

  vtkCoordinate* textCoord = this->TextActor->GetPositionCoordinate();

  // see if we need to clamp to the edge of the viewport
  if (xmin < 0 || xmax > viewport[0] || ymin < 0 || ymax > viewport[1])
    {
    // clamp to offset position
    double newPos[] = { pos[0], pos[1] };
    double xRange[] = { -this->Left + xOffset, viewport[0] - this->Right + xOffset };
    double yRange[] = { -this->Bottom + yOffset, viewport[1] - this->Top + yOffset };
    vtkMath::ClampValue(&newPos[0], xRange);
    vtkMath::ClampValue(&newPos[1], yRange);

    // use display-relative positioning while clamped
    if (!this->ClampedToBorder)
      {
      this->FrameActor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
      textCoord->GetReferenceCoordinate()->SetCoordinateSystemToDisplay();
      }

    this->FrameActor->SetPosition(newPos);
    textCoord->GetReferenceCoordinate()->SetValue(newPos[0], newPos[1]);
    this->ClampedToBorder = true;
    }
  else if (this->ClampedToBorder)
    {
    // the frame is now within the viewport again, so go back to using world coords
    this->FrameActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    this->FrameActor->SetPosition(this->GetPosition());
    textCoord->GetReferenceCoordinate()->SetCoordinateSystemToWorld();
    textCoord->GetReferenceCoordinate()->SetValue(this->GetPosition());
    this->ClampedToBorder = false;
    }

  int count = 0;
  count += this->FrameActor->RenderOpaqueGeometry(v);
  count += this->TextActor->RenderOpaqueGeometry(v);
  return count;
}

//-----------------------------------------------------------------------------
int vtkVgAnnotationActor::HasTranslucentPolygonalGeometry()
{
  if (!this->Text)
    {
    return 0;
    }

  int count = 0;
  count += this->FrameActor->HasTranslucentPolygonalGeometry();
  count += this->TextActor->HasTranslucentPolygonalGeometry();
  return count;
}

//----------------------------------------------------------------------
void vtkVgAnnotationActor::GetActors2D(vtkPropCollection* pc)
{
  pc->AddItem(this->FrameActor);
  pc->AddItem(this->TextActor);
}

//----------------------------------------------------------------------
void vtkVgAnnotationActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Annotation Text: ";

  if (this->Text)
    {
    os << this->Text << '\n';
    }
  else
    {
    os << "(none)\n";
    }
}
