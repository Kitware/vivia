/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqArchiveVideoSource.h"

#include <vtkVgVideoFrameData.h>
#include <vtkVgVideoMetadata.h>

#include <vtkVgAdapt.h>

#include <vtkVgAdaptImage.h>
#include <vtkVgVideoFrameMetaData.h>

#include <vgKwaArchive.h>
#include <vgKwaFrameMetadata.h>
#include <vgKwaVideoClip.h>

#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>

#include <qtStlUtil.h>

#include <QUrlQuery>

#include <algorithm>

vtkStandardNewMacro(vqArchiveVideoSource);

//-----------------------------------------------------------------------------
vqArchiveVideoSource::vqArchiveVideoSource() : vtkVgVideoProviderBase(),
  LastLoopingState(Looping),
  CurrentClip(0)
{
  this->CurrentVideoFrameData = new vtkVgVideoFrameData();

  // Is this a valid assumption?
  this->SetTimeInterval(1.0);
}

//-----------------------------------------------------------------------------
vqArchiveVideoSource::~vqArchiveVideoSource()
{
  delete this->CurrentVideoFrameData;
  this->CurrentVideoFrameData = 0;
}

//-----------------------------------------------------------------------------
void vqArchiveVideoSource::PrintSelf(ostream& os, vtkIndent indent)
{
  // \TODO: Implement this later.
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::AcquireVideoClip(vgKwaArchive* videoArchive)
{
  // Get clip
  vgKwaArchive::Request request;
  request.MissionId = QString::fromLocal8Bit(this->MissionId);
  request.StreamId = QString::fromLocal8Bit(this->StreamId);
  request.StartTime = this->TimeRange[0];
  request.EndTime = this->TimeRange[1];
  request.Padding = this->RequestedPadding;

  QUrl clipUri;
  vgKwaVideoClip* clip = videoArchive->getClip(request, &clipUri);

  if (!clip)
    {
    this->SetVideoClip(0, QUrl());
    return VTK_ERROR; // no clip found
    }

  clipUri.setQuery(QUrlQuery{});
  return this->SetVideoClip(clip, clipUri);
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::AcquireVideoClip(QUrl clipUri)
{
  // Not expecting limits in URI, but strip query just in case...
  clipUri.setQuery(QUrlQuery{});

  // Obtain full clip
  QScopedPointer<vgKwaVideoClip> clip(new vgKwaVideoClip(clipUri));
  if (clip->frameCount() == 0)
    {
    this->SetVideoClip(0, QUrl());
    return VTK_ERROR; // failed to load clip
    }

  // If a time range is set, obtain the appropriate sub-clip
  if (this->TimeRange[0] != -1 || this->TimeRange[1] != -1)
    {
    vgKwaVideoClip* subClip =
      clip->subClip(this->TimeRange[0], this->TimeRange[1],
                    this->RequestedPadding);
    if (!subClip)
      {
      this->SetVideoClip(0, QUrl());
      return VTK_ERROR; // failed to obtain sub-clip
      }
    clip.reset(subClip);
    }

  return this->SetVideoClip(clip.take(), clipUri);
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::SetVideoClip(
  vgKwaVideoClip* clip, const QUrl& clipUri)
{
  this->CurrentClip = QSharedPointer<vgKwaVideoClip>(clip);
  this->CurrentClipUri = clipUri;

  if (!clip)
    {
    return VTK_OK;
    }

  // If a time range was requested...
  if (this->TimeRange[0] != -1 || this->TimeRange[1] != -1)
    {
    // ...save the original time as time mark
    this->AddTimeMark(this->TimeRange[0], this->TimeRange[1]);
    }

  // Set time range to be what we actually get back
  this->SetTimeRange(this->CurrentClip->firstTime().Time,
                     this->CurrentClip->lastTime().Time);

  // Save the actual time as time mark
  this->AddTimeMark(this->TimeRange[0], this->TimeRange[1]);

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetFrame(vtkVgVideoFrameData* frameData, double time)
{
  if (this->CurrentClip)
    {
    this->LastLoopingState = this->Looping;

    // Prevent seeking outside of clip range, which could happen 'by accident'
    // due to floating point round-off error
    const double firstTime = this->CurrentClip->firstTime().Time;
    const double lastTime = this->CurrentClip->lastTime().Time;
    time = qBound(firstTime, time, lastTime);

    const vgTimeStamp newTimeStamp =
      this->CurrentClip->seek(vgTimeStamp::fromTime(time), vg::SeekLowerBound);
    if (newTimeStamp.IsValid())
      {
      this->CopyFrameData(frameData);

      emit this->frameAvailable();

      return VTK_OK;
      }
    }

  return VTK_ERROR;
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetFrame(vtkVgVideoFrameData* vtkNotUsed(frameData),
                                   int vtkNotUsed(frameNumber))
{
  // \TODO: Need to implement this.
  return VTK_ERROR;
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetNextFrame(vtkVgVideoFrameData* frameData)
{
  if (this->CurrentClip)
    {
    this->LastLoopingState = this->Looping;

    // Advance the clip first.
    if (this->Advance())
      {
      this->CopyFrameData(frameData);

      emit this->frameAvailable();

      return VTK_OK;
      }
    }

  return VTK_ERROR;
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetCurrentFrame(vtkVgVideoFrameData* frameData)
{
  frameData->TimeStamp = this->CurrentVideoFrameData->TimeStamp;
  frameData->VideoImage->ShallowCopy(this->CurrentVideoFrameData->VideoImage);
  frameData->VideoMatrix = this->CurrentVideoFrameData->VideoMatrix;
  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetPreviousFrame(vtkVgVideoFrameData* frameData)
{
  if (this->CurrentClip)
    {
    this->LastLoopingState = this->Looping;

    // Recede the clip first.
    this->Recede();

    this->CopyFrameData(frameData);

    emit this->frameAvailable();

    return VTK_OK;
    }
  else
    {
    return VTK_ERROR;
    }
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetNumberOfFrames()
{
  if (this->CurrentClip)
    {
    unsigned int firstFrameNumber = this->CurrentClip->firstTime().FrameNumber;
    unsigned int lastFrameNumber = this->CurrentClip->lastTime().FrameNumber;
    return ((lastFrameNumber - firstFrameNumber) + 1);
    }

  return -1;
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetCurrentMetadata(vtkVgVideoMetadata* metadata)
{
  if (!this->CurrentClip)
    {
    return VTK_ERROR;
    }

  vgKwaFrameMetadata fmd = this->CurrentClip->currentMetadata();
  if (!fmd.timestamp().IsValid())
    {
    return VTK_ERROR;
    }

  this->CopyMetadata(metadata, fmd);
  return VTK_OK;
}

//-----------------------------------------------------------------------------
std::map<vtkVgTimeStamp, vtkVgVideoMetadata> vqArchiveVideoSource::GetMetadata()
{
  if (!this->CurrentClip)
    {
    return std::map<vtkVgTimeStamp, vtkVgVideoMetadata>();
    }

  vgKwaVideoClip::MetadataMap src = this->CurrentClip->metadata();
  std::map<vtkVgTimeStamp, vtkVgVideoMetadata> dst;

  // Build new map containing metadata in VTK types
  foreach (const vgKwaFrameMetadata& fmd, src)
    {
    vtkVgVideoMetadata vtkMetaData;
    this->CopyMetadata(&vtkMetaData, fmd);
    dst.insert(std::make_pair(fmd.timestamp(), vtkMetaData));
    }

  return dst;
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetVideoHeight()
{
  static const int bestGuess = 480;

  if (!this->CurrentClip)
    {
    return bestGuess;
    }

  vgKwaFrameMetadata frameMetaData =
    this->CurrentClip->metadataAt(this->CurrentClip->firstTime());
  int heightFromMetaData = frameMetaData.imageSize().height();
  return (heightFromMetaData > 0 ? heightFromMetaData : bestGuess);
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::GetVideoWidth()
{
  static const int bestGuess = 640;

  if (!this->CurrentClip)
    {
    return bestGuess;
    }

  vgKwaFrameMetadata frameMetaData =
    this->CurrentClip->metadataAt(this->CurrentClip->firstTime());
  int widthFromMetaData = frameMetaData.imageSize().width();
  return (widthFromMetaData > 0 ? widthFromMetaData : bestGuess);
}

//-----------------------------------------------------------------------------
const vgKwaVideoClip* vqArchiveVideoSource::GetCurrentVideoClip() const
{
  return this->CurrentClip.data();
}

//-----------------------------------------------------------------------------
void vqArchiveVideoSource::Update()
{
  // Do nothing.
}

//-----------------------------------------------------------------------------
int vqArchiveVideoSource::Reset()
{
  if (!this->CurrentClip)
    {
    return VTK_ERROR;
    }

  this->CurrentClip->seek(vgTimeStamp::fromTime(this->TimeRange[0]));
  this->LastLoopingState = this->Looping;

  return VTK_OK;
}

//-----------------------------------------------------------------------------
bool vqArchiveVideoSource::SeekNearestEarlier(double time)
{
  // Prevent seeking before start of clip, which could happen 'by accident' due
  // to floating point round-off error
  time = qMin(time, this->CurrentClip->firstTime().Time);

  const vgTimeStamp newTimeStamp =
    this->CurrentClip->seek(vgTimeStamp::fromTime(time), vg::SeekPrevious);
  return newTimeStamp.IsValid();
}

//-----------------------------------------------------------------------------
bool vqArchiveVideoSource::Advance()
{
  // Advance to next time
  vgTimeStamp newTime = this->CurrentClip->advance();

  // If at end of video, and looping is enabled...
  if (!newTime.IsValid() && this->Looping)
    {
    // ...reset to start of video
    this->CurrentClip->rewind();
    }

  return this->CurrentClip->currentTimeStamp().IsValid();
}

//-----------------------------------------------------------------------------
bool vqArchiveVideoSource::Recede()
{
  // Recede to previous time
  vgTimeStamp newTime = this->CurrentClip->recede();

  // If at start of video, and looping is enabled...
  if (!newTime.IsValid() && this->Looping)
    {
    // ...reset to end of video
    this->CurrentClip->seek(this->CurrentClip->lastTime());
    }

  return this->CurrentClip->currentTimeStamp().IsValid();
}

//-----------------------------------------------------------------------------
void vqArchiveVideoSource::ShallowCopy(vtkVgDataSourceBase& other)
{
  this->Superclass::ShallowCopy(other);

  vqArchiveVideoSource* otherDerived =
    vqArchiveVideoSource::SafeDownCast(&other);

  if (otherDerived)
    {
    this->LastLoopingState = otherDerived->LastLoopingState;
    this->CurrentClipUri = otherDerived->CurrentClipUri;
    this->CurrentClip = otherDerived->CurrentClip;
    }
}

//-----------------------------------------------------------------------------
void vqArchiveVideoSource::DeepCopy(vtkVgDataSourceBase& other)
{
  vqArchiveVideoSource* otherDerived =
    vqArchiveVideoSource::SafeDownCast(&other);

  otherDerived ? this->DeepCopy(*otherDerived, CopyClipShared)
  : this->Superclass::DeepCopy(other);
}

//-----------------------------------------------------------------------------
void vqArchiveVideoSource::DeepCopy(vqArchiveVideoSource& other,
                                    DeepCopyMode mode)
{
  this->Superclass::DeepCopy(other);

  this->LastLoopingState = other.LastLoopingState;
  this->CurrentClipUri = other.CurrentClipUri;

  // Drop old clip and frame
  this->CurrentClip.clear();
  delete this->CurrentVideoFrameData;
  this->CurrentVideoFrameData = new vtkVgVideoFrameData();

  // Deep copy clip and frame from other
  if (other.CurrentClip)
    {
    vgKwaVideoClip* clonedClip;
    if (mode == CopyClipDetached)
      {
      clonedClip = new vgKwaVideoClip(this->CurrentClipUri);
      }
    else
      {
      clonedClip = other.CurrentClip->subClip(-1, -1, 0);
      }
    this->CurrentClip = QSharedPointer<vgKwaVideoClip>(clonedClip);
    this->CopyFrameData(0);
    }
}

//-----------------------------------------------------------------------------
void vqArchiveVideoSource::CopyFrameData(vtkVgVideoFrameData* frameData)
{
  if (!this->CurrentClip)
    {
    return;
    }

  const vgVideoFramePtr videoFrame = this->CurrentClip->currentFrame();
  const vgKwaFrameMetadata metaData = this->CurrentClip->currentMetadata();

  this->CurrentVideoFrameData->TimeStamp.SetTime(
    static_cast<double>(videoFrame.time().Time));
  this->CurrentVideoFrameData->VideoImage->ShallowCopy(
    vtkVgAdapt(videoFrame.image()));

  if (frameData)
    {
    frameData->TimeStamp = this->CurrentVideoFrameData->TimeStamp;
    frameData->VideoImage->ShallowCopy(this->CurrentVideoFrameData->VideoImage);
    }

  const vgKwaWorldBox srcCorners = metaData.worldCornerPoints();
  vtkVgVideoFrameMetaData frameMetaData;

  if (!metaData.imageSize().isEmpty())
    {
    frameMetaData.Width = metaData.imageSize().width();
    frameMetaData.Height = metaData.imageSize().height();
    }
  else // Fallback in case meta data didn't have image size
    {
    int* dims = this->CurrentVideoFrameData->VideoImage->GetDimensions();
    frameMetaData.SetWidthAndHeight(dims[0], dims[1]);
    }

  bool cornersValid = srcCorners.GCS != -1;
  double* bounds = 0;

  if (cornersValid)
    {
    vtkVgVideoFrameCorners& dstCorners = frameMetaData.WorldLocation;
    dstCorners.GCS = srcCorners.GCS;
    dstCorners.UpperLeft.Latitude   = srcCorners.UpperLeft.Latitude;
    dstCorners.UpperLeft.Longitude  = srcCorners.UpperLeft.Longitude;
    dstCorners.LowerLeft.Latitude   = srcCorners.LowerLeft.Latitude;
    dstCorners.LowerLeft.Longitude  = srcCorners.LowerLeft.Longitude;
    dstCorners.UpperRight.Latitude  = srcCorners.UpperRight.Latitude;
    dstCorners.UpperRight.Longitude = srcCorners.UpperRight.Longitude;
    dstCorners.LowerRight.Latitude  = srcCorners.LowerRight.Latitude;
    dstCorners.LowerRight.Longitude = srcCorners.LowerRight.Longitude;

    cornersValid = frameMetaData.AreCornerPointsValid();
    }

  if (cornersValid)
    {
    this->CurrentVideoFrameData->VideoMatrix =
      frameMetaData.MakeImageToLatLonMatrix();
    }
  else
    {
    vtkErrorMacro("Corner pts are NOT valid!");
    this->CurrentVideoFrameData->VideoMatrix = 0;
    }

  if (frameData)
    {
    frameData->VideoMatrix = this->CurrentVideoFrameData->VideoMatrix;
    }
}

//-----------------------------------------------------------------------------
void vqArchiveVideoSource::CopyMetadata(vtkVgVideoMetadata* dst,
                                        const vgKwaFrameMetadata& src)
{
  const auto& hmSrc = src.homography();
  vtkMatrix4x4* const hmDst = dst->Homography;
  hmDst->Identity();
  hmDst->SetElement(0, 0, hmSrc(0, 0));
  hmDst->SetElement(0, 1, hmSrc(0, 1));
  hmDst->SetElement(0, 3, hmSrc(0, 2));
  hmDst->SetElement(1, 0, hmSrc(1, 0));
  hmDst->SetElement(1, 1, hmSrc(1, 1));
  hmDst->SetElement(1, 3, hmSrc(1, 2));
  hmDst->SetElement(3, 0, hmSrc(2, 0));
  hmDst->SetElement(3, 1, hmSrc(2, 1));
  hmDst->SetElement(3, 3, hmSrc(2, 2));

  dst->Gsd = src.gsd();
  dst->HomographyReferenceFrame = src.homographyReferenceFrameNumber();
  dst->Time = src.timestamp();
}
