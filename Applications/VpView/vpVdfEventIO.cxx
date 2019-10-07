/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVdfEventIO.h"

#include "vpFileUtil.h"

#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgTrackModel.h>

#include <vdfDataSource.h>
#include <vdfSourceService.h>
#include <vdfEventReader.h>

#include <vgCheckArg.h>

#include <qtEnumerate.h>
#include <qtStlUtil.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

QTE_IMPLEMENT_D_FUNC(vpVdfEventIO)

//-----------------------------------------------------------------------------
class vpVdfEventIOPrivate
{
public:
  QHash<long long, vtkIdType>& EventSourceIdToModelIdMap;
  const QHash<long long, vtkIdType>& TrackSourceIdToModelIdMap;

  QUrl EventsUri;
};

//-----------------------------------------------------------------------------
vpVdfEventIO::vpVdfEventIO(
  QHash<long long, vtkIdType>& eventSourceIdToModelIdMap,
  const QHash<long long, vtkIdType>& trackSourceIdToModelIdMap,
  vtkVgEventModel* eventModel, vtkVgEventTypeRegistry* eventTypes)
  : vpEventIO{eventModel, eventTypes},
    d_ptr{new vpVdfEventIOPrivate{eventSourceIdToModelIdMap,
                                  trackSourceIdToModelIdMap, {}}}
{
}

//-----------------------------------------------------------------------------
vpVdfEventIO::~vpVdfEventIO()
{
}

//-----------------------------------------------------------------------------
void vpVdfEventIO::SetEventsUri(const QUrl& uri)
{
  QTE_D();
  d->EventsUri = uri;
}

//-----------------------------------------------------------------------------
bool vpVdfEventIO::ReadEvents()
{
  QTE_D();

  vdfEventReader reader;

  auto* const trackModel = this->EventModel->GetTrackModel();
  CHECK_ARG(trackModel, false);

  if (d->EventsUri.isLocalFile())
  {
    // TODO resolve relative globs
    // const auto& globPath = d->EventsUri.toLocalFile();
    // const auto& files = vpGlobFiles(QDir::current(), pattern);

    QFileInfo fi{d->EventsUri.toLocalFile()};
    const auto& dir = fi.absoluteDir();
    const auto& pattern = fi.fileName();

    const auto& files = vpGlobFiles(dir, pattern);
    if (files.isEmpty())
    {
      return false;
    }

    // Read through each event file
    for (const auto& filePath : files)
    {
      // Construct the event source and event reader
      const auto& eventUri = QUrl::fromLocalFile(filePath);
      QScopedPointer<vdfDataSource> source{
        vdfSourceService::createArchiveSource(eventUri)};

      if (source && reader.setSource(source.data()))
      {
        // Read events
        if (!reader.exec() && reader.failed())
        {
          // Event reading failed; die
          return false;
        }
      }
    }
  }
  else
  {
    // Construct the event source and event reader
    QScopedPointer<vdfDataSource> source{
      vdfSourceService::createArchiveSource(d->EventsUri)};

    if (source)
    {
      reader.setSource(source.data());

      // Read events
      if (!reader.exec() && reader.failed())
      {
        // Event reading failed; die
        return false;
      }
    }
  }

  if (!reader.hasData())
  {
    // No data was obtained; die
    return false;
  }

  const auto& inEvents = reader.events();
  for (const auto& in : inEvents)
  {
    auto event = vtkSmartPointer<vtkVgEvent>::New();

    // Get (desired) event model identifier
    const auto vtkId = static_cast<vtkIdType>(in.Id);

    // Set (actual) track model identifier
    if (this->EventModel->GetEvent(vtkId))
    {
      const auto fallbackId = this->EventModel->GetNextAvailableId();
      qInfo() << "Event id" << vtkId
              << "is not unique: changing id of imported event to"
              << fallbackId;
      event->SetId(fallbackId);
      d->EventSourceIdToModelIdMap.insert(in.Id, fallbackId);
    }
    else
    {
      event->SetId(vtkId);
    }

    // Set event start/stop
    event->SetStartFrame(in.Start);
    event->SetEndFrame(in.Stop);

    // Set event classifiers
    for (const auto& c : in.Classification)
    {
      event->AddClassifier(this->GetEventType(c.first.c_str()), c.second);
    }

    // Set event tracks
    for (const auto& ti : in.TrackIntervals)
    {
      const auto tsn = ti.Track.SerialNumber;
      const auto trackId =
        d->TrackSourceIdToModelIdMap.value(tsn, static_cast<vtkIdType>(tsn));
      auto* const track = trackModel->GetTrack(trackId);
      if (track)
      {
        event->AddTrack(track, ti.Start, ti.Stop);
      }
      else
      {
        qWarning() << "Track" << ti.Track << "for event" << vtkId
                   << "was not found!";
      }
    }

    this->EventModel->AddEvent(event);
  }

  return true;
}
