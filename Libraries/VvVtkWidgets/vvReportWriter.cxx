/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvReportWriter.h"

#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>
#include <QVector>

#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkPNGWriter.h>
#include <vtkPNMWriter.h>
#include <vtkPoints.h>
#include <vtkPropCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

#include <vtkVgDataSourceBase.h>
#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgRendererUtils.h>
#include <vtkVgTerrain.h>
#include <vtkVgTrack.h>
#include <vtkVgUtil.h>

#include <vgUnixTime.h>

QTE_IMPLEMENT_D_FUNC(vvReportWriter)

enum
{
  ContextImageWidth = 1280,
  ContextImageHeight = 1024
};

struct GeoCoord
{
  double Latitude;
  double Longitude;
};

struct ImageCoord
{
  int X, Y;
};

struct EventOverviewInfo
{
  int EventId;
  int TrackPointsEnd;
  GeoCoord Location;
};

//-----------------------------------------------------------------------------
class vvReportWriterPrivate
{
public:
  vvReportWriterPrivate(QFile& file)
    : File(file), TextStream(&file), Document(),
      OverviewElement(Document.createElement("Overview")),
      EventsElement(Document.createElement("ReportedEvents")),
      PngWriter(vtkSmartPointer<vtkPNGWriter>::New()),
      PnmWriter(vtkSmartPointer<vtkPNMWriter>::New()),
      Event(0), EventTypeRegistry(0), Rank(-1), RelevancyScore(-1.0),
      Track(0), Context(0), ImageData(0), CurrentId(-1)
    {
    QDomElement root = Document.createElement("xml");
    root.appendChild(this->OverviewElement);
    root.appendChild(this->EventsElement);
    this->Document.appendChild(root);
    }

  ImageCoord latLonToContextImage(GeoCoord gc, double worldBounds[4])
    {
    Q_ASSERT(this->LatLonToWorld);
    Q_ASSERT(worldBounds);

    const vgPoint2d point =
      vtkVgApplyHomography(gc.Longitude, gc.Latitude, this->LatLonToWorld);

    double xnorm = (point.X - worldBounds[0]) /
                   (worldBounds[1] - worldBounds[0]);

    double ynorm = 1.0 - (point.Y - worldBounds[2]) /
                   (worldBounds[3] - worldBounds[2]);

    ImageCoord ic;
    ic.X = qRound(xnorm * (ContextImageWidth - 1));
    ic.Y = qRound(ynorm * (ContextImageHeight - 1));
    return ic;
    }

public:
  QFile& File;
  QTextStream TextStream;
  QDomDocument Document;

  QDomElement OverviewElement;
  QDomElement EventsElement;
  QDomElement CurrentEventElement;

  vtkSmartPointer<vtkPNGWriter> PngWriter;
  vtkSmartPointer<vtkPNMWriter> PnmWriter;

  vtkVgEvent* Event;
  vtkVgEventTypeRegistry* EventTypeRegistry;
  QString EventNote;
  QString MissionId;
  QString Source;
  int Rank;
  double RelevancyScore;

  vtkVgTrack* Track;
  vtkMatrix4x4* TrackModelToImage;
  vtkVgTimeStamp TrackStartFrame;
  vtkVgTimeStamp TrackEndFrame;

  vtkVgTerrain* Context;
  vtkVgDataSourceBase* ContextSource;
  vtkMatrix4x4* LatLonToWorld;

  vtkImageData* ImageData;
  vtkVgTimeStamp ImageTimeStamp;
  vtkMatrix4x4* ModelToImage;
  vtkMatrix4x4* ImageToLatLon;

  vtkBoundingBox ContextOverviewBounds;
  QVector<EventOverviewInfo> EventInfos;
  QVector<GeoCoord> TrackPoints;

  int CurrentId;

  QEventLoop EventLoop;
  QProcess EncodeProcess;
};

