/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
