// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgVideoModel0.h"

#include "vtkVgEventModel.h"
#include "vtkVgTimeStamp.h"
#include "vtkVgTrackModel.h"
#include "vtkVgVideoProviderBase.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkTimerLog.h>

// C++
#include <vector>

vtkStandardNewMacro(vtkVgVideoModel0);
vtkCxxSetObjectMacro(vtkVgVideoModel0, EventModel, vtkVgEventModel);
vtkCxxSetObjectMacro(vtkVgVideoModel0, TrackModel, vtkVgTrackModel);

//-----------------------------------------------------------------------------
vtkVgVideoModel0::vtkVgVideoModel0() : vtkVgModelBase(),
  NumberOfFrames(0),
  VisibleScale(1),
  UseSourceTimeStamp(false),
  Playing(0),
  Paused(0),
  Stopped(0),
  Looping(0),
  LastLoopingState(Looping),
  UseFrameIndex(1),
  VideoSource(NULL),
  EventModel(NULL),
  TrackModel(NULL),
  PlayFromBeginning(0),
  UseTimeStamp(1),
  PlaybackSpeed(1.0),
  CurrentSeekTime(0.0),
  LastAppTime(0.0),
  CurrentAppTime(0.0)
{
  this->ClipTimeRangeCache[0] = this->ClipTimeRangeCache[1] = 0.0;

  this->VideoFrameData = new vtkVgVideoFrameData();

  for (int i = 0; i < 6; ++i)
    {
    this->VisibleExtents[i] = -1.0;
    }
}

