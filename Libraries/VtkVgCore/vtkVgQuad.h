/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgQuad_h
#define __vtkVgQuad_h

#include <vtkObject.h>

// VTK includes
#include <vtkSmartPointer.h>

class vtkActor;

class vtkVgQuad
{
public:
  static vtkSmartPointer<vtkActor> CreateQuad(double ulx, double uly,
                                              double llx, double lly,
                                              double lrx, double lry,
                                              double urx, double ury);
};

#endif // __vtkVgQuad_h
