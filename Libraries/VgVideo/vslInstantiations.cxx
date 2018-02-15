/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vnl/io/vnl_io_vector_fixed.hxx>
#include <vsl/vsl_vector_io.hxx>

VNL_IO_VECTOR_FIXED_INSTANTIATE(double, 2);
VSL_VECTOR_IO_INSTANTIATE(vnl_vector_fixed<double VCL_COMMA 2>);
