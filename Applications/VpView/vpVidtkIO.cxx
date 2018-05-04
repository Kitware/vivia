/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkIO.h"

#include "vpVidtkActivityIO.h"
#include "vpVidtkEventIO.h"
#include "vpVidtkReader.h"
#include "vpVidtkTrackIO.h"

//-----------------------------------------------------------------------------
vpVidtkIO::~vpVidtkIO()
{
}

//-----------------------------------------------------------------------------
void vpVidtkIO::SetTrackModel(vtkVpTrackModel* trackModel,
                              vpTrackIO::TrackStorageMode storageMode,
                              vpTrackIO::TrackTimeStampMode timeStampMode,
                              vtkVgTrackTypeRegistry* trackTypes,
                              vtkMatrix4x4* geoTransform,
                              vpFrameMap* frameMap)
{
  this->TrackMap.clear();
  this->TrackIO.reset(
    new vpVidtkTrackIO(this->GetReader(), this->TrackMap,
                       this->SourceTrackIdToModelIdMap,
                       trackModel, storageMode, timeStampMode,
                       trackTypes, geoTransform, frameMap));
}

//-----------------------------------------------------------------------------
void vpVidtkIO::SetEventModel(vtkVgEventModel* eventModel,
                              vtkVgEventTypeRegistry* eventTypes)
{
  this->EventMap.clear();
  this->EventIO.reset(
    new vpVidtkEventIO(this->GetReader(),
                       this->EventMap, this->SourceEventIdToModelIdMap,
                       this->TrackMap, this->SourceTrackIdToModelIdMap,
                       eventModel, eventTypes));
}

//-----------------------------------------------------------------------------
void vpVidtkIO::SetActivityModel(vtkVgActivityManager* activityManager,
                                 vpActivityConfig* activityConfig)
{
  this->ActivityIO.reset(
    new vpVidtkActivityIO(this->GetReader(), activityManager, activityConfig));
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
  static_cast<vpVidtkTrackIO*>(this->TrackIO.data())->UpdateTracks(
    tracks, updateStartFrame, updateEndFrame);
}
