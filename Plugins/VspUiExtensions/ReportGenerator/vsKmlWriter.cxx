/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsKmlWriter.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QMessageBox>

#include <vtkIdList.h>
#include <vtkPoints.h>

#include <vtkVgEvent.h>
#include <vtkVgTrack.h>
#include <vtkVgUtil.h>

#include <vvKmlLine.h>
#include <vvKmlWriter.h>

#include <vgVideoRequestor.h>

#include <vsVideoSource.h>

#include "vsEventUserInfo.h"

QTE_IMPLEMENT_D_FUNC(vsKmlWriter)

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
class vsKmlWriterPrivate
{
public:
  vsKmlWriterPrivate(vsVideoSource* source) : VideoRequestor(source) {}

  void addTrack(vtkIdType eventId, const vtkVgTimeStamp& startTime,
                const vtkVgTimeStamp& endTime, vtkVgTrack* track,
                const QString& note, vtkMatrix4x4* refToImage,
                vtkMatrix4x4* imageToLatLon);


  QString OutputPath;
  QList<vsEventUserInfo> Events;
  vtkVgEventTypeRegistry* Registry;

  vgVideoRequestor VideoRequestor;
  vtkVgVideoFrame Frame;

  vvKmlWriter KmlWriter;

  // Do we need to process based on input parameters set?
  bool Process;
};

//-----------------------------------------------------------------------------
void vsKmlWriterPrivate::addTrack(
  vtkIdType eventId, const vtkVgTimeStamp& startTime,
  const vtkVgTimeStamp& endTime, vtkVgTrack* track,
  const QString& note, vtkMatrix4x4* refToImage,
  vtkMatrix4x4* imageToLatLon)
{
  if (!track)
    {
    qDebug() << "Invalid track. Unable to add track to KML.";
    return;
    }
  if (!refToImage)
    {
    qDebug() << "Invalid ref to image matrix. Unable to add track to KML.";
    return;
    }
  if (!imageToLatLon)
    {
    qDebug() << "Invalid image to latlon matrix. Unable to add track to KML";
    return;
    }

  vtkPoints* points = track->GetPoints();

  if (!points)
    {
    qDebug() << "Invalid track data. Unable to add track to KML";
    return;
    }

  double trackColor[3];
  track->GetColor(trackColor);

  vvKmlLine* line = this->KmlWriter.createLine();
  line->setId(QString("Event_%1").arg(eventId));
  line->setDescription(note);
  line->setColor(trackColor[0], trackColor[1], trackColor[2], 1.0);

  vtkSmartPointer<vtkMatrix4x4> refToLatLon
    = vtkSmartPointer<vtkMatrix4x4>::New();

  vtkMatrix4x4::Multiply4x4(imageToLatLon, refToImage, refToLatLon);

  vtkVgTimeStamp timeStamp;
  track->InitPathTraversal();
  vtkIdType id;
  double lat, lon;

  while ((id = track->GetNextPathPt(timeStamp)) != -1)
    {
    if (startTime.IsValid() && startTime > timeStamp)
      {
      continue;
      }
    if (endTime.IsValid() && timeStamp > endTime)
      {
      break;
      }

    double point[3];
    points->GetPoint(id, point);
    vtkVgApplyHomography(point, refToLatLon, lon, lat);

    // Ignoring altitude for now
    line->addPoint(lat, lon, 0.0);
    }
}

//-----------------------------------------------------------------------------
vsKmlWriter::vsKmlWriter() : d_ptr(new vsKmlWriterPrivate(0))
{
  QTE_D(vsKmlWriter);
  d->Process = false;
}

//-----------------------------------------------------------------------------
vsKmlWriter::vsKmlWriter(QList<vsEventUserInfo> events,
                         vtkVgEventTypeRegistry* registry,
                         vsVideoSource* videoSource)
  : d_ptr(new vsKmlWriterPrivate(videoSource))
{
  Q_ASSERT(videoSource);

  QTE_D(vsKmlWriter);
  d->Events = events;
  d->Registry = registry;
  d->Process = true;
}

//-----------------------------------------------------------------------------
vsKmlWriter::~vsKmlWriter()
{
}

//-----------------------------------------------------------------------------
void vsKmlWriter::setOutputPath(const QString& path)
{
  QTE_D(vsKmlWriter);

  d->OutputPath = path;
}

//-----------------------------------------------------------------------------
void vsKmlWriter::addTrack(
  vtkIdType eventId, const vtkVgTimeStamp& startTime,
  const vtkVgTimeStamp& endTime, vtkVgTrack* track, const QString& note,
  vtkMatrix4x4* refToImage, vtkMatrix4x4* imageToLatLon)
{
  QTE_D(vsKmlWriter);

  d->addTrack(eventId, startTime, endTime, track, note, refToImage,
              imageToLatLon);
}

//-----------------------------------------------------------------------------
static bool eventEarlierThan(const vsEventUserInfo& a,
                             const vsEventUserInfo& b)
{
  return a.Event->GetStartFrame() < b.Event->GetStartFrame();
}

//-----------------------------------------------------------------------------
void vsKmlWriter::process()
{
  QTE_D(vsKmlWriter);

  // Sort by event start time
  qSort(d->Events.begin(), d->Events.end(), eventEarlierThan);

  vtkVgInstance<vtkMatrix4x4> refToImage;

  foreach (vsEventUserInfo info, d->Events)
    {
    if (!info.Event->IsStarred())
      {
      continue;
      }

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
    vtkVgTimeStamp prev = mid;
    if (info.Event->GetNumberOfRegions() > 0 &&
        !info.Event->GetRegionAtOrAfter(mid, npts, ids) && prev == mid)
      {
      qDebug() << "Failed to get event midpoint region.";
      continue;
      }

    if (!d->VideoRequestor.requestFrame(d->Frame, mid, vg::SeekNearest))
      {
      qDebug() << "Frame request failed: KML frame discarded.";
      continue;
      }

    vtkImageData* image = d->Frame.Image.GetVolatilePointer();
    convertHomography(d->Frame.MetaData.Homography, refToImage,
                      image->GetDimensions()[1]);

    vtkSmartPointer<vtkMatrix4x4> imageToLatLon;
    if (d->Frame.MetaData.WorldLocation.GCS != -1)
      {
      imageToLatLon = d->Frame.MetaData.MakeImageToLatLonMatrix();
      }

    if (info.Event->GetNumberOfTracks() > 0)
      {
      vtkVgTrack* track;
      vtkVgTimeStamp startTime;
      vtkVgTimeStamp endTime;
      info.Event->GetTrack(0, track, startTime, endTime);
      const QString note = QString::fromLocal8Bit(info.Event->GetNote());
      d->addTrack(info.Event->GetId(), startTime, endTime,
                  info.Event->GetTrack(0), note, refToImage, imageToLatLon);
      }
    }
}

//-----------------------------------------------------------------------------
void vsKmlWriter::write()
{
  QTE_D(vsKmlWriter);

  Q_ASSERT(!d->OutputPath.isEmpty());

  QString filePath;
  QFileInfo fileInfo(d->OutputPath);
  if (fileInfo.isDir())
    {
    filePath = QString(fileInfo.canonicalFilePath() + '/' + "vsplay.kml");
    }
  else
    {
    filePath = fileInfo.absoluteFilePath();
    }

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly |
                 QIODevice::Text |
                 QIODevice::Truncate))
    {
    qDebug() << file.errorString();
    QMessageBox::warning(0, QString(), "Unable to write KML file.");
    return;
    }

  if (d->Process)
    {
    this->process();
    }

  d->KmlWriter.write(file);
}
