// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
