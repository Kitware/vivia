/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
  return vpFileTrackIOImpl::ReadTrackClassifiers(
           this, reader.GetTrackClassifiersFileName());
}
