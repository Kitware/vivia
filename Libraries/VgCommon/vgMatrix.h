// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgMatrix_h
#define __vgMatrix_h

#include <Eigen/Core>

using vgVector3d = Eigen::Vector3d;
using vgVector4d = Eigen::Vector4d;

// Row-major storage is used for consistency with VTK, VNL to allow copies to
// be made by directly copying the underlying data block

using vgMatrix3d = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>;
using vgMatrix4d = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>;

#endif
