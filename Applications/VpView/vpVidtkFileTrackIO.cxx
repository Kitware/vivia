/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkFileTrackIO.h"

#include "vpFileTrackIOImpl.h"
#include "vpVidtkFileReader.h"

//-----------------------------------------------------------------------------
vpVidtkFileTrackIO::vpVidtkFileTrackIO(
  vpVidtkFileReader& reader,
  vcl_map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
  vcl_map<unsigned int, vtkIdType>& sourceIdToModelIdMap,
  vtkVgTrackModel* trackModel,
  TrackStorageMode storageMode,
  TrackTimeStampMode timeStampMode,
  vtkVgTrackTypeRegistry* trackTypes,
  vtkMatrix4x4* geoTransform,
  vpFrameMap* frameMap) :
  vpVidtkTrackIO(reader, trackMap, sourceIdToModelIdMap, trackModel,
                 storageMode, timeStampMode, trackTypes, geoTransform,
                 frameMap)
{}

//-----------------------------------------------------------------------------
bool vpVidtkFileTrackIO::ReadTracks()
{
  if (!vpVidtkTrackIO::ReadTracks())
    {
    return false;
    }
  vpFileTrackIOImpl::ReadSupplementalFiles(
    this, static_cast<const vpVidtkFileReader&>(
            this->GetReader()).GetTracksFileName());
  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkFileTrackIO::ImportTracks(vtkIdType idsOffset,
                                      float offsetX, float offsetY)
{
  if (!vpVidtkTrackIO::ImportTracks(idsOffset, offsetX, offsetY))
    {
    return false;
    }
  vpFileTrackIOImpl::ImportSupplementalFiles(
    this,
    static_cast<const vpVidtkFileReader&>(this->GetReader()).GetTracksFileName(),
    offsetX, offsetY);
  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkFileTrackIO::ReadTrackTraits()
{
  return vpFileTrackIOImpl::ReadTrackTraits(
           this, static_cast<const vpVidtkFileReader&>(
                   this->GetReader()).GetTrackTraitsFileName());
}
