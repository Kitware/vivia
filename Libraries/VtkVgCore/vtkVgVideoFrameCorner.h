/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
