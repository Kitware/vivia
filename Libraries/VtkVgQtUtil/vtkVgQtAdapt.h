/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgQtAdapt_h
#define __vtkVgQtAdapt_h

#include <vgExport.h>

class QMatrix4x4;

class vtkMatrix4x4;

extern VTKVGQT_UTIL_EXPORT QMatrix4x4 qtAdapt(const vtkMatrix4x4*);
extern VTKVGQT_UTIL_EXPORT void qtAdapt(const QMatrix4x4&, vtkMatrix4x4*);

#endif
