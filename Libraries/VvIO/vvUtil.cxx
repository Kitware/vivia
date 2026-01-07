// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
