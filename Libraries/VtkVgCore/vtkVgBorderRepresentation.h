/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
