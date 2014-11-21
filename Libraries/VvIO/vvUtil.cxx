/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvUtil.h"

#include <vvDescriptor.h>

//-----------------------------------------------------------------------------
std::size_t vvUtil::findMergedRegions(
  const std::vector<vvDescriptor>& descriptors)
{
  const std::size_t k = descriptors.size();
  for (std::size_t n = 0; n < k; ++n)
    {
    const vvDescriptor& d = descriptors[n];
    if (d.ModuleName == "MergedDescriptors" && d.DescriptorName == "metadata")
      return n;
    }

  return k;
}
