/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
