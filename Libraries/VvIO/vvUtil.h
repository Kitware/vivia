/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
