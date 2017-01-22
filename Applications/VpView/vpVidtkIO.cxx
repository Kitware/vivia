/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkIO.h"

//#include "vpVidtkActivityIO.h"
//#include "vpVidtkEventIO.h"
#include "vpVidtkReader.h"
#include "vpVidtkTrackIO.h"

//-----------------------------------------------------------------------------
vpVidtkIO::~vpVidtkIO()
{
  delete this->ActivityIO;
  delete this->EventIO;
  delete this->TrackIO;
}

//-----------------------------------------------------------------------------
void vpVidtkIO::SetTrackModel(vtkVgTrackModel* trackModel,
                              vpTrackIO::TrackStorageMode storageMode,
                              vpTrackIO::TrackTimeStampMode timeStampMode,
                              vtkVgTrackTypeRegistry* trackTypes,
                              vtkMatrix4x4* geoTransform,
                              vpFrameMap* frameMap)
{
  delete this->TrackIO;
  this->TrackMap.clear();
  this->TrackIO = new vpVidtkTrackIO(this->GetReader(), this->TrackMap,
                                     this->SourceTrackIdToModelIdMap,
                                     trackModel, storageMode, timeStampMode,
                                     trackTypes, geoTransform, frameMap);
}

//-----------------------------------------------------------------------------
void vpVidtkIO::SetEventModel(vtkVgEventModel* eventModel,
                              vtkVgEventTypeRegistry* eventTypes)
{
  delete this->EventIO;
/*  this->EventMap.clear();
  this->EventIO = new vpVidtkEventIO(this->GetReader(), this->EventMap,
                                     this->SourceEventIdToModelIdMap,
                                     this->TrackMap,
                                     this->SourceTrackIdToModelIdMap,
                                     eventModel, eventTypes);*/
}

//-----------------------------------------------------------------------------
void vpVidtkIO::SetActivityModel(vtkVgActivityManager* activityManager,
                                 vpActivityConfig* activityConfig)
{
  delete this->ActivityIO;
/*  this->ActivityIO = new vpVidtkActivityIO(this->GetReader(), activityManager,
                                           activityConfig);*/
}

//-----------------------------------------------------------------------------
void vpVidtkIO::SetImageHeight(unsigned int imageHeight)
{
  this->GetReader().SetImageHeight(imageHeight);
}

//-----------------------------------------------------------------------------
unsigned int vpVidtkIO::GetImageHeight() const
{
  return this->GetReader().GetImageHeight();
}

//-----------------------------------------------------------------------------
void vpVidtkIO::UpdateTracks(const vcl_vector<vidtk::track_sptr>& tracks,
                             unsigned int updateStartFrame,
                             unsigned int updateEndFrame)
{
  assert(this->TrackIO);
  static_cast<vpVidtkTrackIO*>(this->TrackIO)->UpdateTracks(
    tracks, updateStartFrame, updateEndFrame);
}
