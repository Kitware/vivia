/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
  delete this->FseTrackIO;
}

//-----------------------------------------------------------------------------
void vpVidtkFileIO::SetTrackModel(vtkVgTrackModel* trackModel,
                                  vpTrackIO::TrackStorageMode storageMode,
                                  vpTrackIO::TrackTimeStampMode timeStampMode,
                                  vtkVgTrackTypeRegistry* trackTypes,
                                  vtkMatrix4x4* geoTransform,
                                  vpFrameMap* frameMap)
{
  delete this->TrackIO;
  this->TrackMap.clear();
  this->TrackIO = new vpVidtkFileTrackIO(
                    this->Reader, this->TrackMap,
                    this->SourceTrackIdToModelIdMap, trackModel,
                    storageMode, timeStampMode, trackTypes,
                    geoTransform, frameMap);

  delete this->FseTrackIO;
  this->FseTrackIO = new vpFseTrackIO(trackModel, storageMode, timeStampMode,
                                      trackTypes, geoTransform, frameMap);
  this->FseTrackIO->SetTracksFileName(this->FseTracksFileName.c_str());
  this->FseTrackIO->SetImageHeight(this->ImageHeight);
}

//-----------------------------------------------------------------------------
void vpVidtkFileIO::SetEventModel(vtkVgEventModel* eventModel,
                                  vtkVgEventTypeRegistry* eventTypes)
{
  delete this->EventIO;
  this->EventMap.clear();
  this->EventIO = new vpVidtkFileEventIO(
                    this->Reader,
                    this->EventMap, this->SourceEventIdToModelIdMap,
                    this->TrackMap, this->SourceTrackIdToModelIdMap,
                    eventModel, eventTypes);
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
void vpVidtkFileIO::SetFseTracksFileName(const char* fseTracksFileName)
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
