// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgVideoFrame.h"

//-----------------------------------------------------------------------------
vtkVgVideoFrame::vtkVgVideoFrame()
{
}

//-----------------------------------------------------------------------------
vtkVgVideoFrame::vtkVgVideoFrame(const vtkSmartPointer<vtkImageData>& sptr) :
  Image(sptr)
{
  sptr->Register(sptr);
}

//-----------------------------------------------------------------------------
vtkImageData* vtkVgVideoFrame::GetImageData()
{
  return this->Image.GetPointer();
}