//-----------------------------------------------------------------------------
vvReportWriter::vvReportWriter(QFile& file)
  : d_ptr(new vvReportWriterPrivate(file))
{
  QTE_D(vvReportWriter);

  connect(&d->EncodeProcess, SIGNAL(error(QProcess::ProcessError)),
          this, SLOT(videoCreateError()));

  connect(&d->EncodeProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
          this, SLOT(videoCreateFinished()));
}

//-----------------------------------------------------------------------------
vvReportWriter::~vvReportWriter()
{
  QTE_D(vvReportWriter);

  d->TextStream << d->Document.toString();
}

//-----------------------------------------------------------------------------
void vvReportWriter::setEvent(vtkVgEvent* event,
                              int outputId,
                              const QString& eventNote,
                              vtkVgEventTypeRegistry* typeRegistry,
                              int rank,
                              double relevancyScore,
                              const QString& missionId,
                              const QString& source)
{
  QTE_D(vvReportWriter);

  d->Event = event;
  d->CurrentId = outputId;
  d->EventNote = eventNote;
  d->EventTypeRegistry = typeRegistry;
  d->Rank = rank;
  d->RelevancyScore = relevancyScore;
  d->MissionId = missionId;
  d->Source = source;
}

//-----------------------------------------------------------------------------
void vvReportWriter::setTrack(vtkVgTrack* track, vtkMatrix4x4* modelToImage,
                              const vtkVgTimeStamp& startFrame,
                              const vtkVgTimeStamp& endFrame)
{
  QTE_D(vvReportWriter);

  d->Track = track;
  d->TrackModelToImage = modelToImage;
  d->TrackStartFrame = startFrame;
  d->TrackEndFrame = endFrame;
}

//-----------------------------------------------------------------------------
void vvReportWriter::setContext(vtkVgTerrain* context,
                                vtkVgDataSourceBase* contextSource,
                                vtkMatrix4x4* latLonToWorld)
{
  QTE_D(vvReportWriter);

  d->Context = context;
  d->ContextSource = contextSource;
  d->LatLonToWorld = latLonToWorld;
}

//-----------------------------------------------------------------------------
void vvReportWriter::setImageData(vtkImageData* imageData,
                                  const vtkVgTimeStamp& imageTimeStamp,
                                  vtkMatrix4x4* modelToImage,
                                  vtkMatrix4x4* imageToLatLon)
{
  QTE_D(vvReportWriter);

  d->ImageData = imageData;
  d->ImageTimeStamp = imageTimeStamp;
  d->ModelToImage = modelToImage;
  d->ImageToLatLon = imageToLatLon;
}

