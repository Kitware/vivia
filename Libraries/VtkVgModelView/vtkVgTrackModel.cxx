/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTrackModel.h"

#include "vtkVgContourOperatorManager.h"
#include "vtkVgScalars.h"
#include "vtkVgTemporalFilters.h"
#include "vtkVgTrack.h"

#include <vtkObjectFactory.h>
#include <vtkCommand.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkTimeStamp.h>

#include <assert.h>
#include <limits>
#include <set>

vtkStandardNewMacro(vtkVgTrackModel);

vtkSetObjectImplementationMacro(vtkVgTrackModel, Points, vtkPoints);

typedef std::map<vtkIdType, vtkVgTrackInfo>           TrackMap;
typedef std::map<vtkIdType, vtkVgTrackInfo>::iterator TrackMapIterator;

static const vtkVgTrackInfo InvalidTrackInfo = vtkVgTrackInfo();

//-----------------------------------------------------------------------------
struct vtkVgTrackModel::vtkInternal
{
  TrackMap         TrackIdMap;
  TrackMapIterator TrackIterator;

  vtkTimeStamp UpdateTime;
  vtkTimeStamp SpatialFilteringUpdateTime;
  vtkTimeStamp TemporalFilteringUpdateTime;
  vtkTimeStamp ComputedAllScalarsNameTime;

  std::vector<std::string> AllScalarsName;

  // Scalars field name to scalars range map
  std::map<std::string, ScalarsRange> ScalarsNameToRangeMap;

  // Scalars field name to last computed range timestamp map
  std::map<std::string, vtkTimeStamp> ScalarsNameToTimeMap;
};

//-----------------------------------------------------------------------------
vtkVgTrackModel::vtkVgTrackModel()
{
  this->Internal = new vtkInternal();
  this->DisplayAllTracks = true;
  this->Points = vtkPoints::New();
  this->CurrentTimeStamp.SetToMinTime();

  this->ShowTracksBeforeStart = false;
  this->ShowTracksAfterExpiration = false;

  this->TrackExpirationOffset.SetFrameNumber(0u);
  this->TrackExpirationOffset.SetTime(0.0);

  this->SceneElementIdsOffset = 0;
}

