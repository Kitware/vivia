// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpVidtkFileTrackIO.h"

#include "vpVidtkFileReader.h"

//-----------------------------------------------------------------------------
vpVidtkFileTrackIO::vpVidtkFileTrackIO(
  vpVidtkFileReader& reader,
  std::map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
  std::map<unsigned int, vtkIdType>& sourceIdToModelIdMap,
  vtkVpTrackModel* trackModel,
  TrackStorageMode storageMode,
  bool interpolateToGround,
  TrackTimeStampMode timeStampMode,
  vtkVgTrackTypeRegistry* trackTypes,
  vgAttributeSet* trackAttributes,
  vtkMatrix4x4* geoTransform,
  vpFrameMap* frameMap) :
  vpVidtkTrackIO{reader, trackMap, sourceIdToModelIdMap, trackModel,
                 storageMode, interpolateToGround, timeStampMode, trackTypes,
                 trackAttributes, geoTransform, frameMap},
  FileReader{this}
{}

//-----------------------------------------------------------------------------
bool vpVidtkFileTrackIO::ReadTracks(int frameOffset)
{
  auto& reader = static_cast<const vpVidtkFileReader&>(this->GetReader());
  const auto& tracksFileName = reader.GetTracksFileName();

  vpFileTrackReader::TrackRegionMap trackRegionMap;
  this->FileReader.ReadRegionsFile(
    tracksFileName, 0.0f, 0.0f, trackRegionMap);

  if (!vpVidtkTrackIO::ReadTracks(frameOffset, &trackRegionMap))
    {
    return false;
    }

  this->FileReader.ReadTypesFile(tracksFileName);
  this->FileReader.ReadAttributesFile(tracksFileName, this->TrackAttributes);

  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkFileTrackIO::ImportTracks(int frameOffset, vtkIdType idsOffset,
                                      float offsetX, float offsetY)
{
  auto& reader = static_cast<const vpVidtkFileReader&>(this->GetReader());
  const auto& tracksFileName = reader.GetTracksFileName();

  vpFileTrackReader::TrackRegionMap trackRegionMap;
  this->FileReader.ReadRegionsFile(
    tracksFileName, offsetX, offsetY, trackRegionMap);

  if (!vpVidtkTrackIO::ImportTracks(&trackRegionMap, frameOffset,
                                    idsOffset, offsetX, offsetY))
    {
    return false;
    }

  this->FileReader.ReadTypesFile(tracksFileName);
  this->FileReader.ReadAttributesFile(tracksFileName, this->TrackAttributes);

  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkFileTrackIO::ReadTrackTraits()
{
  const auto& reader =
    static_cast<const vpVidtkFileReader&>(this->GetReader());
  return this->FileReader.ReadTrackTraits(reader.GetTrackTraitsFileName());
}

//-----------------------------------------------------------------------------
bool vpVidtkFileTrackIO::ReadTrackClassifiers()
{
  auto& reader =
    static_cast<const vpVidtkFileReader&>(this->GetReader());
  return this->FileReader.ReadTrackClassifiers(
    reader.GetTrackClassifiersFileName());
}