//-----------------------------------------------------------------------------
void vvReportWriter::writeOverview()
{
  QTE_D(vvReportWriter);

  if (!d->Context || !d->LatLonToWorld || d->EventInfos.isEmpty())
    {
    return;
    }

  // Setup context overview rendering
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();

  // TODO: Make this resolution configurable
  renderWindow->SetSize(ContextImageWidth, ContextImageHeight);
  renderWindow->SetOffScreenRendering(1);
  renderWindow->AddRenderer(renderer);
  renderer->GetActiveCamera()->ParallelProjectionOn();

  // Add context actors
  vtkPropCollection* props = d->Context->GetActiveDrawables();
  props->InitTraversal();
  while (vtkProp* prop = props->GetNextProp())
    {
    renderer->AddViewProp(prop);
    prop->SetVisibility(1);
    }

  double bounds[6];
  if (d->ContextOverviewBounds.IsValid())
    {
    d->ContextOverviewBounds.GetBounds(bounds);
    }
  else
    {
    // If there were no events with valid corner points, just show the whole
    // context.
    d->Context->ComputeBounds();
    d->Context->GetBounds(bounds);
    }

  double viewBounds[4];
  vtkVgRendererUtils::ZoomToExtents2D(renderer, bounds);
  vtkVgRendererUtils::GetBounds(renderer, viewBounds);

  int extents[4] =
    {
    static_cast<int>(floor(viewBounds[0])),
    static_cast<int>(ceil(viewBounds[1])),
    static_cast<int>(floor(viewBounds[2])),
    static_cast<int>(ceil(viewBounds[3]))
    };

  d->ContextSource->SetVisibleExtents(extents);
  d->ContextSource->SetImageLevel(-1);
  d->ContextSource->Update();

  renderWindow->Render();

  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
    vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(renderWindow);
  windowToImageFilter->SetInputBufferTypeToRGBA();
  windowToImageFilter->Update();

  QString path = QFileInfo(d->File).path();

  const QString filename = "overview.png";
  const QString filePath = QString("%1/%2").arg(path, filename);

  d->PngWriter->SetFileName(qPrintable(filePath));
  d->PngWriter->SetInputConnection(windowToImageFilter->GetOutputPort());
  d->PngWriter->Write();

  // Create overview info
  QDomElement overviewElem = d->Document.createElement("OverviewImage");
  overviewElem.setAttribute("file", filename);
  overviewElem.setAttribute("width", ContextImageWidth);
  overviewElem.setAttribute("height", ContextImageHeight);

  // Translate event lat longs to overview image coordinates
  int trackPointsBegin = 0;
  foreach (EventOverviewInfo info, d->EventInfos)
    {
    QDomElement eventLocElem = d->Document.createElement("Event");
    eventLocElem.setAttribute("id", info.EventId);

    ImageCoord ic = d->latLonToContextImage(info.Location, viewBounds);

    eventLocElem.setAttribute("x", ic.X);
    eventLocElem.setAttribute("y", ic.Y);

    // Translate track points
    if (info.TrackPointsEnd != 0 && info.TrackPointsEnd > trackPointsBegin)
      {
      QDomElement trackPtsElem = d->Document.createElement("TrackPoints");
      for (int i = trackPointsBegin; i < info.TrackPointsEnd; ++i)
        {
        ImageCoord img = d->latLonToContextImage(d->TrackPoints[i],
                                                 viewBounds);
        QDomElement trackPtElem = d->Document.createElement("Point");
        trackPtElem.setAttribute("x", img.X);
        trackPtElem.setAttribute("y", img.Y);
        trackPtsElem.appendChild(trackPtElem);
        }
      trackPointsBegin = info.TrackPointsEnd;
      eventLocElem.appendChild(trackPtsElem);
      }

    overviewElem.appendChild(eventLocElem);
    }

  d->OverviewElement.appendChild(overviewElem);
}

