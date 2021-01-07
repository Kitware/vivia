// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsReportGenerator.h"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QMessageBox>
#include <QProgressDialog>

#include <vtkVgEvent.h>

#include <vvReportWriter.h>

#include <vgVideoRequestor.h>

#include <vsVideoSource.h>

#include "vsEventUserInfo.h"
#include "vsKmlWriter.h"

QTE_IMPLEMENT_D_FUNC(vsReportGenerator)

//-----------------------------------------------------------------------------
static void convertHomography(
  vtkMatrix4x4* in, vtkMatrix4x4* out, int height)
{
  vtkVgInstance<vtkMatrix4x4> hi;
  vtkVgInstance<vtkMatrix4x4> fy;
  hi->DeepCopy(in);
  hi->Invert();
  fy->Identity();
  fy->SetElement(1, 1, -1);
  fy->SetElement(1, 3, height - 1);
  vtkMatrix4x4::Multiply4x4(fy, hi, out);
}

//-----------------------------------------------------------------------------
class vsReportGeneratorPrivate
{
public:
  vsReportGeneratorPrivate(vsVideoSource* source)
    : VideoRequestor(source) {}

  QString OutputPath;
  QList<vsEventUserInfo> Events;
  vtkVgEventTypeRegistry* Registry;

  vgVideoRequestor VideoRequestor;
  vtkVgVideoFrame Frame;
};

//-----------------------------------------------------------------------------
vsReportGenerator::vsReportGenerator(QList<vsEventUserInfo> events,
                                     vtkVgEventTypeRegistry* registry,
                                     vsVideoSource* videoSource)
  : d_ptr(new vsReportGeneratorPrivate(videoSource))
{
  Q_ASSERT(videoSource);

  QTE_D(vsReportGenerator);
  d->Events = events;
  d->Registry = registry;
}

//-----------------------------------------------------------------------------
vsReportGenerator::~vsReportGenerator()
{
}

//-----------------------------------------------------------------------------
void vsReportGenerator::setOutputPath(const QString& path)
{
  QTE_D(vsReportGenerator);

  d->OutputPath = path;
}

//-----------------------------------------------------------------------------
static bool eventEarlierThan(const vsEventUserInfo& a,
                             const vsEventUserInfo& b)
{
  return a.Event->GetStartFrame() < b.Event->GetStartFrame();
}

