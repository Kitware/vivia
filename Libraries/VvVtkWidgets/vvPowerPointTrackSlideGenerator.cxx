// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvPowerPointTrackSlideGenerator.h"

#include "vvPowerPointWriter.h"

#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>
#include <vtkVgUtil.h>

#include <vtkNew.h>
#include <vtkPoints.h>

#include <QDebug>

//-----------------------------------------------------------------------------
vvPowerPointTrackSlideGenerator::vvPowerPointTrackSlideGenerator(
  vtkVgTrack* track) : Track(track)
{
}

//-----------------------------------------------------------------------------
vvPowerPointTrackSlideGenerator::~vvPowerPointTrackSlideGenerator()
{
}

//-----------------------------------------------------------------------------
bool vvPowerPointTrackSlideGenerator::generateSlide(
  vvPowerPointWriter& writer, vgVideoRequestor& videoRequestor, int outputId,
  bool generateVideo, QProgressDialog* progress, int& progressCount)
{
  if (!writer.startNewSlide("track-item-detail"))
    {
    return true;
    }

  // If the various frame times needed by the parent generateSlide fn are not
  // valid, set them now.
  const vtkVgTimeStamp start = this->Track->GetStartFrame();
  const vtkVgTimeStamp end = this->Track->GetEndFrame();
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
    // Compute timestamp halfway through the track
    this->RequestedStillFrame.SetTime(0.5 * (start.GetTime() + end.GetTime()));
    if (start.HasFrameNumber() && end.HasFrameNumber())
      {
      const unsigned int frame =
        (start.GetFrameNumber() + end.GetFrameNumber()) / 2;
      this->RequestedStillFrame.SetFrameNumber(frame);
      }

    // Set timestamp to the "closest" non-interpolated frame of the track,
    // first looking before and then after, but only if necessary.
    if (!this->Track->GetFrameAtOrBefore(this->RequestedStillFrame) &&
        !this->Track->GetFrameAtOrAfter(this->RequestedStillFrame))
      {
      qDebug() << "Failed to get track midpoint region.";
      return true;
      }
    }

  return vvPowerPointSlideGenerator::generateSlide(
           writer, videoRequestor, outputId, generateVideo,
           progress, progressCount);
}

//-----------------------------------------------------------------------------
int vvPowerPointTrackSlideGenerator::outputFrameCount(bool generateVideo)
{
  if (!generateVideo)
    {
    return 1;
    }

  vtkVgTimeStamp start =
    this->VideoStartFrame.IsValid() ? this->VideoStartFrame
                                    : this->Track->GetStartFrame();
  vtkVgTimeStamp end =
    this->VideoEndFrame.IsValid() ? this->VideoEndFrame
                                  : this->Track->GetEndFrame();
  return this->numberOfOutputVideoFrames(start, end) + 1;
}

//-----------------------------------------------------------------------------
void vvPowerPointTrackSlideGenerator::writeSummary(vvPowerPointWriter& writer,
                                                   const ImageInfo& imageInfo,
                                                   int outputId)
{
  const int imageId =
    writer.addImage("track-image", imageInfo.Data, outputId, false);

  writer.addDateTime("track-start-time",
                     this->Track->GetStartFrame().GetRawTimeStamp(),
                     "DATE OF IMAGE: ", "TIME OF IMAGE: ");

  const QString note = QString::fromLocal8Bit(this->Track->GetNote());
  if (note.isEmpty())
    {
    writer.addText("track-comment", "COMMENT: TYPE COMMENTS OR DELETE");
    }
  else
    {
    const QString comment = "COMMENT: " + note;
    writer.addText("track-comment", qPrintable(comment));
    }

  this->addTrack(writer, this->Track, this->Track->GetStartFrame(),
                 this->Track->GetEndFrame(), imageInfo.Transform,
                 "track-path", imageId);
}

//-----------------------------------------------------------------------------
void vvPowerPointTrackSlideGenerator::writeVideoFrame(
  vvPowerPointWriter& writer, const ImageInfo& imageInfo,
  int outputId, int frameNumber)
{
  // Add model and representation
  vtkNew<vtkVgTrackModel> trackModel;
  vtkNew<vtkVgTrackRepresentation> trackRep;

  trackModel->SetPoints(this->Track->GetPoints());
  trackModel->AddTrack(this->Track);
  trackRep->SetTrackModel(trackModel.GetPointer());
  trackRep->SetZOffset(0.1);

  if (imageInfo.Transform)
    {
    trackRep->SetRepresentationMatrix(imageInfo.Transform);
    }

  trackModel->Update(imageInfo.TimeStamp);
  trackRep->Update();

  writer.writeVideoImageWithRep(imageInfo.Data,
                                trackRep->GetActiveRenderObjects(),
                                outputId, frameNumber);
}

//-----------------------------------------------------------------------------
void vvPowerPointTrackSlideGenerator::createVideo(vvPowerPointWriter& writer,
                                                  int outputId)
{
  QString videoName = writer.createVideo(outputId);
  if (!videoName.isEmpty())
    {
    writer.addVideo("track-video", qPrintable(videoName), false);
    }
}

//-----------------------------------------------------------------------------
int vvPowerPointTrackSlideGenerator::addTrack(
  vvPowerPointWriter& writer, vtkVgTrack* track,
  vtkVgTimeStamp startFrame, vtkVgTimeStamp endFrame,
  const vtkMatrix4x4* transform, const char* itemTemplateName, int imageId)
{
  QVector<float> trackPoints;
  if (track)
    {
    vtkPoints* points = track->GetPoints();

    track->InitPathTraversal();
    vtkIdType id;
    vtkVgTimeStamp timeStamp;
    while ((id = track->GetNextPathPt(timeStamp)) != -1)
      {
      if (startFrame.IsValid() && startFrame > timeStamp)
        {
        continue;
        }
      if (endFrame.IsValid() && timeStamp > endFrame)
        {
        break;
        }

      double point[3];
      points->GetPoint(id, point);
      if (transform)
        {
        vtkVgApplyHomography(point, transform, point);
        }

      trackPoints.append(point[0]);
      trackPoints.append(point[1]);
      }
    }

  return writer.addPolyLine(itemTemplateName, imageId, trackPoints);
}
