/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
