/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgContourRepresentation.h"

#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPointPlacer.h"
#include "vtkPoints.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"

#include "vtkVgUtil.h"

vtkStandardNewMacro(vtkVgContourRepresentation);

//------------------------------------------------------------------------
vtkVgContourRepresentation::vtkVgContourRepresentation()
{
  this->TransformMatrix = 0;
  this->TransformedPoints = vtkPoints::New();
  this->Points = vtkPoints::New();

  vtkSphereSource* ss = vtkSphereSource::New();
  ss->SetRadius(0.5);
  ss->Update();
  this->SetActiveCursorShape(ss->GetOutput());
  ss->Delete();

  this->GetProperty()->SetColor(0.25, 1.0, 0.25);

  vtkProperty* property = this->GetActiveProperty();
  property->SetRepresentationToSurface();
  property->SetAmbient(0.1);
  property->SetDiffuse(0.9);
  property->SetSpecular(0.0);
}

//------------------------------------------------------------------------
vtkVgContourRepresentation::~vtkVgContourRepresentation()
{
  if (this->TransformMatrix)
    {
    this->TransformMatrix->Delete();
    }
  this->TransformedPoints->Delete();
  this->Points->Delete();
}

//------------------------------------------------------------------------
void vtkVgContourRepresentation::Initialize(vtkPolyData* pd)
{
  this->Superclass::Initialize(pd);
  this->TransformedPoints->SetNumberOfPoints(this->GetNumberOfNodes());
  this->UpdateTransformedPoints();
}

//------------------------------------------------------------------------
void vtkVgContourRepresentation::Initialize(vtkPolyData* pd, vtkIdList* list)
{
  this->Superclass::Initialize(pd, list);
  this->TransformedPoints->SetNumberOfPoints(this->GetNumberOfNodes());
  this->UpdateTransformedPoints();
}

//------------------------------------------------------------------------
void vtkVgContourRepresentation::SetShowHandles(bool enable)
{
  double o = enable ? 1.0 : 0.0;
  this->GetProperty()->SetOpacity(o);
  this->GetActiveProperty()->SetOpacity(o);
}

//------------------------------------------------------------------------
void vtkVgContourRepresentation::Finalize()
{
  this->ActiveNode = -1;
  this->SetShowHandles(false);
}

//------------------------------------------------------------------------
void vtkVgContourRepresentation::SetCurrentOperation(int op)
{
  if (op == vtkContourRepresentation::Inactive &&
      this->GetCurrentOperation() != vtkContourRepresentation::Inactive)
    {
    this->UpdateTransformedPoints();
    }
  this->Superclass::SetCurrentOperation(op);
}

//------------------------------------------------------------------------
int vtkVgContourRepresentation::AddNodeAtDisplayPosition(double displayPos[2])
{
  if (this->Superclass::AddNodeAtDisplayPosition(displayPos))
    {
    this->TransformedPoints->SetNumberOfPoints(this->GetNumberOfNodes());
    this->UpdateTransformedPoints();
    return 1;
    }
  return 0;
}

//------------------------------------------------------------------------
int vtkVgContourRepresentation::AddNodeOnContour(int X, int Y)
{
  // Workaround crash bug in vtkContourRepresentation that shows up
  // when manipulating single-point contours.
  if (this->GetNumberOfNodes() < 2)
    {
    return 0;
    }
  if (this->Superclass::AddNodeOnContour(X, Y))
    {
    this->TransformedPoints->SetNumberOfPoints(this->GetNumberOfNodes());
    this->UpdateTransformedPoints();
    return 1;
    }
  return 0;
}

//------------------------------------------------------------------------
int vtkVgContourRepresentation::DeleteNthNode(int n)
{
  if (this->Superclass::DeleteNthNode(n))
    {
    this->TransformedPoints->SetNumberOfPoints(this->GetNumberOfNodes());
    this->UpdateTransformedPoints();
    return 1;
    }
  return 0;
}

//------------------------------------------------------------------------
void vtkVgContourRepresentation::SetTransformMatrix(vtkMatrix4x4* mat)
{
  if (!this->TransformMatrix)
    {
    this->TransformMatrix = vtkMatrix4x4::New();
    }
  this->TransformMatrix->DeepCopy(mat);

  // Don't attempt to transform points if the contour is being dragged.
  if (this->GetCurrentOperation() == vtkContourRepresentation::Shift)
    {
    return;
    }

  // Compute the new world positions of the contour nodes based on their
  // initial transformed coordinates.
  for (int i = 0, end = this->TransformedPoints->GetNumberOfPoints();
       i < end; ++i)
    {
    // Don't fight the widget for control of the currently active node.
    if (i == this->ActiveNode)
      {
      continue;
      }

    double point[3];
    this->TransformedPoints->GetPoint(i, point);
    vtkVgApplyHomography(point, this->TransformMatrix, point);
    point[2] = 0.0;

    this->SetNthNodeWorldPosition(i, point);
    }
}

