// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