//-----------------------------------------------------------------------------
void vvReportWriter::writeEventSummary()
{
  QTE_D(vvReportWriter);

  Q_ASSERT(d->Event);
  Q_ASSERT(d->ImageData);

  QString path = QFileInfo(d->File).path();

  QString filePath = "%1/%2.png";
  filePath = filePath.arg(path).arg(d->CurrentId);

  QString contextFilePath = "%1/%2.context.png";
  contextFilePath = contextFilePath.arg(path).arg(d->CurrentId);

  QString warpedFilePath = "%1/%2.warped.png";
  warpedFilePath = warpedFilePath.arg(path).arg(d->CurrentId);

  d->PngWriter->SetFileName(qPrintable(filePath));
  d->PngWriter->SetInputData(d->ImageData);
  d->PngWriter->Write();

  vtkSmartPointer<vtkMatrix4x4> imageToWorld;
  double viewBounds[4];

  // Render the context
  if (d->Context && d->ImageToLatLon && d->LatLonToWorld)
    {
    vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderWindow> renderWindow =
      vtkSmartPointer<vtkRenderWindow>::New();

    // TODO: Make this resolution configurable
    renderWindow->SetSize(ContextImageWidth, ContextImageHeight);
    renderWindow->SetOffScreenRendering(1);
    renderWindow->AddRenderer(renderer);

    // Add context actors
    vtkPropCollection* props = d->Context->GetActiveDrawables();
    props->InitTraversal();
    while (vtkProp* prop = props->GetNextProp())
      {
      renderer->AddViewProp(prop);
      prop->SetVisibility(1);
      }

    vtkSmartPointer<vtkImageActor> imageProp =
      vtkSmartPointer<vtkImageActor>::New();

    imageToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Multiply4x4(d->LatLonToWorld, d->ImageToLatLon,
                              imageToWorld);

    // Make sure homogeneous component is non-negative or it won't render
    if (imageToWorld->GetElement(3, 3) < 0.0)
      {
      for (int i = 0; i < 4; ++i)
        {
        for (int j = 0; j < 4; ++j)
          {
          imageToWorld->SetElement(i, j, -imageToWorld->GetElement(i, j));
          }
        }
      }

    // Offset image slightly in front of context
    imageToWorld->SetElement(2, 3, 0.1 * imageToWorld->GetElement(3, 3));

    // Add warped image actor
    imageProp->SetInputData(d->ImageData);
    imageProp->SetUserMatrix(imageToWorld);
    renderer->AddViewProp(imageProp);

    renderer->GetActiveCamera()->ParallelProjectionOn();

    double bounds[6];
    imageProp->GetBounds(bounds);
    double width = bounds[1] - bounds[0];
    double height = bounds[3] - bounds[2];

    // Expand the bounds of the overview
    d->ContextOverviewBounds.AddBounds(bounds);

    // Make warped image occupy roughly 1/3 of the screen width
    double extbounds[6];
    extbounds[0] = bounds[0] - width;
    extbounds[1] = bounds[1] + width;
    extbounds[2] = bounds[2] - height;
    extbounds[3] = bounds[3] + height;
    extbounds[4] = bounds[4];
    extbounds[5] = bounds[5];

    double cbounds[6];
    d->Context->ComputeBounds();
    d->Context->GetBounds(cbounds);

    // Try not to have any empty space outside of rendered actors
    double minx = std::min(bounds[0], cbounds[0]);
    if (extbounds[0] < minx)
      {
      double diff = minx - extbounds[0];
      extbounds[0] += diff;
      extbounds[1] += diff;
      }

    double maxy = std::max(bounds[3], cbounds[3]);
    if (extbounds[3] > maxy)
      {
      double diff = maxy - extbounds[3];
      extbounds[2] += diff;
      extbounds[3] += diff;
      }

    vtkVgRendererUtils::ZoomToExtents2D(renderer, extbounds);
    vtkVgRendererUtils::GetBounds(renderer, viewBounds);

    int extents[4] =
      {
      static_cast<int>(floor(viewBounds[0])),
      static_cast<int>(ceil(viewBounds[1])),
      static_cast<int>(floor(viewBounds[2])),
      static_cast<int>(ceil(viewBounds[3]))
      };

    // Force context to highest LOD
    d->ContextSource->SetVisibleExtents(extents);
    d->ContextSource->SetImageLevel(0);
    d->ContextSource->Update();

    imageProp->SetVisibility(0);

    // Render context by itself
    renderWindow->Render();

    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
      vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(renderWindow);
    windowToImageFilter->SetInputBufferTypeToRGBA();
    windowToImageFilter->Update();

    d->PngWriter->SetFileName(qPrintable(contextFilePath));
    d->PngWriter->SetInputConnection(windowToImageFilter->GetOutputPort());
    d->PngWriter->Write();

    // Hide context and show image
    props->InitTraversal();
    while (vtkProp* prop = props->GetNextProp())
      {
      prop->SetVisibility(0);
      }
    imageProp->SetVisibility(1);

    // Render warped image in same coordinates as context
    renderWindow->Render();
    windowToImageFilter->Modified();

    d->PngWriter->SetFileName(qPrintable(warpedFilePath));
    d->PngWriter->Write();
    }

  QDomElement eventElem = d->Document.createElement("Event");
  d->CurrentEventElement = eventElem;

  eventElem.setAttribute("id", d->CurrentId);

  QDomElement noteElem = d->Document.createElement("Note");
  noteElem.setAttribute("value", d->EventNote);
  eventElem.appendChild(noteElem);

  vgUnixTime startTime(d->Event->GetStartFrame().GetTime());
  QDomElement startTimeElem = d->Document.createElement("StartTimeStamp");
  startTimeElem.setAttribute("time",
                             startTime.dateString() + " " +
                             startTime.timeString());
  eventElem.appendChild(startTimeElem);

  vgUnixTime endTime(d->Event->GetEndFrame().GetTime());
  QDomElement endTimeElem = d->Document.createElement("EndTimeStamp");
  endTimeElem.setAttribute("time",
                           endTime.dateString() + " " +
                           endTime.timeString());
  eventElem.appendChild(endTimeElem);

  if (d->EventTypeRegistry)
    {
    QDomElement classifiersElem = d->Document.createElement("Classifiers");

    d->Event->InitClassifierTraversal();
    for (bool valid = d->Event->InitClassifierTraversal(); valid;
         valid = d->Event->NextClassifier())
      {
      int type = d->Event->GetClassifierType();
      double prob = d->Event->GetClassifierProbability();

      QDomElement classifierElem = d->Document.createElement("Classifier");
      QString name = d->EventTypeRegistry->GetTypeById(type).GetName();
      classifierElem.setAttribute("name", name);
      classifierElem.setAttribute("probability", prob);

      classifiersElem.appendChild(classifierElem);
      }
    eventElem.appendChild(classifiersElem);
    }

  if (d->RelevancyScore != -1.0)
    {
    QDomElement scoreElem = d->Document.createElement("RelevancyScore");
    scoreElem.setAttribute("value", d->RelevancyScore);
    eventElem.appendChild(scoreElem);
    }

  if (d->Rank != -1)
    {
    QDomElement rankElem = d->Document.createElement("Rank");
    rankElem.setAttribute("value", d->Rank);
    eventElem.appendChild(rankElem);
    }

  if (!d->MissionId.isNull())
    {
    QDomElement missionElem = d->Document.createElement("MissionId");
    missionElem.setAttribute("value", d->MissionId);
    eventElem.appendChild(missionElem);
    }

  if (!d->Source.isNull())
    {
    QDomElement srcElem = d->Document.createElement("VideoSource");
    srcElem.setAttribute("value", QFileInfo(d->Source).fileName());
    eventElem.appendChild(srcElem);
    }

  QDomElement imageElem = d->Document.createElement("SampleImage");
  imageElem.setAttribute("file", QFileInfo(filePath).fileName());
  imageElem.setAttribute("width", d->ImageData->GetDimensions()[0]);
  imageElem.setAttribute("height", d->ImageData->GetDimensions()[1]);

  QDomElement timeStampElem = d->Document.createElement("TimeStamp");

  vgUnixTime timestamp(d->ImageTimeStamp.GetTime());
  timeStampElem.setAttribute("time",
                             timestamp.dateString() + " " +
                             timestamp.timeString());

  if (d->ImageTimeStamp.HasFrameNumber())
    {
    timeStampElem.setAttribute("frame", d->ImageTimeStamp.GetFrameNumber());
    }

  imageElem.appendChild(timeStampElem);

  vtkBoundingBox bbox;
  if (d->Event->GetNumberOfRegions() > 0)
    {
    vtkIdType npts = 0;
    vtkIdType* ids;
    d->Event->GetRegion(d->ImageTimeStamp, npts, ids);
    if (npts <= 0)
      {
      qDebug() << "Failed to get event region.";
      return;
      }

    vtkPoints* regionPts = d->Event->GetRegionPoints();

    QDomElement regionElem = d->Document.createElement("EventRegion");

    // Compute region minimum and maximum corners in image space, and emit
    // individual region points.
    for (int i = 0; i < npts - 1; ++i)
      {
      double p[3];
      regionPts->GetPoint(ids[i], p);

      if (d->ModelToImage)
        {
        vtkVgApplyHomography(p, d->ModelToImage, p);
        }

      p[1] = d->ImageData->GetDimensions()[1] - p[1] - 1;
      bbox.AddPoint(p);

      QDomElement regionPtElem = d->Document.createElement("Point");
      regionPtElem.setAttribute("x", p[0]);
      regionPtElem.setAttribute("y", p[1]);
      regionElem.appendChild(regionPtElem);
      }

    double ul[] =
      {
      bbox.GetMinPoint()[0],
      bbox.GetMinPoint()[1],
      };

    double lr[] =
      {
      bbox.GetMaxPoint()[0],
      bbox.GetMaxPoint()[1],
      };

    // Round to nearest pixel center
    int iul[2] = { qRound(ul[0]), qRound(ul[1]) };
    int ilr[2] = { qRound(lr[0]), qRound(lr[1]) };

    // Event box
    regionElem.setAttribute("ulx", iul[0]);
    regionElem.setAttribute("uly", iul[1]);
    regionElem.setAttribute("lrx", ilr[0]);
    regionElem.setAttribute("lry", ilr[1]);
    imageElem.appendChild(regionElem);
    }
  else
    {
    int* dim = d->ImageData->GetDimensions();
    bbox.AddPoint(dim[0] / 2.0, dim[1] / 2.0, 0.0);
    }

  EventOverviewInfo info;
  info.EventId = -1;
  info.Location.Latitude = 0.0;
  info.Location.Longitude = 0.0;
  info.TrackPointsEnd = 0;

  // Get region center in latitude / longitude
  if (d->ImageToLatLon)
    {
    double geocenter[3];
    bbox.GetCenter(geocenter);
    vtkVgApplyHomography(geocenter, d->ImageToLatLon, geocenter);

    // Remember location for positioning on the overview
    info.EventId = d->CurrentId;
    info.Location.Latitude = geocenter[1];
    info.Location.Longitude = geocenter[0];

    QDomElement geoElem = d->Document.createElement("EventLocation");
    geoElem.setAttribute("latitude", QString::number(geocenter[1], 'f', 6));
    geoElem.setAttribute("longitude", QString::number(geocenter[0], 'f', 6));
    imageElem.appendChild(geoElem);
    }

  // Output image coordinates of all track points for this event
  if (d->Track)
    {
    QDomElement trackElem = d->Document.createElement("TrackPoints");

    vtkVgTimeStamp timeStamp;
    vtkPoints* points = d->Track->GetPoints();

    // Create track-model-to-lat-lon matrix for later use
    vtkSmartPointer<vtkMatrix4x4> trackToLatLon;
    if (d->ImageToLatLon && d->TrackModelToImage)
      {
      trackToLatLon = vtkSmartPointer<vtkMatrix4x4>::New();
      vtkMatrix4x4::Multiply4x4(d->ImageToLatLon,
                                d->TrackModelToImage,
                                trackToLatLon);
      }

    d->Track->InitPathTraversal();
    vtkIdType id;
    while ((id = d->Track->GetNextPathPt(timeStamp)) != -1)
      {
      if (d->TrackStartFrame.IsValid() && d->TrackStartFrame > timeStamp)
        {
        continue;
        }
      if (d->TrackEndFrame.IsValid() && timeStamp > d->TrackEndFrame)
        {
        break;
        }

      double point[4];
      points->GetPoint(id, point);
      point[3] = 1.0;

      vgPoint2d point2;
      if (d->TrackModelToImage)
        {
        point2 = vtkVgApplyHomography(point, d->TrackModelToImage);
        }
      else
        {
        point2.X = point[0];
        point2.Y = point[1];
        }

      point2.Y = d->ImageData->GetDimensions()[1] - point2.Y - 1;

      int ipoint[] = { qRound(point2.X), qRound(point2.Y) };

      QDomElement elem = d->Document.createElement("Point");
      elem.setAttribute("x", ipoint[0]);
      elem.setAttribute("y", ipoint[1]);
      trackElem.appendChild(elem);

      // Compute and save geocoordinates of track points
      if (trackToLatLon)
        {
        GeoCoord gc;
        vtkVgApplyHomography(point, trackToLatLon, gc.Longitude, gc.Latitude);
        d->TrackPoints.append(gc);
        }
      }

    info.EventId = d->CurrentId;
    info.TrackPointsEnd = d->TrackPoints.size();
    imageElem.appendChild(trackElem);
    }

  if (info.EventId != -1)
    {
    d->EventInfos.append(info);
    }
  eventElem.appendChild(imageElem);

  if (d->Context && imageToWorld)
    {
    QDomElement contextElem = d->Document.createElement("ContextData");
    QDomElement contextImgElem = d->Document.createElement("ContextImage");
    QDomElement contextOverlayElem =
      d->Document.createElement("ContextImageOverlay");

    contextImgElem.setAttribute("file",
                                QFileInfo(contextFilePath).fileName());
    contextImgElem.setAttribute("width", ContextImageWidth);
    contextImgElem.setAttribute("height", ContextImageHeight);

    contextOverlayElem.setAttribute("file",
                                    QFileInfo(warpedFilePath).fileName());
    contextOverlayElem.setAttribute("width", ContextImageWidth);
    contextOverlayElem.setAttribute("height", ContextImageHeight);

    contextElem.appendChild(contextImgElem);
    contextElem.appendChild(contextOverlayElem);

    // Get region center in context (world) coords
    double center[3];
    bbox.GetCenter(center);
    vtkVgApplyHomography(center, imageToWorld, center);

    // Convert to pixel coords
    double normalized[] =
      {
      (center[0] - viewBounds[0]) / (viewBounds[1] - viewBounds[0]),
      (center[1] - viewBounds[2]) / (viewBounds[3] - viewBounds[2])
      };

    int imgx = qRound(normalized[0] * (ContextImageWidth - 1));
    int imgy = qRound(normalized[1] * (ContextImageHeight - 1));

    imgx = qBound(0, imgx, ContextImageWidth - 1);
    imgy = qBound(0, imgy, ContextImageHeight - 1);
    imgy = ContextImageHeight - 1 - imgy;

    QDomElement imgPos = d->Document.createElement("EventCenter");
    imgPos.setAttribute("x", imgx);
    imgPos.setAttribute("y", imgy);
    contextElem.appendChild(imgPos);

    eventElem.appendChild(contextElem);
    }

  d->EventsElement.appendChild(eventElem);
}

