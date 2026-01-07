// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgVideoFrameCorner_h
#define __vtkVgVideoFrameCorner_h

#include <vtkVgPythonUtil.h>

//-----------------------------------------------------------------------------
struct vtkVgVideoFrameCorner
{
  vtkVgPythonAttribute(double, Latitude);
  vtkVgPythonAttribute(double, Longitude);
};

#endif
