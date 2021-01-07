// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpVidtkFileIO.h"

#include "vpFseTrackIO.h"
#include "vpVidtkFileEventIO.h"
#include "vpVidtkFileTrackIO.h"

//-----------------------------------------------------------------------------
vpVidtkFileIO::vpVidtkFileIO() :
  ImageHeight(0)
{
}

//-----------------------------------------------------------------------------
vpVidtkFileIO::~vpVidtkFileIO()
{
}

//-----------------------------------------------------------------------------
void vpVidtkFileIO::SetTrackModel(vtkVpTrackModel* trackModel,
                                  vpTrackIO::TrackStorageMode storageMode,
                                  bool interpolateToGround,
                                  vpTrackIO::TrackTimeStampMode timeStampMode,
                                  vtkVgTrackTypeRegistry* trackTypes,
                                  vgAttributeSet* trackAttributes,
                                  vtkMatrix4x4* geoTransform,
                                  vpFrameMap* frameMap)
{
  this->TrackMap.clear();
  this->TrackIO.reset(
    new vpVidtkFileTrackIO(this->Reader, this->TrackMap,
                           this->SourceTrackIdToModelIdMap,
                           trackModel, storageMode, interpolateToGround,
                           timeStampMode, trackTypes, trackAttributes,
                           geoTransform, frameMap));

  this->FseTrackIO.reset(
    new vpFseTrackIO(trackModel, storageMode, interpolateToGround,
                     timeStampMode, trackTypes, geoTransform, frameMap));
  this->FseTrackIO->SetTracksFileName(this->FseTracksFileName);
  this->FseTrackIO->SetImageHeight(this->ImageHeight);
}

//-----------------------------------------------------------------------------
void vpVidtkFileIO::SetEventModel(vtkVgEventModel* eventModel,
                                  vtkVgEventTypeRegistry* eventTypes)
{
  this->EventMap.clear();
  this->EventIO.reset(
    new vpVidtkFileEventIO(this->Reader,
                           this->EventMap, this->SourceEventIdToModelIdMap,
                           this->TrackMap, this->SourceTrackIdToModelIdMap,
                           eventModel, eventTypes));
}

//-----------------------------------------------------------------------------
void vpVidtkFileIO::SetImageHeight(unsigned int imageHeight)
{
  this->ImageHeight = imageHeight;
  this->GetReader().SetImageHeight(imageHeight);
  if (this->FseTrackIO)
    {
    this->FseTrackIO->SetImageHeight(imageHeight);
    }
}

//-----------------------------------------------------------------------------
unsigned int vpVidtkFileIO::GetImageHeight() const
{
  return this->ImageHeight;
}

//-----------------------------------------------------------------------------
void vpVidtkFileIO::SetFseTracksFileName(const QString& fseTracksFileName)
{
  this->FseTracksFileName = fseTracksFileName;
  if (this->FseTrackIO)
    {
    this->FseTrackIO->SetTracksFileName(fseTracksFileName);
    }
}

//-----------------------------------------------------------------------------
vpVidtkReader& vpVidtkFileIO::GetReader()
{
  return this->Reader;
}

//-----------------------------------------------------------------------------
const vpVidtkReader& vpVidtkFileIO::GetReader() const
{
  return this->Reader;
}
