// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvQueryResult_h
#define __vvQueryResult_h

#include <string>
#include <vector>

#include <vgGeoTypes.h>

#include "vvDescriptor.h"
#include "vvIqr.h"
#include "vvUserData.h"

//-----------------------------------------------------------------------------
struct vvQueryResult
{
  vvQueryResult()
    : InstanceId(-1), StartTime(0), EndTime(0), Rank(-1),
      RelevancyScore(-1.0), PreferenceScore(-1.0),
      UserScore(vvIqr::UnclassifiedExample) {}

  std::string MissionId;
  std::string QueryId;
  std::string StreamId;
  long long InstanceId;
  std::vector<vvTrack> Tracks;
  std::vector<vvDescriptor> Descriptors;
  long long StartTime;
  long long EndTime;
  vgGeocodedCoordinate Location;
  long long Rank;
  double RelevancyScore;
  double PreferenceScore;
  vvIqr::Classification UserScore;
  vvUserData::Data UserData;
};

#endif
