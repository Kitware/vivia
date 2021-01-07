// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgColorUtil.h"

//-----------------------------------------------------------------------------
double vtkVgColorUtil::clamp(double a, double min, double max)
{
  return (a < min ? min : a > max ? max : a);
}

//-----------------------------------------------------------------------------
void vtkVgColorUtil::convert(const double in[3], unsigned char (&out)[3])
{
  out[0] = static_cast<unsigned char>(clamp(in[0]) * 255.0);
  out[1] = static_cast<unsigned char>(clamp(in[1]) * 255.0);
  out[2] = static_cast<unsigned char>(clamp(in[2]) * 255.0);
}

//-----------------------------------------------------------------------------
void vtkVgColorUtil::convert(const double in[4], unsigned char (&out)[4])
{
  out[0] = static_cast<unsigned char>(clamp(in[0]) * 255.0);
  out[1] = static_cast<unsigned char>(clamp(in[1]) * 255.0);
  out[2] = static_cast<unsigned char>(clamp(in[2]) * 255.0);
  out[3] = static_cast<unsigned char>(clamp(in[3]) * 255.0);
}

//-----------------------------------------------------------------------------
void vtkVgColorUtil::convertMultiplied(
  const double in[3], double multiplier, unsigned char (&out)[3])
{
  out[0] = static_cast<unsigned char>(clamp(in[0] * multiplier) * 255.0);
  out[1] = static_cast<unsigned char>(clamp(in[1] * multiplier) * 255.0);
  out[2] = static_cast<unsigned char>(clamp(in[2] * multiplier) * 255.0);
}

//-----------------------------------------------------------------------------
void vtkVgColorUtil::convertMultiplied(
  const double in[4], double multiplier, unsigned char (&out)[4])
{
  out[0] = static_cast<unsigned char>(clamp(in[0] * multiplier) * 255.0);
  out[1] = static_cast<unsigned char>(clamp(in[1] * multiplier) * 255.0);
  out[2] = static_cast<unsigned char>(clamp(in[2] * multiplier) * 255.0);
  out[3] = static_cast<unsigned char>(clamp(in[3] * multiplier) * 255.0);
}
