/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvEvent_h
#define __vvEvent_h

#include "vvTrack.h"

//-----------------------------------------------------------------------------
struct vvTrackInterval
{
  vvTrackId Track;
  vgTimeStamp Start;
  vgTimeStamp Stop;
};

//-----------------------------------------------------------------------------
typedef std::map<std::string, double> vvEventClassification;
typedef std::vector<vvTrackInterval> vvTrackIntervals;
struct vvEvent
{
  long long Id;
  vvEventClassification Classification;
  vvTrackIntervals TrackIntervals;
  vgTimeStamp Start;
  vgTimeStamp Stop;
};

#endif
