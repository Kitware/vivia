// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgMacros_h
#define __vtkVgMacros_h

// VTK includes.
#include <vtkSmartPointer.h>

#define vtkVgClassMacro(class)  \
  typedef vtkSmartPointer<class> SmartPtr;

#endif // __vtkVgMacros.h
