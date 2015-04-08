/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgUtil.h"

#include <vtkMatrix4x4.h>

#include <math.h>

//-----------------------------------------------------------------------------
vgPoint2d vtkVgApplyHomography(double x, double y, const vtkMatrix4x4& xf)
{
  double p[4] = { x, y, 0.0, 1.0 };
  vtkMatrix4x4::MultiplyPoint(*xf.Element, p, p);

  const double wi = (fabs(p[3]) > 1e-10 ? 1.0 / p[3] : 1.0);
  return vgPoint2d(p[0] * wi, p[1] * wi);
}
