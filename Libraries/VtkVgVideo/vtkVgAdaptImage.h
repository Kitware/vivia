// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgAdaptImage_h
#define __vtkVgAdaptImage_h

#include <vtkSmartPointer.h>

#include <vgExport.h>

class vtkImageData;

class vgImage;

extern VTKVG_VIDEO_EXPORT vtkSmartPointer<vtkImageData>
vtkVgAdapt(const vgImage&);

#endif
