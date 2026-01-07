// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsAlert_h
#define __vsAlert_h

#include <vvQuery.h>

#include <vsEventInfo.h>

struct vsAlert
{
  vsEventInfo eventInfo;
  vvSimilarityQuery query;
  double displayThreshold;
};

#endif
