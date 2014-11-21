/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgEvent.h"

// VTK includes.
#include <vtkBoundingBox.h>
#include <vtkIdListCollection.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

#include "vtkVgTrack.h"
#include "vtkVgTypeDefs.h"

#include <set>

vtkStandardNewMacro(vtkVgEvent);

vtkImplementMetaObject(vtkVgEventTrackInfo, vtkVgEventTrackInfoBase);

//-----------------------------------------------------------------------------
vtkVgEventTrackInfo::vtkVgEventTrackInfo(vtkVgTrack* track,
                                         const vtkVgTimeStamp& startFrame,
                                         const vtkVgTimeStamp& endFrame) :
  vtkVgEventTrackInfoBase(-1, startFrame, endFrame)
{
  this->Track = 0x0;
  this->SetTrack(track);
  this->TrackId = track->GetId();
}

//-----------------------------------------------------------------------------
vtkVgEventTrackInfo::vtkVgEventTrackInfo(const vtkVgEventTrackInfoBase* fromTrackInfo) :
  vtkVgEventTrackInfoBase(*fromTrackInfo)
{
  this->Track = 0x0;
}

//-----------------------------------------------------------------------------
vtkVgEventTrackInfo::vtkVgEventTrackInfo(const vtkVgEventTrackInfo* fromTrackInfo) :
  vtkVgEventTrackInfoBase(*fromTrackInfo)
{
  this->Track = 0x0;
  if (fromTrackInfo->Track)
    {
    this->SetTrack(fromTrackInfo->Track);
    this->TrackId = this->Track->GetId();
    }
}

//-----------------------------------------------------------------------------
vtkVgEventTrackInfoBase* vtkVgEventTrackInfo::Clone() const
{
  return new vtkVgEventTrackInfo(this);
}

//-----------------------------------------------------------------------------
vtkVgEventTrackInfo::~vtkVgEventTrackInfo()
{
  if (this->Track)
    {
    this->Track->UnRegister(NULL);
    }
}

//-----------------------------------------------------------------------------
const char* vtkVgEventTrackInfo::CheckValid() const
{
  if (!this->Track)
    {
    return "Invalid track.\n";
    }

  return vtkVgEventTrackInfoBase::CheckBaseValid();
}

//-----------------------------------------------------------------------------
void vtkVgEventTrackInfo::SetTrack(vtkVgTrack* track)
{
  if (this->Track != track)
    {
    vtkVgTrack* temp = this->Track;
    this->Track = track;
    if (this->Track != NULL)
      {
      this->Track->Register(NULL);
      }
    if (temp != NULL)
      {
      temp->UnRegister(NULL);
      }
    }
}

//-----------------------------------------------------------------------------
vtkVgEvent::vtkVgEvent()
{
  this->FullEventIdCollection = vtkIdListCollection::New();

  this->IconIds = vtkIdList::New();

  this->UseCustomColor = false;
  this->CustomColor[0] = this->CustomColor[1] = this->CustomColor[2] = 0.5;
}

//-----------------------------------------------------------------------------
vtkVgEvent::~vtkVgEvent()
{
  this->FullEventIdCollection->Delete();
  this->IconIds->Delete();
}

//-----------------------------------------------------------------------------
double* vtkVgEvent::GetFullBounds()
{
  if (this->GetNumberOfTracks() < 1)
    {
    vtkErrorMacro("Track must be set!");
    return 0;
    }
  vtkBoundingBox bbox;
  std::vector<vtkVgEventTrackInfoBase*>::iterator trackIter = this->Tracks.begin();
  for (; trackIter != this->Tracks.end(); trackIter++)
    {
    vtkVgEventTrackInfo* trackInfoPtr =
      vtkVgEventTrackInfo::SafeDownCast(*trackIter);
    if (!trackInfoPtr)
      {
      vtkErrorMacro("Invalid TrackInfo type!");
      return 0;
      }
    if (!trackInfoPtr->Track)
      {
      return 0;  // no tracks, so for now (until something breaks), no bounds
      }
    vtkIdList* trackIds = trackInfoPtr->Track->GetPointIds();
    vtkIdType startFramePtId = trackInfoPtr->Track->GetClosestFramePtId(
                                 trackInfoPtr->StartFrame);
    if (startFramePtId < 0)
      {
      continue;  // for some reason track isn't started
      }
    vtkIdType endFramePtId = trackInfoPtr->Track->GetClosestFramePtId(
                               trackInfoPtr->EndFrame);
    vtkIdType startFrameIndex = trackIds->IsId(startFramePtId);
    vtkIdType endFrameIndex = trackIds->IsId(endFramePtId);
    for (vtkIdType i = startFrameIndex; i <= endFrameIndex; i++)
      {
      double* pt = trackInfoPtr->Track->GetPoints()->GetPoint(trackIds->GetId(i));
      bbox.AddPoint(pt);
      }
    }
  bbox.GetBounds(this->FullBounds);
  return this->FullBounds;
}