//------------------------------------------------------------------------
void vtkVgContourRepresentation::UpdateTransformedPoints()
{
  if (!this->TransformedPoints)
    {
    return;
    }

  vtkMatrix4x4* invertMat = vtkMatrix4x4::New();

  if (this->TransformMatrix)
    {
    invertMat->DeepCopy(this->TransformMatrix);
    invertMat->Invert();
    }
  else
    {
    invertMat->Identity();
    }

  for (int i = 0, end = this->TransformedPoints->GetNumberOfPoints();
       i < end; ++i)
    {
    double pos[3];
    this->GetNthNodeWorldPosition(i, pos);
    vtkVgApplyHomography(pos, invertMat, pos);
    pos[2] = 0.0;

    this->TransformedPoints->SetPoint(i, pos);
    }

  invertMat->Delete();
}

//------------------------------------------------------------------------
vtkPoints* vtkVgContourRepresentation::GetPoints()
{
  if (this->PointsBuildTime > this->TransformedPoints->GetMTime())
    {
    return this->Points;
    }

  vtkIdType numPts = this->TransformedPoints->GetNumberOfPoints();

  this->Points->Reset();
  this->Points->Allocate(numPts);

  // Remove (implicit) degenerate lines, which will cause problems for the
  // line and polygon intersection tests used by vtkImplicitSelectionLoop.
  for (int i = 0; i < numPts; ++i)
    {
    double a[3];
    this->TransformedPoints->GetPoint(i, a);
    if (i != 0)
      {
      double b[3];
      this->TransformedPoints->GetPoint(i - 1, b);
      if (a[0] == b[0] && a[1] == b[1] && a[2] == b[2])
        {
        continue;
        }
      }
    this->Points->InsertNextPoint(a);
    }

  this->PointsBuildTime.Modified();
  return this->Points;
}

//------------------------------------------------------------------------
int vtkVgContourRepresentation::FindClosestPoint(int X, int Y, double pos[3])
{
  int idx;
  return this->FindClosestPointOnContour(X, Y, pos, &idx);
}

//------------------------------------------------------------------------
void vtkVgContourRepresentation::WidgetInteraction(double eventPos[2])
{
  if (this->CurrentOperation == vtkContourRepresentation::Scale)
    {
    // Call our corrected version of the vtkOrientedGlyphContourRep's
    // ScaleContour function.
    this->ScaleContourFixed(eventPos);
    this->LastEventPosition[0] = eventPos[0];
    this->LastEventPosition[1] = eventPos[1];
    return;
    }

  this->Superclass::WidgetInteraction(eventPos);
}

//----------------------------------------------------------------------
void vtkVgContourRepresentation::ScaleContourFixed(
  double eventPos[2])
{
  double ref[3];

  if (!this->GetActiveNodeWorldPosition(ref))
    {
    return;
    }

  double centroid[3];
  ComputeCentroid(centroid);

  double r2 = vtkMath::Distance2BetweenPoints(ref, centroid);

  double displayPos[2];
  displayPos[0] = eventPos[0] + this->InteractionOffset[0];
  displayPos[1] = eventPos[1] + this->InteractionOffset[1];

  double worldPos[3];
  double worldOrient[9] = {1.0, 0.0, 0.0,
                           0.0, 1.0, 0.0,
                           0.0, 0.0, 1.0
                          };
  if (this->PointPlacer->ComputeWorldPosition(this->Renderer.GetPointer(),
                                              displayPos, ref, worldPos,
                                              worldOrient))
    {
    double d2 = vtkMath::Distance2BetweenPoints(worldPos, centroid);
    if (d2 != 0.)
      {
      double ratio = sqrt(d2 / r2);
      for (int i = 0; i < this->GetNumberOfNodes(); i++)
        {
        this->GetNthNodeWorldPosition(i, ref);
        worldPos[0] = centroid[0] + ratio * (ref[0] - centroid[0]);
        worldPos[1] = centroid[1] + ratio * (ref[1] - centroid[1]);
        worldPos[2] = centroid[2] + ratio * (ref[2] - centroid[2]);
        this->SetNthNodeWorldPosition(i, worldPos, worldOrient);
        }
      }
    }
}