//-----------------------------------------------------------------------------
vtkVgTrackModel::~vtkVgTrackModel()
{
  this->Points->Delete();
  this->ClearTracks();
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::ClearTracks()
{
  for (TrackMapIterator itr = this->Internal->TrackIdMap.begin(),
       end = this->Internal->TrackIdMap.end(); itr != end; ++itr)
    {
    itr->second.GetTrack()->UnRegister(this);
    }
  this->Internal->TrackIdMap.clear();
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::Initialize()
{
  this->ClearTracks();
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::AddTrack(vtkVgTrack* track)
{
  // Check if the incoming track is using same points
  // as the track model, if not toss it.
  if (!track->GetPoints())
    {
    track->SetPoints(this->GetPoints());
    }
  else if (track->GetPoints() != this->GetPoints())
    {
    vtkWarningMacro("Mismatch between track and model points.\n")
    vtkWarningMacro("Resetting track points\n");

    track->SetPoints(this->GetPoints());
    }

  vtkVgTrackInfo trackInfo(track);
  if (!this->DisplayAllTracks)
    {
    trackInfo.SetDisplayTrackOff();
    }
  trackInfo.GetTrack()->Register(this);
  this->Internal->TrackIdMap[track->GetId()] = trackInfo;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::RemoveTrack(vtkIdType trackId)
{
  TrackMapIterator trackIter = this->Internal->TrackIdMap.find(trackId);
  if (trackIter != this->Internal->TrackIdMap.end())
    {
    trackIter->second.GetTrack()->UnRegister(this);
    this->Internal->TrackIdMap.erase(trackIter);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vtkVgTrack* vtkVgTrackModel::GetTrack(vtkIdType trackId)
{
  TrackMapIterator trackIter = this->Internal->TrackIdMap.find(trackId);
  if (trackIter != this->Internal->TrackIdMap.end())
    {
    return trackIter->second.GetTrack();
    }
  return 0;
}

//-----------------------------------------------------------------------------
vtkVgTrackInfo vtkVgTrackModel::GetTrackInfo(vtkIdType trackId)
{
  TrackMapIterator trackIter = this->Internal->TrackIdMap.find(trackId);
  if (trackIter != this->Internal->TrackIdMap.end())
    {
    return trackIter->second;
    }
  return InvalidTrackInfo;
}

//-----------------------------------------------------------------------------
vtkVgTrackDisplayData vtkVgTrackModel::GetTrackDisplayData(vtkVgTrack* track,
  bool forceDisplay)
{
  if (!track->IsStarted())    // verify track is even started
    {
    return vtkVgTrackDisplayData();
    }

  vtkVgTimeStamp trackStart = this->GetTrackStartDisplayFrame(track);
  vtkVgTimeStamp trackExpiration = this->GetTrackEndDisplayFrame(track);

  // Never display the track if its start occurs before the reference time.
  if (this->ReferenceTimeStamp.IsValid() &&
      trackStart < this->ReferenceTimeStamp)
    {
    return vtkVgTrackDisplayData();
    }

  if (!forceDisplay &&
      !this->ShowTracksBeforeStart &&
      this->CurrentTimeStamp < trackStart)
    {
    return vtkVgTrackDisplayData();
    }

  if (!forceDisplay &&
      !this->ShowTracksAfterExpiration &&
      this->CurrentTimeStamp > trackExpiration)
    {
    return vtkVgTrackDisplayData();
    }

  // Display the whole track if showing before the start of the track.
  if (this->CurrentTimeStamp < trackStart)
    {
    return track->GetDisplayData(vtkVgTimeStamp(false),
                                 vtkVgTimeStamp(true));
    }

  // May need to move the start time forward if max duration is set.
  if (!forceDisplay && this->MaxDisplayDuration.IsValid())
    {
    // Don't show completed tracks if a max duration is set.
    if (this->CurrentTimeStamp > track->GetEndFrame())
      {
      return vtkVgTrackDisplayData();
      }

    vtkVgTimeStamp duration = this->CurrentTimeStamp;
    duration.ShiftBackward(trackStart);
    if (duration > this->MaxDisplayDuration)
      {
      vtkVgTimeStamp start = this->CurrentTimeStamp;
      start.ShiftBackward(this->MaxDisplayDuration);
      return track->GetDisplayData(start, this->CurrentTimeStamp);
      }
    }

  // By default, display the track from its start up until the current frame.
  return track->GetDisplayData(vtkVgTimeStamp(false), this->CurrentTimeStamp);
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vtkVgTrackModel::GetTrackStartDisplayFrame(vtkVgTrack* track)
{
  return track->GetStartFrame();
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vtkVgTrackModel::GetTrackEndDisplayFrame(vtkVgTrack* track)
{
  vtkVgTimeStamp endFrame = track->GetEndFrame();
  if (!endFrame.IsValid())
    {
    // Track not yet closed.
    endFrame.SetToMaxTime();
    return endFrame;
    }

  endFrame.ShiftForward(this->TrackExpirationOffset);
  return endFrame;
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::InitTrackTraversal()
{
  this->Internal->TrackIterator = this->Internal->TrackIdMap.begin();
}

//-----------------------------------------------------------------------------
vtkVgTrackInfo vtkVgTrackModel::GetNextTrack()
{
  if (this->Internal->TrackIterator != this->Internal->TrackIdMap.end())
    {
    return (this->Internal->TrackIterator++)->second;
    }

  return InvalidTrackInfo;
}

//-----------------------------------------------------------------------------
vtkVgTrackInfo vtkVgTrackModel::GetNextDisplayedTrack()
{
  while (this->Internal->TrackIterator != this->Internal->TrackIdMap.end())
    {
    if (this->Internal->TrackIterator->second.GetDisplayTrack() &&
        this->Internal->TrackIterator->second.GetPassesFilters())
      {
      return (this->Internal->TrackIterator++)->second;
      }
    ++this->Internal->TrackIterator;
    }

  return InvalidTrackInfo;
}

//-----------------------------------------------------------------------------
int vtkVgTrackModel::GetNumberOfTracks()
{
  return static_cast<int>(this->Internal->TrackIdMap.size());
}

//-----------------------------------------------------------------------------
int vtkVgTrackModel::Update(const vtkVgTimeStamp& timestamp,
                            const vtkVgTimeStamp* referenceFrameTimeStamp/*=0*/)
{
  bool updateSpatial = this->ContourOperatorManager &&
                       (this->GetMTime() >
                        this->Internal->SpatialFilteringUpdateTime ||
                        this->ContourOperatorManager->GetMTime() >
                        this->Internal->SpatialFilteringUpdateTime);

  bool updateTemporal = this->TemporalFilters &&
                        (this->GetMTime() >
                         this->Internal->TemporalFilteringUpdateTime ||
                         this->TemporalFilters->GetMTime() >
                         this->Internal->TemporalFilteringUpdateTime);

  if (this->CurrentTimeStamp == timestamp &&
      this->Internal->UpdateTime > this->GetMTime() &&
      !(updateSpatial || updateTemporal))
    {
    return VTK_OK;
    }

  this->CurrentTimeStamp = timestamp;
  this->ReferenceTimeStamp = referenceFrameTimeStamp ? *referenceFrameTimeStamp
                             : vtkVgTimeStamp();

  this->Internal->UpdateTime.Modified();
  if (updateSpatial || updateTemporal)
    {
    this->Internal->SpatialFilteringUpdateTime.Modified();
    this->Internal->TemporalFilteringUpdateTime.Modified();
    }

  for (TrackMapIterator trackIter = this->Internal->TrackIdMap.begin();
       trackIter != this->Internal->TrackIdMap.end(); ++trackIter)
    {
    vtkVgTrackInfo& info = trackIter->second;

    // Update filtering. Since we only track one "filtered" bit, both must
    // update at the same time. If updating both ends up being a performance
    // problem, use separate filtered bits to optimize.
    if (updateTemporal || updateSpatial)
      {
      info.SetPassesFiltersOn();
      if (this->TemporalFilters)
        {
        this->UpdateTemporalFiltering(info);
        }
      if (this->ContourOperatorManager && info.GetPassesFilters())
        {
        this->UpdateSpatialFiltering(info);
        }
      }

    // Update head visibility flag.
    info.SetHeadVisibleOn();
    if (this->ContourOperatorManager &&
        (this->ContourOperatorManager->GetNumberOfEnabledFilters() > 0 ||
         this->ContourOperatorManager->GetNumberOfEnabledSelectors() > 0))
      {
      vtkVgTrackDisplayData tdd = this->GetTrackDisplayData(info.GetTrack());
      if (tdd.NumIds > 0)
        {
        bool headVisible =
          this->ContourOperatorManager->EvaluatePoint(
            info.GetTrack()->GetPoints()->GetPoint(
              tdd.IdsStart[tdd.NumIds - 1]));

        headVisible ? info.SetHeadVisibleOn() : info.SetHeadVisibleOff();
        }
      }
    }

  this->InvokeEvent(vtkCommand::UpdateDataEvent);
  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::SetTrackDisplayState(vtkIdType trackId, bool displayTrack)
{
  TrackMapIterator trackIter = this->Internal->TrackIdMap.find(trackId);
  if (trackIter != this->Internal->TrackIdMap.end() &&
      trackIter->second.GetDisplayTrack() != displayTrack)
    {
    displayTrack ? trackIter->second.SetDisplayTrackOn()
    : trackIter->second.SetDisplayTrackOff();
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::SetDisplayAllTracks(bool state)
{
  if (state != this->DisplayAllTracks)
    {
    this->DisplayAllTracks = state;
    if (state)
      {
      this->SetAllTracksDisplayState(state);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::SetAllTracksDisplayState(bool state)
{
  bool modified = false;
  for (TrackMapIterator trackIter = this->Internal->TrackIdMap.begin();
       trackIter != this->Internal->TrackIdMap.end(); ++trackIter)
    {
    if (trackIter->second.GetDisplayTrack() != state)
      {
      state ? trackIter->second.SetDisplayTrackOn()
      : trackIter->second.SetDisplayTrackOff();
      modified = true;
      }
    }
  if (modified)
    {
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::SetTrackExpirationOffset(const vtkVgTimeStamp& offset)
{
  this->TrackExpirationOffset = offset;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::SetMaximumDisplayDuration(const vtkVgTimeStamp &
                                                maxDisplayDuration)
{
  this->MaxDisplayDuration = maxDisplayDuration;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::SetTrackColor(vtkIdType trackId, double color[3])
{
  TrackMapIterator trackIter = this->Internal->TrackIdMap.find(trackId);
  if (trackIter != this->Internal->TrackIdMap.end())
    {
    trackIter->second.GetTrack()->SetColor(color);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
double* vtkVgTrackModel::GetTrackColor(vtkIdType trackId)
{
  TrackMapIterator trackIter = this->Internal->TrackIdMap.find(trackId);
  if (trackIter != this->Internal->TrackIdMap.end())
    {
    return trackIter->second.GetTrack()->GetColor();
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
unsigned long vtkVgTrackModel::GetUpdateTime()
{
  return this->Internal->UpdateTime.GetMTime();
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::SetActiveScalars(const std::string& name)
{
  if (name.empty())
    {
    vtkWarningMacro("<< Name is empty \n");
    return;
    }

  if (this->ActiveScalarsName == name)
    {
    return;
    }

  bool modified = false;
  for (TrackMapIterator trackIter = this->Internal->TrackIdMap.begin();
       trackIter != this->Internal->TrackIdMap.end(); ++trackIter)
    {
      vtkVgTrack* track = trackIter->second.GetTrack();

      if (track)
        {
        track->SetActiveScalars(name);
        modified = true;
        }
    }

  if (modified)
    {
    this->ActiveScalarsName = name;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
const std::string& vtkVgTrackModel::GetActiveScalars()
{
  return this->ActiveScalarsName;
}

//-----------------------------------------------------------------------------
const std::string& vtkVgTrackModel::GetActiveScalars() const
{
  return this->ActiveScalarsName;
}

//-----------------------------------------------------------------------------
std::vector<std::string> vtkVgTrackModel::GetAllScalarsName()
{
  std::set<std::string> existingNames;

  if (this->Internal->ComputedAllScalarsNameTime.GetMTime() > this->GetMTime())
    {
    return this->Internal->AllScalarsName;
    }

  for (TrackMapIterator trackIter = this->Internal->TrackIdMap.begin();
       trackIter != this->Internal->TrackIdMap.end(); ++trackIter)
    {
    vtkVgTrack* track = trackIter->second.GetTrack();

    if (track)
      {
      std::vector<std::string> names = track->GetScalarsName();
      for (size_t i = 0; i < names.size(); ++i)
        {
        if (existingNames.find(names[i]) == existingNames.end())
          {
          existingNames.insert(names[i]);
          this->Internal->AllScalarsName.push_back(names[i]);
          }
        }
      }
    }

  this->Internal->ComputedAllScalarsNameTime.Modified();

  return this->Internal->AllScalarsName;
}

//-----------------------------------------------------------------------------
vgRange<double> vtkVgTrackModel::GetScalarsRange(const std::string& name)
{
  if (this->Internal->ScalarsNameToTimeMap.find(name) !=
      this->Internal->ScalarsNameToTimeMap.end() &&
      this->Internal->ScalarsNameToTimeMap[name].GetMTime() > this->GetMTime())
    {
    return this->Internal->ScalarsNameToRangeMap[name];
    }

  ScalarsRange overallRange;
  overallRange.lower =  std::numeric_limits<double>::max();
  overallRange.upper = -std::numeric_limits<double>::max();

  for (TrackMapIterator trackIter = this->Internal->TrackIdMap.begin();
       trackIter != this->Internal->TrackIdMap.end(); ++trackIter)
    {
    vtkVgTrack* track = trackIter->second.GetTrack();

    if (track)
      {
      vtkVgScalars* scalars = track->GetScalars(name);
      if (!scalars)
        {
        continue;
        }
      const double* scalarsRange = scalars->GetRange();
      if (!scalarsRange)
        {
        continue;
        }

      if (scalarsRange[0] < overallRange.lower)
        {
        overallRange.lower = scalarsRange[0];
        }
      if (scalarsRange[1] > overallRange.upper)
        {
        overallRange.upper = scalarsRange[1];
        }
      }
    }

  this->Internal->ScalarsNameToRangeMap[name] = overallRange;
  vtkTimeStamp lastComputedTimeStamp;
  lastComputedTimeStamp.Modified();
  this->Internal->ScalarsNameToTimeMap[name] = lastComputedTimeStamp;

  return overallRange;
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTrackModel::GetNextAvailableId()
{
  if (this->Internal->TrackIdMap.empty())
    {
    return 0;
    }
  return (--this->Internal->TrackIdMap.end())->first + 1;
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::UpdateTemporalFiltering(vtkVgTrackInfo& info)
{
  vtkVgTrack* track = info.GetTrack();

  bool pass =
    this->TemporalFilters->EvaluateInterval(track->GetStartFrame(),
                                            track->GetEndFrame());
  if (!pass)
    {
    info.SetPassesFiltersOff();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackModel::UpdateSpatialFiltering(vtkVgTrackInfo& info)
{
  vtkVgTrack* track = info.GetTrack();

  bool pass =
    this->ContourOperatorManager->EvaluatePath(track->GetPoints(),
                                               track->GetPointIds());
  if (!pass)
    {
    info.SetPassesFiltersOff();
    }
}


//-----------------------------------------------------------------------------
void vtkVgTrackModel::UpdateColorOfTracksOfType(int typeIndex, double *rgb)
{
  bool modified = false;
  for (TrackMapIterator trackIter = this->Internal->TrackIdMap.begin();
       trackIter != this->Internal->TrackIdMap.end(); ++trackIter)
    {
    if (trackIter->second.GetTrack()->GetType() == typeIndex)
      {
      trackIter->second.GetTrack()->SetColor(rgb);
      modified = true;
      }
    }
  if (modified)
    {
    this->Modified();
    }
}
