/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgUtil_h
#define __vtkVgUtil_h

#include "vtkVgInstance.h"

#include <vgExport.h>

#include <vector>

class vtkMatrix4x4;

struct vgPoint2d
{
  vgPoint2d(double x = 0.0, double y = 0.0) : X(x), Y(y) {}
  double X;
  double Y;
};

inline bool operator==(const vgPoint2d& a, const vgPoint2d& b)
{
  return (a.X == b.X) && (a.Y == b.Y);
}

typedef std::vector<vgPoint2d> vgPolygon2d;

extern VTKVG_CORE_EXPORT vgPoint2d vtkVgApplyHomography(
  double x, double y, const vtkMatrix4x4& xf);

inline vgPoint2d vtkVgApplyHomography(
  double x, double y, const vtkMatrix4x4* xf)
{ return (xf ? vtkVgApplyHomography(x, y, *xf) : vgPoint2d()); }

inline vgPoint2d vtkVgApplyHomography(
  double x, double y, const vtkVgInstance<vtkMatrix4x4>& xf)
{ return vtkVgApplyHomography(x, y, *xf); }

#define IMPLEMENT_APPLY_HOMOGRAPHY(MatrixType) \
inline vgPoint2d vtkVgApplyHomography(const double in[2], MatrixType xf) \
{ \
  return vtkVgApplyHomography(in[0], in[1], xf); \
} \
\
inline void vtkVgApplyHomography( \
  double inX, double inY, MatrixType xf, double result[2]) \
{ \
  const vgPoint2d p = vtkVgApplyHomography(inX, inY, xf); \
  result[0] = p.X; \
  result[1] = p.Y; \
} \
\
inline void vtkVgApplyHomography( \
  const vgPoint2d& in, MatrixType xf, double result[2]) \
{ \
  const vgPoint2d p = vtkVgApplyHomography(in.X, in.Y, xf); \
  result[0] = p.X; \
  result[1] = p.Y; \
} \
\
inline void vtkVgApplyHomography( \
  const double in[2], MatrixType xf, double result[2]) \
{ \
  const vgPoint2d p = vtkVgApplyHomography(in[0], in[1], xf); \
  result[0] = p.X; \
  result[1] = p.Y; \
} \
\
inline void vtkVgApplyHomography( \
  double inX, double inY, MatrixType xf, double& outX, double& outY) \
{ \
  const vgPoint2d p = vtkVgApplyHomography(inX, inY, xf); \
  outX = p.X; \
  outY = p.Y; \
} \
\
inline void vtkVgApplyHomography( \
  const vgPoint2d& in, MatrixType xf, double& outX, double& outY) \
{ \
  const vgPoint2d p = vtkVgApplyHomography(in.X, in.Y, xf); \
  outX = p.X; \
  outY = p.Y; \
} \
\
inline void vtkVgApplyHomography( \
  const double in[2], MatrixType xf, double& outX, double& outY) \
{ \
  const vgPoint2d p = vtkVgApplyHomography(in[0], in[1], xf); \
  outX = p.X; \
  outY = p.Y; \
}

IMPLEMENT_APPLY_HOMOGRAPHY(const vtkMatrix4x4&)
IMPLEMENT_APPLY_HOMOGRAPHY(const vtkMatrix4x4*)
IMPLEMENT_APPLY_HOMOGRAPHY(const vtkVgInstance<vtkMatrix4x4>&)

#undef IMPLEMENT_APPLY_HOMOGRAPHY

#endif
