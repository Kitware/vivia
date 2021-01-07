// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVpTrackModel.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVpTrackModel);

//-----------------------------------------------------------------------------
vtkVpTrackModel::vtkVpTrackModel() : vtkVgTrackModel()
{
}

//-----------------------------------------------------------------------------
vtkVpTrackModel::~vtkVpTrackModel()
{
}

//-----------------------------------------------------------------------------
void vtkVpTrackModel::RemoveTrack(vtkIdType trackId)
{
  this->vtkVgTrackModel::RemoveTrack(trackId);
  this->TrackKeyframes.erase(trackId);
}

//-----------------------------------------------------------------------------
void vtkVpTrackModel::AddKeyframe(
  vtkIdType trackId, const vtkVgTimeStamp& timeStamp)
{
  auto& trackKeyframes = this->TrackKeyframes[trackId];
  trackKeyframes.insert(timeStamp);
}

//-----------------------------------------------------------------------------
void vtkVpTrackModel::RemoveKeyframe(
  vtkIdType trackId, const vtkVgTimeStamp& timeStamp)
{
    auto& trackKeyframes = this->TrackKeyframes[trackId];
    trackKeyframes.erase(timeStamp);
}

//-----------------------------------------------------------------------------
bool vtkVpTrackModel::GetIsKeyframe(
  vtkIdType trackId, const vtkVgTimeStamp& timeStamp) const
{
  const auto& trackKeyframesIter = this->TrackKeyframes.find(trackId);
  if (trackKeyframesIter != this->TrackKeyframes.end())
    {
    const auto& trackKeyframes = trackKeyframesIter->second;
    const auto& keyframeIter = trackKeyframes.find(timeStamp);
    return keyframeIter != trackKeyframes.end();
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkVpTrackModel::SetTrackId(vtkVgTrack* track, vtkIdType newId)
{
  const auto oldId = track->GetId();

  this->vtkVgTrackModel::RemoveTrack(oldId);
  track->SetId(newId);
  this->vtkVgTrackModel::AddTrack(track);

  const auto& tkfIter = this->TrackKeyframes.find(oldId);
  if (tkfIter != this->TrackKeyframes.end())
    {
#if __cplusplus >= 201703L
    auto tkfNode = this->TrackKeyframes.extract(tkfIter);
    tkfNode.key() = newId;
    this->TrackKeyframes.insert(strd::move(tkfNode));
#else
    const auto& trackKeyframes = tkfIter->second;
    this->TrackKeyframes.emplace(newId, trackKeyframes);
    this->TrackKeyframes.erase(oldId);
#endif
    }
}
