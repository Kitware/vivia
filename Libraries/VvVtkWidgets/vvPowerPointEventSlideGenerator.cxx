/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvPowerPointEventSlideGenerator.h"

#include "vvPowerPointTrackSlideGenerator.h"
#include "vvPowerPointWriter.h"

#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgTrack.h>
#include <vtkVgUtil.h>

#include <vtkNew.h>
#include <vtkPoints.h>

#include <QDebug>

//-----------------------------------------------------------------------------
vvPowerPointEventSlideGenerator::vvPowerPointEventSlideGenerator(
  vtkVgEvent* event, vtkVgEventTypeRegistry* registry) :
  Event(event), Registry(registry)
{
}

//-----------------------------------------------------------------------------
vvPowerPointEventSlideGenerator::~vvPowerPointEventSlideGenerator()
{
}

//-----------------------------------------------------------------------------
bool vvPowerPointEventSlideGenerator::generateSlide(
  vvPowerPointWriter& writer, vgVideoRequestor& videoRequestor, int outputId,
  bool generateVideo, QProgressDialog* progress, int& progressCount)
{
  if (!writer.startNewSlide("event-item-detail"))
    {
    return true;
    }

  // If the various frame times needed by the parent generateSlide fn are not
  // valid, set them now.
  const vtkVgTimeStamp start = this->Event->GetStartFrame();
  const vtkVgTimeStamp end = this->Event->GetEndFrame();
  if (!this->VideoStartFrame.IsValid())
    {
    this->VideoStartFrame = start;
    }
  if (!this->VideoEndFrame.IsValid())
    {
    this->VideoEndFrame = end;
    }
  if (!this->RequestedStillFrame.IsValid())
    {
    // Compute timestamp halfway through the event
    this->RequestedStillFrame.SetTime(0.5 * (start.GetTime() + end.GetTime()));
    if (start.HasFrameNumber() && end.HasFrameNumber())
      {
      const unsigned int frame =
        (start.GetFrameNumber() + end.GetFrameNumber()) / 2;
      this->RequestedStillFrame.SetFrameNumber(frame);
      }

    // Set timestamp to the next closest frame with a valid region; if there
    // are no regions, just use the already computed timestamp
    vtkIdType npts;
    vtkIdType* ids;
    if (this->Event->GetNumberOfRegions() > 0 &&
        !this->Event->GetClosestDisplayRegion(this->RequestedStillFrame,
                                              npts, ids) &&
        npts == 0)
      {
      qDebug() << "Failed to get event midpoint region.";
      return true;
      }
    }

  return vvPowerPointSlideGenerator::generateSlide(
           writer, videoRequestor, outputId, generateVideo,
           progress, progressCount);
}

//-----------------------------------------------------------------------------
int vvPowerPointEventSlideGenerator::outputFrameCount(bool generateVideo)
{
  if (!generateVideo)
    {
    return 1;
    }

  vtkVgTimeStamp start =
    this->VideoStartFrame.IsValid() ? this->VideoStartFrame
                                    : this->Event->GetStartFrame();
  vtkVgTimeStamp end =
    this->VideoEndFrame.IsValid() ? this->VideoEndFrame
                                  : this->Event->GetEndFrame();
  return this->numberOfOutputVideoFrames(start, end) + 1;
}

//-----------------------------------------------------------------------------
void vvPowerPointEventSlideGenerator::writeSummary(vvPowerPointWriter& writer,
                                                   const ImageInfo& imageInfo,
                                                   int outputId)
{
  const int imageId =
    writer.addImage("event-image", imageInfo.Data, outputId, false);

  writer.addDateTime("event-start-time",
                     this->Event->GetStartFrame().GetRawTimeStamp(),
                     "DATE OF IMAGE: ", "TIME OF IMAGE: ");

  const QString note = QString::fromLocal8Bit(this->Event->GetNote());
  if (note.isEmpty())
    {
    writer.addText("event-comment", "COMMENT: TYPE COMMENTS OR DELETE");
    }
  else
    {
    const QString comment = "COMMENT: " + note;
    writer.addText("event-comment", qPrintable(comment));
    }

  this->addEventRegion(writer, this->Event, imageInfo.Transform,
                       imageInfo.TimeStamp, "region-boundary", imageId);

  // Set Track information (of the event), if present
  for (unsigned int i = 0; i < this->Event->GetNumberOfTracks(); ++i)
    {
    vtkVgTrack* track;
    vtkVgTimeStamp trackStart;
    vtkVgTimeStamp trackEnd;
    this->Event->GetTrack(i, track, trackStart, trackEnd);
    vvPowerPointTrackSlideGenerator::addTrack(
      writer, track, trackStart, trackEnd, imageInfo.Transform,
      "event-path", imageId);
    }
}

//-----------------------------------------------------------------------------
void vvPowerPointEventSlideGenerator::writeVideoFrame(
  vvPowerPointWriter& writer, const ImageInfo& imageInfo,
  int outputId, int frameNumber)
{
  // Add event representation
  vtkNew<vtkVgEventBase> eventCopy;
  vtkNew<vtkVgEventModel> eventModel;
  vtkNew<vtkVgEventRegionRepresentation> eventRep;

  // Need to make a copy of the event so that it points to the right points
  eventCopy->SetRegionPoints(eventModel->GetSharedRegionPoints());
  eventCopy->DeepCopy(this->Event);

  eventModel->AddEvent(eventCopy.GetPointer());
  eventRep->SetEventModel(eventModel.GetPointer());
  eventRep->SetEventTypeRegistry(this->Registry);
  eventRep->SetRegionZOffset(0.1);

  if (imageInfo.Transform)
    {
    eventRep->SetRepresentationMatrix(imageInfo.Transform);
    }

  eventModel->Update(imageInfo.TimeStamp);
  eventRep->Update();

  writer.writeVideoImageWithRep(imageInfo.Data,
                                eventRep->GetActiveRenderObjects(),
                                outputId, frameNumber);
}

//-----------------------------------------------------------------------------
void vvPowerPointEventSlideGenerator::createVideo(vvPowerPointWriter& writer,
                                                  int outputId)
{
  QString videoName = writer.createVideo(outputId);
  if (!videoName.isEmpty())
    {
    writer.addVideo("event-video", qPrintable(videoName), false);
    }
}

//-----------------------------------------------------------------------------
int vvPowerPointEventSlideGenerator::addEventRegion(
  vvPowerPointWriter& writer, vtkVgEvent* event, const vtkMatrix4x4* transform,
  const vtkVgTimeStamp& timeStamp, const char* itemTemplateName, int imageId)
{
  vtkIdType* pts;
  vtkIdType npts;
  event->GetRegion(timeStamp, npts, pts);
  if (!npts)
    {
    return -1;
    }

  vtkPoints* points = event->GetRegionPoints();
  QVector<float> regionPoints;
  regionPoints.reserve(npts * 2);
  for (vtkIdType i = 0; i < npts; ++i)
    {
    double point[3];
    points->GetPoint(pts[i], point);
    if (transform)
      {
      vtkVgApplyHomography(point, transform, point);
      }

    regionPoints.append(point[0]);
    regionPoints.append(point[1]);
    }

  return writer.addPolyLine(itemTemplateName, imageId, regionPoints);
}
