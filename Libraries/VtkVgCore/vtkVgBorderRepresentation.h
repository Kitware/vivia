// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// Allows a border widget to use world coordinates rather than the default
// normalized viewport coords.

#ifndef __vtkVgBorderRepresentation_h
#define __vtkVgBorderRepresentation_h

#include "vtkBorderRepresentation.h"

#include <vgExport.h>

class vtkCoordinate;

class VTKVG_CORE_EXPORT vtkVgBorderRepresentation
  : public vtkBorderRepresentation
{
public:
  static vtkVgBorderRepresentation* New();

  vtkTypeMacro(vtkVgBorderRepresentation, vtkBorderRepresentation);

  virtual void BuildRepresentation();
  virtual void StartWidgetInteraction(double eventPos[2]);
  virtual void WidgetInteraction(double eventPos[2]);

  // Set the corners of the rectangle in display coordinates
  void SetDisplayPoint1(int p1[2]);
  void SetDisplayPoint2(int p2[2]);

protected:
  vtkVgBorderRepresentation();
  ~vtkVgBorderRepresentation();

  void SetDisplayPoint(int p[2], double world[2]);
  void SetCoordValue(double p[2], vtkCoordinate* coord);

  double Point1[2];
  double Point2[2];
};

#endif