//-----------------------------------------------------------------------------
void vvReportWriter::writeEventVideoImage(int number)
{
  QTE_D(vvReportWriter);

  Q_ASSERT(d->Event);
  Q_ASSERT(d->ImageData);

  int dim[] = { d->ImageData->GetDimensions()[0],
                d->ImageData->GetDimensions()[1]
              };

  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();

  renderWindow->SetSize(dim[0], dim[1]);
  renderWindow->SetOffScreenRendering(1);
  renderWindow->AddRenderer(renderer);
  renderer->GetActiveCamera()->ParallelProjectionOn();

  vtkSmartPointer<vtkImageActor> imageProp =
    vtkSmartPointer<vtkImageActor>::New();

  // Add image actor
  imageProp->SetInputData(d->ImageData);
  renderer->AddViewProp(imageProp);

  double bounds[6];
  imageProp->GetBounds(bounds);
  vtkVgRendererUtils::ZoomToExtents2D(renderer, bounds);

  // Add event representation
  vtkSmartPointer<vtkVgEventModel> eventModel =
    vtkSmartPointer<vtkVgEventModel>::New();

  vtkSmartPointer<vtkVgEventRegionRepresentation> eventRep =
    vtkSmartPointer<vtkVgEventRegionRepresentation>::New();

  vtkSmartPointer<vtkVgEventBase> eventCopy =
    vtkSmartPointer<vtkVgEventBase>::New();

  // Need to make a copy of the event so that it points to the right points
  eventCopy->SetRegionPoints(eventModel->GetSharedRegionPoints());
  eventCopy->DeepCopy(d->Event);

  eventModel->AddEvent(eventCopy);
  eventRep->SetEventModel(eventModel);
  eventRep->SetEventTypeRegistry(d->EventTypeRegistry);
  eventRep->SetRegionZOffset(0.1);

  if (d->ModelToImage)
    {
    eventRep->SetRepresentationMatrix(d->ModelToImage);
    }

  eventModel->Update(d->ImageTimeStamp);
  eventRep->Update();

  vtkPropCollection* props = eventRep->GetActiveRenderObjects();
  props->InitTraversal();
  while (vtkProp* prop = props->GetNextProp())
    {
    renderer->AddViewProp(prop);
    }

  // Render to image
  renderWindow->Render();

  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
    vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(renderWindow);
  windowToImageFilter->Update();

  QString path = "%1/%2";
  path = path.arg(QFileInfo(d->File).path()).arg(d->CurrentId);
  QDir dir(path);
  if (!dir.exists())
    {
    dir.mkpath(path);
    }

  // Write as PNM/PPM file as it's much faster than generating a PNG
  QString filepath = "%1/%2.pnm";
  filepath = filepath.arg(path)
                     .arg(number, 6, 10, QChar('0'));
  d->PnmWriter->SetFileName(qPrintable(filepath));
  d->PnmWriter->SetInputConnection(windowToImageFilter->GetOutputPort());
  d->PnmWriter->Write();
}

