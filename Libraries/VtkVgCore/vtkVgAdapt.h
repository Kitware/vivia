/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgAdapt_h
#define __vtkVgAdapt_h

#include <vgMatrix.h>

#include <vgExport.h>

#include <vtkSmartPointer.h>

#include <vil/vil_image_view.h>
#include <vnl/vnl_matrix_fixed.h>

class vtkImageData;
class vtkMatrix4x4;

extern VTKVG_CORE_EXPORT void
vtkVgAdapt(const vnl_matrix_fixed<double, 3, 3>& in, vtkMatrix4x4* out);

extern VTKVG_CORE_EXPORT vtkSmartPointer<vtkMatrix4x4>
vtkVgAdapt(const vnl_matrix_fixed<double, 3, 3>&);

extern VTKVG_CORE_EXPORT vgMatrix4d
vtkVgAdapt(const vtkMatrix4x4*);

extern VTKVG_CORE_EXPORT void
vtkVgAdapt(const vgMatrix4d& in, vtkMatrix4x4* out);

extern VTKVG_CORE_EXPORT vtkSmartPointer<vtkMatrix4x4>
vtkVgAdapt(const vgMatrix4d&);

extern VTKVG_CORE_EXPORT void
vtkVgAdapt(const vil_image_view<double>& img, vtkImageData* data);

extern VTKVG_CORE_EXPORT void
vtkVgAdapt(vtkImageData* data, vil_image_view<vxl_byte>& img);

#endif
