// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgVideoFrameData_h
#define __vtkVgVideoFrameData_h

#include "vtkVgTimeStamp.h"

// VTK includes.
#include <vtkSmartPointer.h>

#include <vgExport.h>

// Forward declarations.
class vtkImageData;
class vtkMatrix4x4;

struct VTKVG_MODELVIEW_EXPORT vtkVgVideoFrameData
{
  vtkVgVideoFrameData();
  ~vtkVgVideoFrameData();

  vtkVgTimeStamp                      TimeStamp;
  vtkSmartPointer<vtkImageData>       VideoImage;
  vtkSmartPointer<vtkMatrix4x4>       VideoMatrix;
};

#endif // __vtkVgVideoFrameData_h
