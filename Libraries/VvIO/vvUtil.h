// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvUtil_h
#define __vvUtil_h

#include <vector>

#include <vgExport.h>

struct vvDescriptor;

namespace vvUtil
{
  extern VV_IO_EXPORT std::size_t findMergedRegions(
    const std::vector<vvDescriptor>&);
}

#endif
