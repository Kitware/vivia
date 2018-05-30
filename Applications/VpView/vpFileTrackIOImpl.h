/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileTrackIOImpl_h
#define __vpFileTrackIOImpl_h

#include <string>
#include <unordered_map>
#include <vector>

class vpTrackIO;

class vpFileTrackIOImpl
{
public:
  struct FrameRegionInfo
    {
    bool KeyFrame;
    int NumberOfPoints;
    std::vector<float> Points;
    };

  using TrackRegions = std::unordered_map<unsigned int, FrameRegionInfo>;
  using TrackRegionMap = std::unordered_map<unsigned int, TrackRegions>;

  static bool ReadTrackTraits(vpTrackIO* io,
                              const std::string& trackTraitsFileName);

  static bool ReadRegionsFile(vpTrackIO* io,
                              const std::string& tracksFileName,
                              float offsetX, float offsetY,
                              TrackRegionMap& trackRegionMap);

  static void ReadTypesFile(vpTrackIO* io, const std::string& tracksFileName);
};

#endif // __vpFileTrackIOImpl_h
