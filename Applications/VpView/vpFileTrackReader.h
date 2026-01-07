// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpFileTrackReader_h
#define __vpFileTrackReader_h

#include <unordered_map>
#include <vector>

class QString;

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

  bool ReadTrackTraits(const QString& trackTraitsFileName) const;

  bool ReadTrackClassifiers(const QString& trackClassifiersFileName) const;

  bool ReadRegionsFile(const QString& tracksFileName,
                       float offsetX, float offsetY,
                       TrackRegionMap& trackRegionMap) const;

  bool ReadAttributesFile(const QString& tracksFileName,
                          vgAttributeSet* trackAttributes) const;

  void ReadTypesFile(const QString& tracksFileName) const;

protected:
  vpTrackIO* const IO;
};

#endif