//-----------------------------------------------------------------------------
void vsReportGenerator::generateReport(bool generateVideo)
{
  QTE_D(vsReportGenerator);

  Q_ASSERT(!d->OutputPath.isEmpty());

  QFile file(d->OutputPath + '/' + "report.xml");
  if (!file.open(QIODevice::WriteOnly |
                 QIODevice::Text |
                 QIODevice::Truncate))
    {
    qDebug() << file.errorString();
    QMessageBox::warning(0, QString(), "Unable to write report file.");
    return;
    }

  vvReportWriter writer(file);

  vsKmlWriter kmlWriter;
  kmlWriter.setOutputPath(d->OutputPath);

  // Sort by event start time
  qSort(d->Events.begin(), d->Events.end(), eventEarlierThan);

  // Count the total number of work items
  int total = 0;
  foreach (vsEventUserInfo info, d->Events)
    {
    if (info.Event->IsStarred())
      {
      if (!generateVideo)
        {
        ++total;
        }
      else if (info.Event->GetStartFrame().HasFrameNumber() &&
               info.Event->GetEndFrame().HasFrameNumber())
        {
        total += info.Event->GetEndFrame().GetFrameNumber() -
                 info.Event->GetStartFrame().GetFrameNumber() + 1;
        }
      else
        {
        total += 100;
        }
      }
    }

  // Pop up progress bar
  QProgressDialog progress("Generating report...", "Cancel", 0, total);
  progress.setWindowModality(Qt::ApplicationModal);
  progress.setMinimumDuration(0);
  progress.setAutoClose(false);
  progress.setAutoReset(false);
  progress.setValue(0);

  vtkVgInstance<vtkMatrix4x4> transform;

  int count = 0;
  int currId = 1;
  foreach (vsEventUserInfo info, d->Events)
    {
    if (!info.Event->IsStarred())
      {
      continue;
      }

    const QString note = QString::fromLocal8Bit(info.Event->GetNote());
    writer.setEvent(info.Event, currId++, note, d->Registry);

    vtkVgTimeStamp start = info.Event->GetStartFrame();
    vtkVgTimeStamp end = info.Event->GetEndFrame();

    // Compute timestamp halfway through the event
    vtkVgTimeStamp mid(0.5 * (start.GetTime() + end.GetTime()));
    if (start.HasFrameNumber() && end.HasFrameNumber())
      {
      const unsigned int frame =
        (start.GetFrameNumber() + end.GetFrameNumber()) / 2;
      mid.SetFrameNumber(frame);
      }

    // Set timestamp to the next closest frame with a valid region; if there
    // are no regions, just use the already computed timestamp
    vtkIdType npts;
    vtkIdType* ids;
    if (info.Event->GetNumberOfRegions() > 0 &&
        !info.Event->GetClosestDisplayRegion(mid, npts, ids) && npts == 0)
      {
      qDebug() << "Failed to get event midpoint region.";
      continue;
      }

    if (!d->VideoRequestor.requestFrame(d->Frame, mid, vg::SeekNearest))
      {
      qDebug() << "Frame request failed: report frame discarded.";
      continue;
      }

    vtkImageData* image = d->Frame.Image.GetVolatilePointer();
    convertHomography(d->Frame.MetaData.Homography, transform,
                      image->GetDimensions()[1]);

    vtkSmartPointer<vtkMatrix4x4> imageToLatLon;
    if (d->Frame.MetaData.WorldLocation.GCS != -1)
      {
      imageToLatLon = d->Frame.MetaData.MakeImageToLatLonMatrix();
      }

    if (info.Event->GetNumberOfTracks() > 0)
      {
      vtkVgTrack* track;
      vtkVgTimeStamp trackStart;
      vtkVgTimeStamp trackEnd;
      info.Event->GetTrack(0, track, trackStart, trackEnd);
      const QString note = QString::fromLocal8Bit(info.Event->GetNote());
      kmlWriter.addTrack(info.Event->GetId(), trackStart, trackEnd, track,
                         note, transform, imageToLatLon);
      writer.setTrack(track, transform, trackStart, trackEnd);
      }
    else
      {
      writer.setTrack(0);
      }

    writer.setImageData(image, d->Frame.MetaData.Time, transform,
                        imageToLatLon);
    writer.writeEventSummary();

    // Done with this event if we're not writing video images
    if (!generateVideo)
      {
      progress.setValue(++count);
      continue;
      }

    int framenum = 0;
    vtkVgTimeStamp seekTime = start;
    vg::SeekMode seekDirection = vg::SeekNearest;

    do
      {
      if (!d->VideoRequestor.requestFrame(d->Frame, seekTime, seekDirection))
        {
        // If request fails, we must have stepped outside the bounds of the
        // video?
        break;
        }

      vtkImageData* image = d->Frame.Image.GetVolatilePointer();
      convertHomography(d->Frame.MetaData.Homography, transform,
                        image->GetDimensions()[1]);

      writer.setImageData(image, d->Frame.MetaData.Time, transform);
      writer.writeEventVideoImage(++framenum);

      // Set up the seek to give the next frame
      seekTime = d->Frame.MetaData.Time;
      seekDirection = vg::SeekNext;

      if (progress.wasCanceled())
        {
        break;
        }

      progress.setValue(++count);

      } while (seekTime < end);

    writer.writeEventVideo();
    }

  // Assuming that KML write should be quick
  kmlWriter.write();
}
