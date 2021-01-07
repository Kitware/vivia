// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgLineRepresentation_h
#define __vtkVgLineRepresentation_h

#include <vtkLineRepresentation.h>

#include <vgExport.h>

class vtkMatrix4x4;

class VTKVG_CORE_EXPORT vtkVgLineRepresentation : public vtkLineRepresentation
{
public:
  vtkTypeMacro(vtkVgLineRepresentation, vtkLineRepresentation);

  static vtkVgLineRepresentation* New();

  void SetTransformMatrix(vtkMatrix4x4* mat);

  virtual void WidgetInteraction(double e[2]);

  virtual void PlaceWidget(double bounds[6]);

protected:
  vtkVgLineRepresentation();
  ~vtkVgLineRepresentation();

  void UpdateTransformedPoints();

private:
  vtkMatrix4x4* TransformMatrix;
  double TransformedPoint1[3];
  double TransformedPoint2[3];
  bool PointsAreValid;
};

#endif
