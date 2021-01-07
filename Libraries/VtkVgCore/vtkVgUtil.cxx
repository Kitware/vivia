// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
