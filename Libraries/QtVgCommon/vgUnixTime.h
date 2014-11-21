/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgUnixTime_h
#define __vgUnixTime_h

#include "vgTime.h"

struct UnixTimeTraits
{
  enum { EpochOffset = 0 };
};

typedef vgTime<UnixTimeTraits> vgUnixTime;

#endif
