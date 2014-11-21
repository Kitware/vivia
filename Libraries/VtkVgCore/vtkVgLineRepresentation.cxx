/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgLineRepresentation.h"

#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>

#include "vtkVgUtil.h"

vtkStandardNewMacro(vtkVgLineRepresentation);

//------------------------------------------------------------------------
vtkVgLineRepresentation::vtkVgLineRepresentation()
{
  this->TransformMatrix = 0;
  this->PointsAreValid = false;

  this->SetLineColor(1.0, 1.0, 0.0);
  this->GetEndPointProperty()->SetColor(1.0, 0.0, 0.0);
  this->GetEndPoint2Property()->SetColor(1.0, 0.0, 0.0);
}

//------------------------------------------------------------------------
vtkVgLineRepresentation::~vtkVgLineRepresentation()
{
  if (this->TransformMatrix)
    {
    this->TransformMatrix->Delete();
    }
}

//------------------------------------------------------------------------
void vtkVgLineRepresentation::WidgetInteraction(double e[2])
{
  this->Superclass::WidgetInteraction(e);
  this->UpdateTransformedPoints();
}

//------------------------------------------------------------------------
void vtkVgLineRepresentation::PlaceWidget(double bounds[6])
{
  this->Superclass::PlaceWidget(bounds);
  this->UpdateTransformedPoints();
}

//------------------------------------------------------------------------
void vtkVgLineRepresentation::SetTransformMatrix(vtkMatrix4x4* mat)
{
  if (!this->TransformMatrix)
    {
    this->TransformMatrix = vtkMatrix4x4::New();
    }
  this->TransformMatrix->DeepCopy(mat);

  if (!this->PointsAreValid)
    {
    return;
    }

  // Compute the new world positions of the line endpoints based on their
  // initial transformed coordinates. Don't attempt to transform points that
  // are being dragged.
  if (this->InteractionState != vtkLineRepresentation::TranslatingP1)
    {
    double point[3];
    vtkVgApplyHomography(this->TransformedPoint1, this->TransformMatrix, point);
    point[2] = this->TransformedPoint1[2];
    this->SetPoint1WorldPosition(point);
    }

  if (this->InteractionState != vtkLineRepresentation::TranslatingP2)
    {
    double point[3];
    vtkVgApplyHomography(this->TransformedPoint2, this->TransformMatrix, point);
    point[2] = this->TransformedPoint2[2];
    this->SetPoint2WorldPosition(point);
    }
}

//------------------------------------------------------------------------
void vtkVgLineRepresentation::UpdateTransformedPoints()
{
  vtkMatrix4x4* invertMat = vtkMatrix4x4::New();

  if (this->TransformMatrix)
    {
    invertMat->DeepCopy(this->TransformMatrix);
    invertMat->Invert();
    }

    {
    double pos[3];
    this->GetPoint1WorldPosition(pos);
    vtkVgApplyHomography(pos, invertMat, this->TransformedPoint1);
    this->TransformedPoint1[2] = 0.0;
    }

    {
    double pos[3];
    this->GetPoint2WorldPosition(pos);
    vtkVgApplyHomography(pos, invertMat, this->TransformedPoint2);
    this->TransformedPoint2[2] = 0.0;
    }

  invertMat->Delete();
  this->PointsAreValid = true;
}
