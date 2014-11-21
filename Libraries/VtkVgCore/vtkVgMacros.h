/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgMacros_h
#define __vtkVgMacros_h

// VTK includes.
#include <vtkSmartPointer.h>

#define vtkVgClassMacro(class)  \
  typedef vtkSmartPointer<class> SmartPtr;

#endif // __vtkVgMacros.h
