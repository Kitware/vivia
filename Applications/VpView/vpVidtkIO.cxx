// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
                              bool interpolateToGround,
                              vpTrackIO::TrackTimeStampMode timeStampMode,
                              vtkVgTrackTypeRegistry* trackTypes,
                              vgAttributeSet* trackAttributes,
                              vtkMatrix4x4* geoTransform,
                              vpFrameMap* frameMap)
{
  this->TrackMap.clear();
  this->TrackIO.reset(
    new vpVidtkTrackIO(this->GetReader(), this->TrackMap,
                       this->SourceTrackIdToModelIdMap,
                       trackModel, storageMode, interpolateToGround,
                       timeStampMode, trackTypes, trackAttributes,
                       geoTransform, frameMap));
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
void vpVidtkIO::UpdateTracks(const std::vector<vidtk::track_sptr>& tracks,
                             unsigned int updateStartFrame,
                             unsigned int updateEndFrame)
{
  assert(this->TrackIO);
  static_cast<vpVidtkTrackIO*>(this->TrackIO.get())->UpdateTracks(
    tracks, updateStartFrame, updateEndFrame);
}
