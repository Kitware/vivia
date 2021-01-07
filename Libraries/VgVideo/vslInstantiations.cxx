// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <vnl/io/vnl_io_vector_fixed.hxx>
#include <vsl/vsl_vector_io.hxx>

VNL_IO_VECTOR_FIXED_INSTANTIATE(double, 2);
VSL_VECTOR_IO_INSTANTIATE(vnl_vector_fixed<double VCL_COMMA 2>);
