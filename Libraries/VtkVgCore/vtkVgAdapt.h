/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgAdapt_h
#define __vtkVgAdapt_h

#include <vtkSmartPointer.h>

#include <vil/vil_image_view.h>
#include <vnl/vnl_matrix_fixed.h>

#include <vgExport.h>

class vtkImageData;
class vtkMatrix4x4;

extern VTKVG_CORE_EXPORT void
vtkVgAdapt(const vnl_matrix_fixed<double, 3, 3>& in, vtkMatrix4x4* out);

extern VTKVG_CORE_EXPORT vtkSmartPointer<vtkMatrix4x4>
vtkVgAdapt(const vnl_matrix_fixed<double, 3, 3>&);

extern VTKVG_CORE_EXPORT void
vtkVgAdapt(const vil_image_view<double>& img, vtkImageData* data);

extern VTKVG_CORE_EXPORT void
vtkVgAdapt(vtkImageData* data, vil_image_view<vxl_byte>& img);

#endif
