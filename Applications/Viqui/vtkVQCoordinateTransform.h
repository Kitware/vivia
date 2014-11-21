/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVQCoordinateTransform_h
#define __vtkVQCoordinateTransform_h

// VTK includes.
#include <vtkObject.h>

// VG includes.
#include "vtkVgMacros.h"

// Forward declarations.
class vtkMatrix4x4;

class vtkVQCoordinateTransform : public vtkObject
{
public:
  vtkVgClassMacro(vtkVQCoordinateTransform);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVQCoordinateTransform, vtkObject);

  static vtkVQCoordinateTransform* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  void SetFromPoints(double pt1[2], double pt2[2], double pt3[2], double pt4[2]);
  void SetFromPoints(double x1, double y1,
                     double x2, double y2,
                     double x3, double y3,
                     double x4, double y4);

  void SetToPoints(double pt1[2], double pt2[2], double pt3[2], double pt4[2]);
  void SetToPoints(double x1, double y1,
                   double x2, double y2,
                   double x3, double y3,
                   double x4, double y4);

  vtkSmartPointer<vtkMatrix4x4> GetHomographyMatrix();

protected:
  vtkVQCoordinateTransform();
  virtual  ~vtkVQCoordinateTransform();

  double FromPoint[4][2];
  double ToPoint[4][2];


private:
  vtkVQCoordinateTransform(const vtkVQCoordinateTransform&); // Not implemented.
  void operator=(const vtkVQCoordinateTransform&);           // Not implemented.
};

#endif // __vtkVQCoordinateTransform_h
