/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgVideoFrameCorners_h
#define __vtkVgVideoFrameCorners_h

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

#include <vgExport.h>

#include "vtkVgVideoFrameCorner.h"

//-----------------------------------------------------------------------------
struct vtkVgVideoFrameCorners
{
  vtkVgVideoFrameCorners() : GCS(-1) {}

  vtkVgPythonAttribute(int, GCS);
  vtkVgPythonAttribute(vtkVgVideoFrameCorner, UpperLeft);
  vtkVgPythonAttribute(vtkVgVideoFrameCorner, UpperRight);
  vtkVgPythonAttribute(vtkVgVideoFrameCorner, LowerLeft);
  vtkVgPythonAttribute(vtkVgVideoFrameCorner, LowerRight);
};

#endif