//-----------------------------------------------------------------------------
void vvReportWriter::writeEventVideo()
{
  QTE_D(vvReportWriter);

  QString imagePath = "%1/%2";
  imagePath = imagePath.arg(QFileInfo(d->File.fileName()).path())
                       .arg(d->CurrentId);

  QString filePath = imagePath + ".wmv";

  QStringList args;
  args << "-y"; // overwrite existing
  args << "-r";
  args << "10"; // assuming 10 fps
  args << "-i";
  args << QString("%1/%06d.pnm").arg(imagePath);
  args << "-b";
  args << "5M";
  args << filePath;

  qDebug() << "starting ffmpeg";
  d->EncodeProcess.start("ffmpeg", args);

  if (!d->EncodeProcess.waitForStarted())
    {
    qDebug() << "ffmpeg failed to start:" << d->EncodeProcess.errorString();
    return;
    }

  if (d->EventLoop.exec() != 0)
    {
    qDebug() << d->EncodeProcess.errorString();
    }
  else
    {
    int exitCode = d->EncodeProcess.exitCode();
    if (exitCode != EXIT_SUCCESS)
      {
      qDebug() << d->EncodeProcess.readAllStandardError();
      }
    else
      {
      QDomElement vidElem = d->Document.createElement("Video");
      vidElem.setAttribute("file", QFileInfo(filePath).fileName());
      d->CurrentEventElement.appendChild(vidElem);
      }
    qDebug() << "ffmpeg returned with exit code:" << exitCode;
    }
}

//-----------------------------------------------------------------------------
void vvReportWriter::videoCreateError()
{
  QTE_D(vvReportWriter);

  d->EventLoop.exit(1);
}

//-----------------------------------------------------------------------------
void vvReportWriter::videoCreateFinished()
{
  QTE_D(vvReportWriter);

  d->EventLoop.exit(0);
}