//-----------------------------------------------------------------------------
void vtkVgEvent::GetFullBounds(double bounds[4])
{
  this->GetFullBounds();
  bounds[0] = this->FullBounds[0];
  bounds[1] = this->FullBounds[1];
  bounds[2] = this->FullBounds[2];
  bounds[3] = this->FullBounds[3];
}

//-----------------------------------------------------------------------------
bool vtkVgEvent::GetDisplayPosition(const vtkVgTimeStamp& timeStamp,
                                    double position[2])
{
  position[0] = position[1] = 0.0;
  double trackPt[2];
  int numTracks = 0;
  std::vector<vtkVgEventTrackInfoBase*>::iterator trackIter = this->Tracks.begin();
  for (; trackIter != this->Tracks.end(); trackIter++)
    {
    vtkVgEventTrackInfo* trackInfoPtr =
      vtkVgEventTrackInfo::SafeDownCast(*trackIter);
    if (!trackInfoPtr)
      {
      vtkErrorMacro("Invalid TrackInfo type!");
      return false;
      }
    // can't proceed without a track pointer, or a track that hasn't been started
    if (!trackInfoPtr->Track || !trackInfoPtr->Track->IsStarted())
      {
      return false;
      }

    // what time stamp should we be using to pull the closest frame pt
    vtkVgTimeStamp testTimeStamp = timeStamp;
    if (timeStamp < trackInfoPtr->StartFrame)
      {
      testTimeStamp = trackInfoPtr->StartFrame;
      }
    else if (timeStamp > trackInfoPtr->EndFrame)
      {
      testTimeStamp = trackInfoPtr->EndFrame;
      }

    // if we get a valid point, add it to our sum
    if (trackInfoPtr->Track->GetClosestFramePt(testTimeStamp, trackPt))
      {
      position[0] += trackPt[0];
      position[1] += trackPt[1];
      numTracks++;
      }
    }

  if (numTracks  == 0)
    {
    return false;
    }

  position[0] /= numTracks;
  position[1] /= numTracks;
  return true;
}

//-----------------------------------------------------------------------------
vtkIdListCollection* vtkVgEvent::GetFullEventIdCollection()
{
  if (this->FullEventIdCollection->GetNumberOfItems() == 0)
    {
    std::vector<vtkVgEventTrackInfoBase*>::iterator trackIter = this->Tracks.begin();
    for (; trackIter != this->Tracks.end(); trackIter++)
      {
      vtkVgEventTrackInfo* trackInfoPtr =
        vtkVgEventTrackInfo::SafeDownCast(*trackIter);
      if (!trackInfoPtr)
        {
        vtkErrorMacro("Invalid TrackInfo type!");
        return 0;
        }
      if (!trackInfoPtr->Track)
        {
        continue; // can't proceed without a track pointer
        }

      vtkIdList* trackIds = trackInfoPtr->Track->GetPointIds();

      vtkIdType startFramePtId = trackInfoPtr->Track->GetClosestFramePtId(
                                   trackInfoPtr->StartFrame);
      if (startFramePtId < 0)
        {
        continue;  // for some reason track isn't started
        }
      vtkIdType endFramePtId = trackInfoPtr->Track->GetClosestFramePtId(
                                 trackInfoPtr->EndFrame);
      vtkIdType startFrameIndex = trackIds->IsId(startFramePtId);
      vtkIdType endFrameIndex = trackIds->IsId(endFramePtId);

      vtkIdType numberOfFrames = endFrameIndex - startFrameIndex + 1;

      vtkIdList* idList = vtkIdList::New();
      memcpy(idList->WritePointer(0, numberOfFrames),
             trackIds->GetPointer(startFrameIndex),
             sizeof(vtkIdType) * numberOfFrames);
      this->FullEventIdCollection->AddItem(idList);
      idList->FastDelete();
      }
    }
  return this->FullEventIdCollection;
}