//-----------------------------------------------------------------------------
vtkVgVideoModel0::~vtkVgVideoModel0()
{
  delete this->VideoFrameData;
  this->VideoFrameData = NULL;
  this->SetEventModel(0);
  this->SetTrackModel(0);
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel0::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  // \TODO Fill this one up.
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel0::SetVideoSource(vtkVgVideoProviderBase* videoSource)
{
  if (videoSource && this->VideoSource != videoSource)
    {
    this->VideoSource = videoSource;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vtkVgVideoProviderBase* vtkVgVideoModel0::GetVideoSource()
{
  return this->VideoSource;
}

//-----------------------------------------------------------------------------
const vtkVgVideoProviderBase* vtkVgVideoModel0::GetVideoSource() const
{
  return this->VideoSource;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::Update(
  const vtkVgTimeStamp& timeStamp,
  const vtkVgTimeStamp* referenceFrameTimeStamp/*=0*/)
{
  this->CurrentTimeStamp = timeStamp;

  // \TODO Pass the visibile extents and scale to the source
  int retVal = VTK_OK;

  // Skip padding always when in neutral (play,pause,stop = fasle) state
  if (!this->Initialized)
    {
    this->SkipPadding();
    }

  if (this->Playing || !this->GetInitialized())
    {
    retVal = this->ActionPlay(timeStamp, referenceFrameTimeStamp);

    switch (retVal)
      {
      case VTK_OK:
        this->OnActionPlaySuccess(timeStamp, referenceFrameTimeStamp);
        break;
      case VTK_ERROR:
        this->OnActionPlayFailure(timeStamp, referenceFrameTimeStamp);
        break;
      default:
        break;
      }
    }
  else if (this->Paused)
    {
    this->ActionPause(timeStamp, referenceFrameTimeStamp);
    }
  else if (this->Stopped)
    {
    this->ActionStop(timeStamp, referenceFrameTimeStamp);
    }

  return retVal;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::Play()
{
  if (!this->Paused && this->PlayFromBeginning)
    {
    this->UndoSkipPadding();

    // Since play by default advances the clip and since
    // the first clip has not been rendered yet, recede to previous
    // frame to show off the first frame when playing.
    if (!this->UseTimeStamp)
      {
      this->VideoSource->Recede();
      }
    }

  this->Playing = 1;
  this->Paused  = 0;
  this->Stopped = 0;

  // Reset cached application time
  this->LastAppTime = 0.0;

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::IsPlaying()
{
  return this->Playing;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::Pause()
{
  this->Playing = 0;
  this->Paused  = 1;
  this->Stopped = 0;

  this->PlayFromBeginningOff();

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::IsPaused()
{
  return this->Paused;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::Stop()
{
  this->Playing = 0;
  this->Paused  = 0;
  this->Stopped = 1;

  this->CurrentAppTime  = 0.0;
  this->LastAppTime     = this->CurrentAppTime;
  this->CurrentSeekTime = this->ClipTimeRangeCache[0];

  this->PlayFromBeginningOff();

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::IsStopped()
{
  return this->Stopped;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::Next()
{
  int retVal = VTK_ERROR;

  // If we are playing this won't do anything.
  if (this->IsPlaying())
    {
    this->Pause();
    }

  if (this->VideoSource)
    {
    this->VideoSource->Update();
    retVal = this->VideoSource->GetNextFrame(this->VideoFrameData);

    if (retVal == VTK_OK)
      {
      this->UpdateDataRequestTime.Modified();

      // Also invoke the event.
      this->InvokeEvent(vtkCommand::UpdateDataEvent);

      if (this->TrackModel)
        {
        this->TrackModel->Update(this->VideoFrameData->TimeStamp);
        }

      // We may need to clarify what is expected here.
      if (this->EventModel)
        {
        this->EventModel->Update(this->VideoFrameData->TimeStamp);
        }
      }
    }

  return retVal;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::Previous()
{
  int retVal = VTK_ERROR;

  if (this->Playing)
    {
    retVal = VTK_ERROR;
    return retVal;
    }

  if (this->VideoSource)
    {
    this->VideoSource->Update();
    retVal = this->VideoSource->GetPreviousFrame(this->VideoFrameData);

    if (retVal == VTK_OK)
      {
      this->UpdateDataRequestTime.Modified();

      // Also invoke the event.
      this->InvokeEvent(vtkCommand::UpdateDataEvent);

      if (this->TrackModel)
        {
        this->TrackModel->Update(this->VideoFrameData->TimeStamp);
        }

      // We may need to clarify what is expected here.
      if (this->EventModel)
        {
        this->EventModel->Update(this->VideoFrameData->TimeStamp);
        }
      }
    }

  return retVal;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::SeekTo(double time)
{
  if (this->Playing)
    {
    return VTK_ERROR;
    }

  if (!this->VideoSource)
    {
    return VTK_ERROR;
    }

  this->VideoSource->Update();
  int retVal = this->VideoSource->GetFrame(this->VideoFrameData, time);

  if (retVal == VTK_OK)
    {
    // Update internal seek time
    this->CurrentSeekTime = time;

    this->UpdateDataRequestTime.Modified();

    // Also invoke the event.
    this->InvokeEvent(vtkCommand::UpdateDataEvent);

    if (this->TrackModel)
      {
      this->TrackModel->Update(this->VideoFrameData->TimeStamp);
      }

    // We may need to clarify what is expected here.
    if (this->EventModel)
      {
      this->EventModel->Update(this->VideoFrameData->TimeStamp);
      }
    }

  return retVal;
}

//-----------------------------------------------------------------------------
const vtkVgVideoFrameData* vtkVgVideoModel0::GetFrameData()
{
  return this->VideoFrameData;
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel0::SetVideoMatrix(vtkMatrix4x4* matrix)
{
  if (!matrix || (this->VideoFrameData->VideoMatrix == matrix))
    {
    return;
    }

  // Don't need to register as done by the assignment operator.
  this->VideoFrameData->VideoMatrix = matrix;

  this->UpdateDataRequestTime.Modified();
}

//-----------------------------------------------------------------------------
unsigned long vtkVgVideoModel0::GetUpdateTime()
{
  return this->UpdateTime.GetMTime();
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel0::SetInitialized(int arg)
{
  if (this->Initialized != arg)
    {
    this->Initialized = arg;
    if (!this->Initialized)
      {
      // reset to initial system state
      this->Playing = 0;
      this->Paused = 0;
      this->Stopped = 0;
      }
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::ActionPlay(
  const vtkVgTimeStamp& timeStamp,
  const vtkVgTimeStamp* vtkNotUsed(referenceFrameTimeStamp))
{
  // Mostly expecting success
  int retVal = VTK_OK;

  this->UpdateTime.Modified();

  // Update the source first.
  this->VideoSource->Update();

  retVal = (this->UseTimeStamp ? this->GetFrameUsingTimeStamp(timeStamp) :
            this->GetSequencialFrame());

  if (!this->VideoFrameData->VideoMatrix)
    {
    return VTK_ERROR;
    }

  return retVal;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::ActionPause(
  const vtkVgTimeStamp& vtkNotUsed(timeStamp),
  const vtkVgTimeStamp* vtkNotUsed(referenceFrameTimeStamp))
{
  // Do nothing
  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::ActionStop(
  const vtkVgTimeStamp& vtkNotUsed(timeStamp),
  const vtkVgTimeStamp* vtkNotUsed(referenceFrameTimeStamp))
{
  this->SetInitialized(0);
  this->UpdateTime.Modified();
  this->VideoSource->Reset();

  this->InvokeEvent(vtkCommand::UpdateDataEvent);
  this->InvokeEvent(vtkCommand::EndAnimationCueEvent);

  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel0::OnActionPlaySuccess(
  const vtkVgTimeStamp& timeStamp,
  const vtkVgTimeStamp* referenceFrameTimeStamp)
{
  this->UpdateDataRequestTime.Modified();

  // Also invoke events
  this->InvokeEvent(vtkCommand::UpdateDataEvent);

  this->InvokeEvent(vtkCommand::AnimationCueTickEvent);

  // Update models only when we have a valid frame data.
  if (this->TrackModel)
    {
    this->TrackModel->Update(this->UseSourceTimeStamp ?
                             this->VideoFrameData->TimeStamp :
                             timeStamp, referenceFrameTimeStamp);
    }

  if (this->EventModel)
    {
    this->EventModel->Update(this->UseSourceTimeStamp ?
                             this->VideoFrameData->TimeStamp :
                             timeStamp, referenceFrameTimeStamp);
    }

  this->SetInitialized(1);
}

//-----------------------------------------------------------------------------
void vtkVgVideoModel0::OnActionPlayFailure(
  const vtkVgTimeStamp& timeStamp,
  const vtkVgTimeStamp* referenceFrameTimeStamp)
{
  this->InvokeEvent(vtkCommand::AnimationCueTickEvent);
  this->InvokeEvent(vtkCommand::UpdateDataEvent);
  if (this->TrackModel)
    {
    this->TrackModel->Update(this->UseSourceTimeStamp ?
                             this->VideoFrameData->TimeStamp :
                             timeStamp, referenceFrameTimeStamp);
    }

  if (this->EventModel)
    {
    this->EventModel->Update(this->UseSourceTimeStamp ?
                             this->VideoFrameData->TimeStamp :
                             timeStamp, referenceFrameTimeStamp);
    }

  this->SetInitialized(1);
  this->InvokeEvent(vtkCommand::ErrorEvent);
}

//-----------------------------------------------------------------------------
bool vtkVgVideoModel0::SkipPadding(bool updateSouce)
{
  std::vector<std::pair<double, double> > timeMarks;
  timeMarks = this->VideoSource->GetTimeMarks();

  if (timeMarks.empty())
    {
    // This should not happen
    return false;
    }
  else
    {
    double timeRange[2];
    this->VideoSource->GetTimeRange(timeRange);

    this->ClipTimeRangeCache[0] = timeMarks[0].first;

    // Do not skip padding for the end time
    this->ClipTimeRangeCache[1] = timeRange[1];

    this->CurrentSeekTime = this->ClipTimeRangeCache[0];

    // Assuming that first time mark should be the original start time
    if (updateSouce)
      {
      return this->VideoSource->SeekNearestEarlier(this->CurrentSeekTime);
      }

    return false;
    }
}

//-----------------------------------------------------------------------------
bool vtkVgVideoModel0::UndoSkipPadding(bool updateSouce)
{
  this->VideoSource->GetTimeRange(this->ClipTimeRangeCache);

  this->CurrentSeekTime = this->ClipTimeRangeCache[0];

  if (updateSouce)
    {
    return this->VideoSource->SeekNearestEarlier(this->CurrentSeekTime);
    }

  return true;
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::GetFrameUsingTimeStamp(const vtkVgTimeStamp& timeStamp)
{
  this->UseSourceTimeStamp = true;

  // Use internal time and calculate delta time using application timestamp
  if (this->UseInternalTimeStamp)
    {
    // Time is in microseconds
    this->CurrentAppTime = timeStamp.GetTime();

    // This will happen if user has paused or stopped
    if (this->LastAppTime <= 0.0)
      {
      this->LastAppTime = this->CurrentAppTime;
      }

    const double delta = (this->CurrentAppTime - this->LastAppTime) *
                         (this->PlaybackSpeed);

    this->CurrentSeekTime += delta;

    this->LastAppTime = this->CurrentAppTime;

    int retVal = this->VideoSource->GetFrame(
                   this->VideoFrameData, this->CurrentSeekTime);

    // \TODO When not looping we need to stop and notify observers that
    // we have reached the end of the video.
    if (this->Looping && this->CurrentSeekTime > this->ClipTimeRangeCache[1])
      {
      this->UndoSkipPadding(false);
      this->LastAppTime = 0.0;
      }

    return retVal;
    }

  // Use application timestamp as it is
  if (this->UseFrameIndex && this->Looping)
    {
    const int frameNumber = static_cast<int>(
      timeStamp.GetFrameNumber() % this->VideoSource->GetNumberOfFrames());

    return this->VideoSource->GetFrame(this->VideoFrameData, frameNumber);
    }
  else if (this->UseFrameIndex && !this->Looping)
    {
    const int frameNumber = static_cast<int>(timeStamp.GetFrameNumber());
    return this->VideoSource->GetFrame(this->VideoFrameData, frameNumber);
    }
  else
    {
    // Looping is not supported in this case
    return this->VideoSource->GetFrame(this->VideoFrameData, timeStamp.GetTime());
    }
}

//-----------------------------------------------------------------------------
int vtkVgVideoModel0::GetSequencialFrame()
{
  this->UseSourceTimeStamp = true;

  if (this->LastLoopingState != this->Looping)
    {
    this->VideoSource->SetLooping(this->Looping);
    this->LastLoopingState = this->Looping;
    }

  return this->VideoSource->GetNextFrame(this->VideoFrameData);
}
