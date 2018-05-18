/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfSimpleCsvTrackArchiveSource.h"

#include <vdfTrackSource.h>

#include <qtEnumerate.h>
#include <qtKstReader.h>
#include <qtStlUtil.h>

#include <QDebug>
#include <QFile>
#include <QHash>
#include <QSharedPointer>

//-----------------------------------------------------------------------------
class vdfSimpleCsvTrackDataSourcePrivate
{
public:
  vdfSimpleCsvTrackDataSourcePrivate(
    vdfSimpleCsvTrackDataSource* q, const QSharedPointer<QFile>& file);

  const QSharedPointer<QFile> File;
  vdfTrackSource* const TrackSourceInterface;
};

QTE_IMPLEMENT_D_FUNC(vdfSimpleCsvTrackDataSource)

//-----------------------------------------------------------------------------
vdfSimpleCsvTrackDataSourcePrivate::vdfSimpleCsvTrackDataSourcePrivate(
  vdfSimpleCsvTrackDataSource* q, const QSharedPointer<QFile>& file) :
  File{file}, TrackSourceInterface{q->addInterface<vdfTrackSource>()}
{
}

//-----------------------------------------------------------------------------
vdfSimpleCsvTrackDataSource::vdfSimpleCsvTrackDataSource(
  const QUrl& uri, const QSharedPointer<QFile>& file, QObject* parent) :
  vdfThreadedArchiveSource{uri, parent},
  d_ptr{new vdfSimpleCsvTrackDataSourcePrivate{this, file}}
{
}

//-----------------------------------------------------------------------------
vdfSimpleCsvTrackDataSource::~vdfSimpleCsvTrackDataSource()
{
}

//-----------------------------------------------------------------------------
bool vdfSimpleCsvTrackDataSource::processArchive(const QUrl&)
{
  QTE_D();

  QHash<qint64, QList<vvTrackState>> tracks;
  QHash<qint64, vvTrackObjectClassification> trackTocs;

  while (!d->File->atEnd())
    {
    const auto line = d->File->readLine().replace("\n", ";\n");
    if (line.trimmed().startsWith('#'))
      {
      continue;
      }

    qtKstReader reader{QString::fromUtf8(line)};

    // Read track state
    qint64 trackId, frameId;
    int x0, x1, y0, y1;

    if (!(reader.readLong(trackId, 0) &&
          reader.readLong(frameId, 2) &&
          reader.readInt(x0, 3) &&
          reader.readInt(y0, 4) &&
          reader.readInt(x1, 5) &&
          reader.readInt(y1, 6)))
      {
      qDebug() << "Failed to read record" << line;
      continue;
      }

    // Update track trajectory
    vvTrackState state;
    state.TimeStamp.FrameNumber = static_cast<unsigned int>(frameId);
    state.ImageBox = {{x0, y0}, {x1, y1}};
    state.ImagePoint = {0.5 * static_cast<double>(x0 + x1),
                        0.5 * static_cast<double>(y0 + y1)};

    tracks[trackId].append(state);

    // Read track classification (if present)
    if (reader.seek(0, 9))
      {
      vvTrackObjectClassification toc;
      QString typeName;
      double typeConfidence;

      while (reader.readString(typeName) && reader.nextValue() &&
            reader.readReal(typeConfidence) && reader.nextValue())
        {
        toc.emplace(stdString(typeName), typeConfidence);
        }

      if (!toc.empty())
        {
        trackTocs.insert(trackId, toc);
        }
      }
    }

  // Emit track states for all tracks
  foreach (const auto& ti, qtEnumerate(tracks))
    {
    const auto id = vdfTrackId{this, ti.key()};
    const auto toc = trackTocs.value(ti.key());

    emit d->TrackSourceInterface->trackUpdated(id, ti.value());

    if (!toc.empty())
      {
      emit d->TrackSourceInterface->trackClassificationAvailable(id, toc);
      }

    emit d->TrackSourceInterface->trackClosed(id);
    }

  return true;
}
