/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgCellPicker_h
#define __vtkVgCellPicker_h

#include <vtkCellPicker.h>

// VTK includes.
#include <vtkSmartPointer.h>

#include <vgExport.h>

// Forward declarations.
class vtkPerspectiveTransform;

class VTKVG_CORE_EXPORT vtkVgCellPicker : public vtkCellPicker
{
public:
  static vtkVgCellPicker* New();
  vtkTypeMacro(vtkVgCellPicker, vtkCellPicker);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int Pick(double selectionX, double selectionY,
                   double selectionZ, vtkRenderer* renderer);


protected:
  vtkVgCellPicker();
  virtual ~vtkVgCellPicker();

  virtual double IntersectActorWithLine(const double p1[3], const double p2[3],
                                        double t1, double t2, double tol,
                                        vtkProp3D* prop, vtkMapper* mapper);

  vtkSmartPointer<vtkPerspectiveTransform> PerspectiveTransform;


private:
  // Not implemented.
  vtkVgCellPicker(const vtkVgCellPicker&);

  vtkGenericCell* Cell; // used to accelerate picking
  vtkIdList* PointIds; // used to accelerate picking

  // Not implemented.
  void operator=(const vtkVgCellPicker&);
};

#endif // __vtkVgCellPicker_h
