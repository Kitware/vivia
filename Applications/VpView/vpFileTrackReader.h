/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileTrackReader_h
#define __vpFileTrackReader_h

#include <string>
#include <unordered_map>
#include <vector>

class vgAttributeSet;

class vpTrackIO;

class vpFileTrackReader
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

  vpFileTrackReader(vpTrackIO* io);

  bool ReadTrackTraits(const std::string& trackTraitsFileName) const;

  bool ReadTrackClassifiers(const std::string& trackClassifiersFileName) const;

  bool ReadRegionsFile(const std::string& tracksFileName,
                       float offsetX, float offsetY,
                       TrackRegionMap& trackRegionMap) const;

  bool ReadAttributesFile(const std::string& tracksFileName,
                          vgAttributeSet* trackAttributes) const;

  void ReadTypesFile(const std::string& tracksFileName) const;

protected:
  vpTrackIO* const IO;
};

#endif
