// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgUnixTime_h
#define __vgUnixTime_h

#include "vgTime.h"

struct UnixTimeTraits
{
  enum { EpochOffset = 0 };
};

typedef vgTime<UnixTimeTraits> vgUnixTime;

#endif
