/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgVideoFrameData.h"

// VTK includes.
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"

//-----------------------------------------------------------------------------
vtkVgVideoFrameData::vtkVgVideoFrameData()
{
  this->VideoImage  = vtkSmartPointer<vtkImageData>::New();
  this->VideoMatrix = NULL;
}

//-----------------------------------------------------------------------------
vtkVgVideoFrameData::~vtkVgVideoFrameData()
{
}
