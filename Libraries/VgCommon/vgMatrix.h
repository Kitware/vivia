/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