//-----------------------------------------------------------------------------
vtkVgTrackDisplayData vtkVgEvent::GetTrackDisplayData(unsigned int trackIndex,
                                                      bool useTrackGroups,
                                                      vtkVgTimeStamp start,
                                                      vtkVgTimeStamp end)
{
  if (this->Tracks.size() == 0)
    {
    return vtkVgTrackDisplayData();
    }

  vtkVgTrack* theTrack = 0;
  vtkVgTimeStamp startTime, endTime;
  if (useTrackGroups)
    {
    if (!this->GetTrackGroupInfo(trackIndex, theTrack, startTime, endTime))
      {
      return vtkVgTrackDisplayData();
      }
    }
  else
    {
    vtkVgEventTrackInfo* eti = vtkVgEventTrackInfo::SafeDownCast(
                                 this->Tracks[trackIndex]);

    theTrack = eti->Track;
    startTime = eti->StartFrame;
    endTime = eti->EndFrame;
    }

  if (!theTrack || !theTrack->IsStarted())
    {
    return vtkVgTrackDisplayData();
    }

  // check if any of the event track is active in the requested time interval
  if (end < startTime || start > endTime)
    {
    return vtkVgTrackDisplayData();
    }

  // clamp start time to valid range for this track
  if (start < startTime)
    {
    start = startTime;
    }
  else if (start > endTime)
    {
    start = endTime;
    }

  // clamp end time to valid range for this track
  if (end > endTime)
    {
    end = endTime;
    }
  else if (end < startTime)
    {
    end = startTime;
    }

  vtkIdType startId = theTrack->GetClosestFramePtId(start);
  vtkIdType endId = theTrack->GetClosestFramePtId(end);

  vtkIdList* trackIds = theTrack->GetPointIds();
  vtkIdType startIndex = trackIds->IsId(startId); // linear lookup
  vtkIdType endIndex = trackIds->IsId(endId);     // linear lookup

  vtkVgTrackDisplayData tdd;
  tdd.IdsStart = trackIds->GetPointer(startIndex);
  tdd.NumIds = endIndex - startIndex + 1;
  return tdd;
}

