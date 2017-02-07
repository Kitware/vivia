/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileTrackIOImpl_h
#define __vpFileTrackIOImpl_h

#include <map>
#include <string>
#include <vector>

class vpTrackIO;

class vpFileTrackIOImpl
{
public:
  struct FrameRegionInfo
    {
    bool KeyFrame;
    std::vector<float> Points;
    };

  typedef std::map<int, std::map<int, FrameRegionInfo> > TrackRegionMapType;
  TrackRegionMapType TrackRegionMap;

  static bool ReadTrackTraits(vpTrackIO* io,
                              const std::string& trackTraitsFileName);

  static bool ReadRegionsFile(vpTrackIO* io,
                              const std::string& tracksFileName,
                              float offsetX, float offsetY,
                              TrackRegionMapType* trackRegionMap);

  static void ReadTypesFile(vpTrackIO* io, const std::string& tracksFileName);
};

#endif // __vpFileTrackIOImpl_h
