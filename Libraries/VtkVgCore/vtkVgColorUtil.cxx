/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