//-----------------------------------------------------------------------------
bool vtkVgEvent::GetTrackGroupInfo(unsigned int trackGroupIndex,
                                   vtkVgTrack*& theTrack,
                                   vtkVgTimeStamp& startTime,
                                   vtkVgTimeStamp& endTime)
{
  bool trackOnlyUsedOnce = true;
  theTrack = 0;
  std::vector<vtkVgEventTrackInfoBase*>::const_iterator trackIter;
  std::set<vtkIdType> uniqueTracks;
  for (trackIter = this->Tracks.begin();
       trackIter != this->Tracks.end(); trackIter++)
    {
    uniqueTracks.insert((*trackIter)->TrackId);
    if (!theTrack && uniqueTracks.size() > trackGroupIndex)
      {
      // this is the track group we want
      theTrack = vtkVgEventTrackInfo::SafeDownCast(*trackIter)->GetTrack();
      if (!theTrack || !theTrack->IsStarted())
        {
        return false;
        }

      startTime = (*trackIter)->StartFrame;
      endTime = (*trackIter)->EndFrame;
      }
    else if (theTrack && (*trackIter)->TrackId == theTrack->GetId())
      {
      // we only make it here if there is more than one track in this group;
      // if only one track, we can use the information we already set
      trackOnlyUsedOnce = false;
      break;
      }
    }

  if (!theTrack)
    {
    return false;
    }

  if (trackOnlyUsedOnce)
    {
    return true;
    }

  // collect start / end time info from all track that match our
  for (trackIter = this->Tracks.begin(); trackIter != this->Tracks.end();
       trackIter++)
    {
    if ((*trackIter)->TrackId != theTrack->GetId())
      {
      continue;
      }

    vtkVgEventTrackInfo* eti = vtkVgEventTrackInfo::SafeDownCast(*trackIter);

    if (eti->StartFrame < startTime)
      {
      startTime = eti->StartFrame;
      }
    if (eti->EndFrame > endTime)
      {
      endTime = eti->EndFrame;
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
vtkPoints* vtkVgEvent::GetPoints()
{
  if (this->Tracks.size() == 0)
    {
    return 0;
    }

  vtkVgEventTrackInfo* infoPtr =
    vtkVgEventTrackInfo::SafeDownCast(this->Tracks.front());
  if (!infoPtr)
    {
    vtkErrorMacro("Invalid TrackInfo type!");
    return 0;
    }

  if (!infoPtr->Track)  // make sure we have a track ptr
    {
    return 0;
    }


  return infoPtr->Track->GetPoints();
}

//-----------------------------------------------------------------------------
void vtkVgEvent::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgEvent::AddTrack(vtkVgTrack* track, const vtkVgTimeStamp& startFrame,
                          const vtkVgTimeStamp& endFrame)
{
  vtkVgEventTrackInfoBase* trackInfo = new vtkVgEventTrackInfo(track,
      startFrame, endFrame);

  this->Superclass::AddTrack(trackInfo);
}

//-----------------------------------------------------------------------------
void vtkVgEvent::GetTrack(unsigned int index, vtkVgTrack*& track,
                          vtkVgTimeStamp& startFrame, vtkVgTimeStamp& endFrame)
{
  if (index >= this->Tracks.size())
    {
    vtkErrorMacro("Index out of range.\n");
    return;
    }

  vtkVgEventTrackInfo* trackInfo =
    vtkVgEventTrackInfo::SafeDownCast(this->Tracks[index]);
  if (!trackInfo)
    {
    vtkErrorMacro("Invalid TrackInfo type!");
    return;
    }
  track           = trackInfo->Track;
  startFrame      = trackInfo->StartFrame;
  endFrame        = trackInfo->EndFrame;
}

//-----------------------------------------------------------------------------
vtkVgTrack* vtkVgEvent::GetTrack(unsigned int index)
{
  if (index >= this->Tracks.size())
    {
    vtkErrorMacro("Index out of range.\n");
    return 0;
    }

  vtkVgEventTrackInfo* trackInfoPtr =
    vtkVgEventTrackInfo::SafeDownCast(this->Tracks[index]);
  if (!trackInfoPtr)
    {
    vtkErrorMacro("Invalid TrackInfo type!");
    return 0;
    }
  return trackInfoPtr->Track;
}

//-----------------------------------------------------------------------------
vtkVgTrack* vtkVgEvent::GetTrackGroupTrack(unsigned int trackGroupIndex)
{
  if (trackGroupIndex > this->GetNumberOfTrackGroups())
    {
    vtkErrorMacro("Index out of range.\n");
    return 0;
    }

  vtkVgEventTrackInfo* trackInfoPtr = 0;
  std::vector<vtkVgEventTrackInfoBase*>::const_iterator trackIter;
  std::set<vtkIdType> uniqueTracks;
  for (trackIter = this->Tracks.begin(); trackIter != this->Tracks.end();
       trackIter++)
    {
    uniqueTracks.insert((*trackIter)->TrackId);
    if (uniqueTracks.size() > trackGroupIndex)
      {
      // this is the track group we want
      trackInfoPtr = vtkVgEventTrackInfo::SafeDownCast(*trackIter);
      break;
      }
    }
  if (!trackInfoPtr)
    {
    vtkErrorMacro("Invalid TrackInfo type!");
    return 0;
    }
  return trackInfoPtr->Track;
}

//-----------------------------------------------------------------------------
bool vtkVgEvent::HasTrack(int trackId)
{
  std::vector<vtkVgEventTrackInfoBase*>::const_iterator itr;
  itr = this->Tracks.begin();

  while (itr != this->Tracks.end())
    {
    if (vtkVgEventTrackInfo::SafeDownCast(*itr)->Track->GetId() == trackId)
      {
      return true;
      }
    ++itr;
    }

  return false;
}

//-----------------------------------------------------------------------------
void vtkVgEvent::CopyTracks(std::vector<vtkVgEventTrackInfoBase*>& tracks)
{
  vtkVgEventTrackInfo* infoPtr;
  vtkVgEventTrackInfoBase* trackInfo;
  std::vector<vtkVgEventTrackInfoBase*>::const_iterator trackIter;
  for (trackIter = tracks.begin();
       trackIter != tracks.end(); trackIter++)
    {
    infoPtr = vtkVgEventTrackInfo::SafeDownCast(*trackIter);
    if (infoPtr)
      {
      trackInfo = new vtkVgEventTrackInfo(infoPtr);
      }
    else
      {
      trackInfo = new vtkVgEventTrackInfo(*trackIter);
      }
    this->Tracks.push_back(trackInfo);
    }
}

//-----------------------------------------------------------------------------
bool vtkVgEvent::SetTrackPtr(unsigned int index, vtkVgTrack* track)
{
  if (index >= this->Tracks.size())
    {
    vtkErrorMacro("Index out of range.\n");
    return false;
    }
  else if (!track)
    {
    vtkErrorMacro("Null track ptr, can't set track ptr!");
    return false;
    }
  else if (track->GetId() != this->Tracks[index]->TrackId)
    {
    vtkErrorMacro("TrackId doesn't match Id of TrackPtr: " <<
                  this->Tracks[index]->TrackId << " " << track->GetId());
    return false;
    }

  vtkVgEventTrackInfo* infoPtr;
  if (!(infoPtr = vtkVgEventTrackInfo::SafeDownCast(this->Tracks[index])))
    {
    vtkErrorMacro("Invalid TrackInfo type!");
    return 0;
    }
  infoPtr->SetTrack(track);
  return true;
}
