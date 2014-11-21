/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgColorUtil_h
#define __vtkVgColorUtil_h

#include <vgExport.h>

namespace vtkVgColorUtil
{
  VTKVG_CORE_EXPORT void convert(const double in[3], unsigned char (&out)[3]);
  VTKVG_CORE_EXPORT void convert(const double in[4], unsigned char (&out)[4]);

  VTKVG_CORE_EXPORT void convertMultiplied(
    const double in[3], double multiplier, unsigned char (&out)[3]);
  VTKVG_CORE_EXPORT void convertMultiplied(
    const double in[4], double multiplier, unsigned char (&out)[4]);

  VTKVG_CORE_EXPORT double clamp(double a, double min = 0.0, double max = 1.0);
};

#endif // __vtkVgColorUtil_h
