/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgAdaptImage_h
#define __vtkVgAdaptImage_h

#include <vtkSmartPointer.h>

#include <vgExport.h>

class vtkImageData;

class vgImage;

extern VTKVG_VIDEO_EXPORT vtkSmartPointer<vtkImageData>
vtkVgAdapt(const vgImage&);

#endif
