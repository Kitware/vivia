// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
