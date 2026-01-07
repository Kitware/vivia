// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
