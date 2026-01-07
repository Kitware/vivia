// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvPowerPointSlideGenerator.h"

#include <vgVideoRequestor.h>

#include <vtkVgInstance.h>

#include <vtkMatrix4x4.h>

#include <QDebug>
#include <QProgressDialog>

//-----------------------------------------------------------------------------
bool vvPowerPointSlideGenerator::generateSlide(
  vvPowerPointWriter& writer, vgVideoRequestor& videoRequestor, int outputId,
  bool generateVideo, QProgressDialog* progress, int& progressCount)
{
  vtkVgVideoFrame frame;
  if (!videoRequestor.requestFrame(frame, this->RequestedStillFrame,
                                   vg::SeekNearest))
    {
    qDebug() << "Frame request failed: report frame discarded.";
    return true;
    }

  vtkVgInstance<vtkMatrix4x4> xf;
  ImageInfo imageInfo(xf);

  // Invert the homography to map back to image coordinates
  this->invertHomography(frame.MetaData.Homography, xf);

  /* TODO: context image not used in vvPowerPointWriter at this time
  vtkSmartPointer<vtkMatrix4x4> imageToLatLon;
  if (frame.MetaData.WorldLocation.GCS != -1)
  {
  imageToLatLon = frame.MetaData.MakeImageToLatLonMatrix();
  }
  */

  imageInfo.Data = frame.Image.GetVolatilePointer();
  imageInfo.TimeStamp = frame.MetaData.Time;

  this->writeSummary(writer, imageInfo, outputId);

  if (progress->wasCanceled())
    {
    writer.cancel();
    return false;
    }

  // Done with this object if we're not writing video images
  if (!generateVideo)
    {
    progress->setValue(++progressCount);
    return true;
    }

  int framenum = 0;
  vtkVgTimeStamp seekTime = this->VideoStartFrame;
  vg::SeekMode seekDirection = vg::SeekNearest;

  do
    {
    if (!videoRequestor.requestFrame(frame, seekTime, seekDirection))
      {
      // If request fails, we must have stepped outside the bounds of the
      // video?
      break;
      }

    // Convert the homography, which includes a Y flip, to map back to
    // VTK image coordinates
    imageInfo.Data = frame.Image.GetVolatilePointer();
    imageInfo.TimeStamp = frame.MetaData.Time;
    this->invertHomography(frame.MetaData.Homography, xf,
                           imageInfo.Data->GetDimensions()[1]);

    this->writeVideoFrame(writer, imageInfo, outputId, ++framenum);

    // Set up the seek to give the next frame
    seekTime = frame.MetaData.Time;
    seekDirection = vg::SeekNext;

    if (progress->wasCanceled())
      {
      writer.cancel();
      return false;
      }

    progress->setValue(++progressCount);

    }
  while (seekTime < this->VideoEndFrame);

  this->createVideo(writer, outputId);
  return true;
}

//-----------------------------------------------------------------------------
int vvPowerPointSlideGenerator::numberOfOutputVideoFrames(
  vtkVgTimeStamp start, vtkVgTimeStamp end)
{
  // If we have frame numbers, assume we have every frame
  if (start.HasFrameNumber() && end.HasFrameNumber())
    {
    return end.GetFrameNumber() - start.GetFrameNumber() + 1;
    }

  // If we don't have frame numbers (and do have time), assume 10 fps
  if (start.HasTime() && end.HasTime())
    {
    return end.GetTimeDifferenceInSecs(start) * 10 + 1;
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vvPowerPointSlideGenerator::invertHomography(
  vtkMatrix4x4* in, vtkMatrix4x4* out)
{
  out->DeepCopy(in);
  out->Invert();
}

//-----------------------------------------------------------------------------
void vvPowerPointSlideGenerator::invertHomography(
  vtkMatrix4x4* in, vtkMatrix4x4* out, int height)
{
  vvPowerPointSlideGenerator::invertHomography(in, out);

  vtkVgInstance<vtkMatrix4x4> fy;
  fy->Identity();
  fy->SetElement(1, 1, -1);
  fy->SetElement(1, 3, height - 1);
  vtkMatrix4x4::Multiply4x4(fy, out, out);
}
