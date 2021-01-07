// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgVideoFrame_h
#define __vtkVgVideoFrame_h

#include <vtkImageData.h>
#include <vtkSmartPointer.h>

#include <vgExport.h>

#include "vtkVgPythonUtil.h"
#include "vtkVgSharedInstance.h"
#include "vtkVgVideoFrameMetaData.h"

struct VTKVG_CORE_EXPORT vtkVgVideoFrame
{
  vtkVgVideoFrame();
  explicit vtkVgVideoFrame(const vtkSmartPointer<vtkImageData>& sptr);

  vtkVgSharedInstance<vtkImageData> Image;
  vtkVgPythonAttribute(vtkVgVideoFrameMetaData, MetaData);

public: // Accessor method for Python wrapping
  // Description:
  // Get mutable pointer to image data
  vtkImageData* GetImageData();
};

#endif
