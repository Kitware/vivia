/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsQfVideoSource.h"

#include <vtkVgAdapt.h>
#include <vtkVgVideoFrameData.h>
#include <vtkVgVideoMetadata.h>

#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>

#include <vgVideoPlayer.h>
#include <vgVideoSource.h>

vtkStandardNewMacro(vsQfVideoSource);

//-----------------------------------------------------------------------------
void vsQfVideoSource::updateVideoFrame(
  vtkVgVideoFrame frame, qint64 seekRequestId)
{
  Q_UNUSED(seekRequestId);

  this->CurrentVideoFrameData->TimeStamp = frame.MetaData.Time;
  this->CurrentVideoFrameData->VideoImage = frame.Image.GetPointer();
  this->CurrentVideoMetadata->Gsd = frame.MetaData.Gsd;
  this->CurrentVideoMetadata->Time = frame.MetaData.Time;
  this->CurrentVideoMetadata->Homography = frame.MetaData.Homography;
  this->CurrentVideoMetadata->HomographyReferenceFrame =
    frame.MetaData.HomographyReferenceFrame;

  emit this->frameAvailable();
}

//-----------------------------------------------------------------------------
vsQfVideoSource::vsQfVideoSource()
  : vtkVgVideoProviderBase(), Player(new vgVideoPlayer)
{
  this->RequestId = 0;

  this->CurrentVideoFrameData = new vtkVgVideoFrameData();
  this->CurrentVideoMetadata = new vtkVgVideoMetadata();

  connect(this->Player.data(),
          SIGNAL(frameAvailable(vtkVgVideoFrame, qint64)),
          SLOT(updateVideoFrame(vtkVgVideoFrame, qint64)));

  // Is this a valid assumption?
  this->SetTimeInterval(1.0);
}

//-----------------------------------------------------------------------------
vsQfVideoSource::~vsQfVideoSource()
{
  delete this->CurrentVideoFrameData;
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::SetTimeRange(double a, double b)
{
  this->Superclass::SetTimeRange(a, b);
  this->UpdatePlayerFrameRange();
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::SetTimeRange(double range[2])
{
  this->Superclass::SetTimeRange(range);
  this->UpdatePlayerFrameRange();
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::UpdatePlayerFrameRange()
{
  this->Player->setSourceFrameRange(
    vtkVgTimeStamp(this->TimeRange[0]),
    vtkVgTimeStamp(this->TimeRange[1]));
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::SetSource(vgVideoSource* source)
{
  this->Player->setVideoSource(source);
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::Play()
{
  this->Player->setPlaybackSpeed(vgVideoPlayer::Playing);
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::Pause()
{
  this->Player->setPlaybackSpeed(vgVideoPlayer::Paused);
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::Stop()
{
  this->Player->setPlaybackSpeed(vgVideoPlayer::Stopped);
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::GetFrame(
  vtkVgVideoFrameData* vtkNotUsed(frameData), double time)
{
  this->Player->seekToTimestamp(this->RequestId++, vtkVgTimeStamp(time));

  // indicate asynchronous
  return -1;
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::GetFrame(
  vtkVgVideoFrameData* vtkNotUsed(frameData), int vtkNotUsed(frameNumber))
{
  // \TODO: Need to implement this.
  return VTK_ERROR;
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::GetNextFrame(vtkVgVideoFrameData* vtkNotUsed(frameData))
{
  double time = this->CurrentVideoFrameData->TimeStamp.GetTime() + 1.0e4;
  this->Player->seekToTimestamp(this->RequestId++, vtkVgTimeStamp(time),
                                vg::SeekLowerBound);

  // indicate asynchronous
  return -1;
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::GetCurrentFrame(vtkVgVideoFrameData* frameData)
{
  frameData->TimeStamp = this->CurrentVideoFrameData->TimeStamp;
  frameData->VideoImage->ShallowCopy(this->CurrentVideoFrameData->VideoImage);
  frameData->VideoMatrix = this->CurrentVideoFrameData->VideoMatrix;
  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::GetPreviousFrame(
  vtkVgVideoFrameData* vtkNotUsed(frameData))
{
  double time = this->CurrentVideoFrameData->TimeStamp.GetTime() - 1.0e4;
  this->Player->seekToTimestamp(this->RequestId++, vtkVgTimeStamp(time),
                                vg::SeekUpperBound);

  // indicate asynchronous
  return -1;
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::GetNumberOfFrames()
{
  return -1;
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::GetCurrentMetadata(vtkVgVideoMetadata* metadata)
{
  *metadata = *this->CurrentVideoMetadata;
  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::SetMetadata(
  const std::map<vtkVgTimeStamp, vtkVgVideoMetadata>& allMetadata)
{
  this->AllMetadata = allMetadata;
}

//-----------------------------------------------------------------------------
std::map<vtkVgTimeStamp, vtkVgVideoMetadata> vsQfVideoSource::GetMetadata()
{
  return this->AllMetadata;
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::GetVideoHeight()
{
  // TODO
  return -1;
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::Update()
{
  if (!this->CurrentVideoFrameData->TimeStamp.IsValid())
    {
    // Get first frame
    this->Player->seekToTimestamp(this->RequestId++,
                                  vtkVgTimeStamp(this->TimeRange[0]));
    }
}

//-----------------------------------------------------------------------------
int vsQfVideoSource::Reset()
{
  *this->CurrentVideoFrameData = vtkVgVideoFrameData();
  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::ShallowCopy(vtkVgDataSourceBase& other)
{
  this->Superclass::ShallowCopy(other);

  vsQfVideoSource* otherDerived = vsQfVideoSource::SafeDownCast(&other);

  if (otherDerived)
    {
    // TODO
    }
}

//-----------------------------------------------------------------------------
void vsQfVideoSource::DeepCopy(vtkVgDataSourceBase& other)
{
  this->Superclass::DeepCopy(other);

  vsQfVideoSource* otherDerived = vsQfVideoSource::SafeDownCast(&other);

  if (otherDerived)
    {
    // TODO
    }
}
