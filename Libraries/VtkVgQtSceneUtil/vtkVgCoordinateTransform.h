// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgCoordinateTransform_h
#define __vtkVgCoordinateTransform_h

// VTK includes.
#include <vtkObject.h>

// VG includes.
#include <vgExport.h>
#include <vtkVgMacros.h>

// Forward declarations.
class vtkMatrix4x4;

class VTKVGQT_SCENEUTIL_EXPORT vtkVgCoordinateTransform : public vtkObject
{
public:
  vtkVgClassMacro(vtkVgCoordinateTransform);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgCoordinateTransform, vtkObject);

  static vtkVgCoordinateTransform* New();

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
  vtkVgCoordinateTransform();
  virtual  ~vtkVgCoordinateTransform();

  double FromPoint[4][2];
  double ToPoint[4][2];

private:
  vtkVgCoordinateTransform(const vtkVgCoordinateTransform&); // Not implemented.
  void operator=(const vtkVgCoordinateTransform&);           // Not implemented.
};

#endif // __vtkVgCoordinateTransform_h
