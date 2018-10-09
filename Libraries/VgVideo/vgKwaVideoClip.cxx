/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgKwaVideoClip.h"

#include "vgKwaFrameMetadata.h"
#include "vgKwaUtilPrivate.h"
#include "vgVideoFramePtrPrivate.h"
#include "vgVideoPrivate.h"

#include <vgCheckArg.h>

#include <vgDebug.h>

#include <vsl/vsl_binary_io.h>
#include <vsl/vsl_stream.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>

#include <limits>

#include <cmath>

#define die(...) do { \
  qDebug() << "vgKwaVideoClip:" << __VA_ARGS__; \
  return; \
  } while (0)

//-----------------------------------------------------------------------------
uint qHash(const vgTimeStamp& ts)
{
  return qHash(qRound64(ts.Time));
}

//-----------------------------------------------------------------------------
class vgKwaVideoClipPrivate : public vgVideoPrivate
{
public:
  QString missionId;
  QString streamId;
  QSharedPointer<vgIStream> store;
  QSharedPointer<vsl_b_istream> stream;
  QHash<vgTimeStamp, quint64> dataOffsets;
  vgKwaVideoClip::MetadataMap metadata;

  vgKwaFrameMetadata metadataAt(const vgTimeStamp& ts) const;
  double applyPadding(double time, double padding, int direction) const;
};

//-----------------------------------------------------------------------------
vgKwaFrameMetadata vgKwaVideoClipPrivate::metadataAt(
  const vgTimeStamp& ts) const
{
  // Use loaded metadata from .meta, if present
  if (this->metadata.contains(ts))
    {
    return this->metadata[ts];
    }

  // Try to read from .data
  if (this->dataOffsets.contains(ts))
    {
    CHECK_ARG(this->store->seek(0), vgKwaFrameMetadata());

    vsl_b_istream stream(*this->store);

    int dataVersion;
    vsl_b_read(stream, dataVersion);

    const int maxVersion = vgKwaFrameMetadata::SupportedDataVersion;
    if (dataVersion < 1 || dataVersion > maxVersion ||
        !this->store->seek(this->dataOffsets[ts]))
      {
      // Not working out...
      return vgKwaFrameMetadata();
      }

    return vgKwaFrameMetadata(stream, dataVersion, false);
    }

  // No luck
  return vgKwaFrameMetadata();
}

//-----------------------------------------------------------------------------
double vgKwaVideoClipPrivate::applyPadding(
  double time, double padding, int direction) const
{
  // Get initial position
  const vg::SeekMode seekDirection =
    (direction > 0 ? vg::SeekUpperBound : vg::SeekLowerBound);
  vgKwaVideoClip::MetadataMap::const_iterator iter =
    this->metadata.find(vgTimeStamp::fromTime(time), seekDirection);

  // If reference time isn't an exact match to a frame, add the difference to
  // the padding value and reset the reference time
  if (time != iter.key().Time)
    {
    padding += fabs(iter.key().Time - time);
    time = iter.key().Time;
    }

  // Get reference frame and boundary position
  uint hrf = iter->homographyReferenceFrameNumber();
  const vgKwaVideoClip::MetadataMap::const_iterator boundary =
    (direction > 0 ? this->metadata.constEnd() - 1 :
                     this->metadata.constBegin());

  // Start applying padding
  while (iter != boundary)
    {
    // Check next frame
    iter += direction;

    // If frame lies in another shot, we are done
    if (iter->homographyReferenceFrameNumber() != hrf)
      {
      break;
      }

    // Reduce padding; if we run out, we are done
    padding -= fabs(time - iter.key().Time);
    if (padding < 0.0)
      {
      break;
      }

    // Otherwise, accept up to current position
    time = iter.key().Time;
    }

  return time;
}

QTE_IMPLEMENT_D_FUNC(vgKwaVideoClip)

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgKwaVideoFramePtr

//-----------------------------------------------------------------------------
class vgKwaVideoFramePtr : public vgVideoFramePtrPrivate
{
public:
  explicit vgKwaVideoFramePtr(
    vgTimeStamp ts, const QSharedPointer<vsl_b_istream>& stream,
    const QSharedPointer<vgIStream>& store, quint64 offset, int version);

  vgImage data() const;

protected:
  QSharedPointer<vsl_b_istream> dataStream;
  QSharedPointer<vgIStream> dataStore;
  quint64 dataOffset;
  int dataVersion;
};

//-----------------------------------------------------------------------------
vgKwaVideoFramePtr::vgKwaVideoFramePtr(
  vgTimeStamp ts, const QSharedPointer<vsl_b_istream>& stream,
  const QSharedPointer<vgIStream>& store, quint64 offset, int version) :
  dataStream(stream),
  dataStore(store),
  dataOffset(offset),
  dataVersion(version)
{
  this->time = ts;
}

//-----------------------------------------------------------------------------
vgImage vgKwaVideoFramePtr::data() const
{
  const auto offset = static_cast<qint64>(this->dataOffset);
  const auto frameData =
    vgKwaUtil::readFrame(this->dataStore.data(), this->dataStream.data(),
                         this->dataVersion, offset, "vgKwaVideoFramePtr:",
                         this->time);

  // Sanity check timestamp
  vgTimeStamp dataTime, indexTime = this->time;
  indexTime.FrameNumber = vgTimeStamp::InvalidFrameNumber();
  dataTime.Time = frameData.first;
  if (dataTime != indexTime)
    {
    qDebug() << "vgKwaVideoFramePtr: warning: time" << dataTime
             << "in data does not match time" << this->time << "from index";
    }

  return frameData.second;
}

//END vgKwaVideoFramePtr

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgKwaVideoClip

//-----------------------------------------------------------------------------
vgKwaVideoClip::vgKwaVideoClip(const QUrl& uri) :
  vgVideo(new vgKwaVideoClipPrivate)
{
  QTE_D(vgKwaVideoClip);

  auto indexQuery = QUrlQuery{uri};
  bool okay;

  // Get temporal limits (if any) from URI
  qint64 startTime = std::numeric_limits<qint64>::min();
  qint64 endTime = std::numeric_limits<qint64>::max();
  if (indexQuery.hasQueryItem("StartTime"))
    {
    startTime = indexQuery.queryItemValue("StartTime").toLongLong(&okay);
    if (!okay)
      {
      qWarning() << "vgKwaVideoClip: Ignoring invalid start time"
                 << indexQuery.queryItemValue("StartTime");
      startTime = std::numeric_limits<qint64>::min();
      }
    indexQuery.removeAllQueryItems("StartTime");
    }
  if (indexQuery.hasQueryItem("EndTime"))
    {
    endTime = indexQuery.queryItemValue("EndTime").toLongLong(&okay);
    if (!okay)
      {
      qWarning() << "vgKwaVideoClip: Ignoring invalid end time"
                 << indexQuery.queryItemValue("EndTime");
      startTime = std::numeric_limits<qint64>::min();
      }
    indexQuery.removeAllQueryItems("EndTime");
    }

  auto indexUri = uri;
  indexUri.setQuery(indexQuery);

  // Open index file
  QString indexName = indexUri.toLocalFile();
  QFile indexFile(indexName);
  if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    die("Unable to open index" << indexUri.toString());
    }

  // Initialize index stream
  QTextStream indexStream(&indexFile);
  QString line = indexStream.readLine();
  long lineNumber = 1;

  // Read index version
  int indexVersion = line.toInt(&okay);
  if (!okay)
    {
    die("Bad index version" << line);
    }
  if (indexVersion < 1 || indexVersion > 4)
    {
    die("Don't know how to parse index version" << indexVersion);
    }

  // Read header
  QString dataName = indexStream.readLine();
  QString metaName;
  if (!vgKwaUtil::resolvePath(dataName, indexName, "data"))
    {
    return;
    }
  if (indexVersion > 1)
    {
    if (indexVersion > 2)
      {
      metaName = indexStream.readLine();
      if (!vgKwaUtil::resolvePath(metaName, indexName, "meta"))
        {
        return;
        }
      ++lineNumber;
      }

    // Read mission ID
    d->missionId = indexStream.readLine();
    ++lineNumber;

    if (indexVersion > 3)
      {
      // Read stream ID
      d->streamId = indexStream.readLine();
      ++lineNumber;
      }
    }
  ++lineNumber;

  // Open meta stream, if we have one
  QScopedPointer<vsl_b_istream> metaVblStream;
  vgIStream metaStream;
  int metaVersion = 0;
  if (!metaName.isEmpty())
    {
    if (!metaStream.open(metaName))
      {
      die("Unable to open metadata file" << metaName);
      }
    metaVblStream.reset(new vsl_b_istream(metaStream));

    // Read metadata version
    vsl_b_read(*metaVblStream, metaVersion);
    if (metaVersion < 1 || metaVersion > 2)
      {
      die("Don't know how to parse metadata version" << metaVersion);
      }
    }

  // Open the data file
  d->store = QSharedPointer<vgIStream>(new vgIStream);
  if (!d->store->open(dataName))
    {
    die("Unable to open data file" << metaName);
    }
  d->stream = QSharedPointer<vsl_b_istream>(new vsl_b_istream(*d->store));
  int dataVersion;
  vsl_b_read(*d->stream, dataVersion);

  // Process the index
  while (!indexStream.atEnd())
    {
    line = indexStream.readLine();
    ++lineNumber;

    // Read index line
    qint64 time;
    quint64 offset;

    QTextStream lineStream(&line);
    lineStream >> time >> offset;

    if (lineStream.status() != QTextStream::Ok)
      {
      die("Error parsing index file" << indexUri.toString()
          << "at line" << lineNumber);
      }

    // Create timestamp
    vgTimeStamp ts;
    ts.Time = time;

    // Read metadata for frame
    if (metaVersion > 0)
      {
      vgKwaFrameMetadata fmd(*metaVblStream, metaVersion, true);
      ts.FrameNumber = fmd.timestamp().FrameNumber;
      if (time >= startTime && time <= endTime)
        {
        // Only insert metadata if between start and end times
        d->metadata.insert(ts, fmd);
        }

      // Verify information from metadata matches index
      if (ts != fmd.timestamp())
        {
        die("Processing error: timestamp" << fmd.timestamp()
            << "in medata file" << metaName
            << "does not match timestamp" << ts
            << "in index file" << indexUri.toString()
            << "at line" << lineNumber);
        }
      }

    // Add offset to map...
    if (time >= startTime && time <= endTime)
      {
      // ...but only if between start and end times
      d->dataOffsets.insert(ts, offset);
      }
    }

  // Generate frame pointers
  typedef QHash<vgTimeStamp, quint64>::const_iterator Iterator;
  foreach_iter (Iterator, iter, d->dataOffsets)
    {
    vgKwaVideoFramePtr* frame =
      new vgKwaVideoFramePtr(iter.key(), d->stream, d->store, iter.value(),
                             dataVersion);
    d->frames.insert(iter.key(), vgVideoFramePtr(frame));
    }
}

//-----------------------------------------------------------------------------
vgKwaVideoClip::vgKwaVideoClip(const vgKwaVideoClip& other,
                               double startTime,
                               double endTime) :
  vgVideo(new vgKwaVideoClipPrivate)
{
  QTE_D(vgKwaVideoClip);
  const vgKwaVideoClipPrivate* od = other.d_func();

  d->missionId = od->missionId;
  d->streamId = od->streamId;
  d->stream = od->stream;
  d->store = od->store;

  // Copy metadata
  typedef vgKwaVideoClip::MetadataMap::const_iterator MetadataIterator;
  foreach_iter (MetadataIterator, mdIter, od->metadata)
    {
    // Get raw time
    const double time = mdIter.key().Time;
    if (time >= startTime && time <= endTime)
      {
      // Copy metadata, if in range
      d->metadata.insert(mdIter.key(), mdIter.value());
      }
    }

  // Copy frame pointers
  typedef vgVideo::FrameMap::const_iterator FrameIterator;
  foreach_iter (FrameIterator, fIter, od->frames)
    {
    // Get raw time
    const double time = fIter.key().Time;
    if (time >= startTime && time <= endTime)
      {
      // Copy frame, if in range
      d->frames.insert(fIter.key(), fIter.value());
      }
    }
}

//-----------------------------------------------------------------------------
vgKwaVideoClip::~vgKwaVideoClip()
{
}

//-----------------------------------------------------------------------------
QString vgKwaVideoClip::missionId() const
{
  QTE_D_CONST(vgKwaVideoClip);
  return d->missionId;
}

//-----------------------------------------------------------------------------
QString vgKwaVideoClip::streamId() const
{
  QTE_D_CONST(vgKwaVideoClip);
  return d->streamId;
}

//-----------------------------------------------------------------------------
vgKwaFrameMetadata vgKwaVideoClip::currentMetadata() const
{
  QTE_D_CONST(vgKwaVideoClip);
  if (d->pos == d->frames.end())
    {
    return vgKwaFrameMetadata();
    }
  return d->metadataAt(d->pos.key());
}

//-----------------------------------------------------------------------------
vgKwaVideoClip::MetadataMap vgKwaVideoClip::metadata() const
{
  QTE_D_CONST(vgKwaVideoClip);
  return d->metadata;
}

//-----------------------------------------------------------------------------
vgKwaFrameMetadata vgKwaVideoClip::metadataAt(
  vgTimeStamp pos, vg::SeekMode direction) const
{
  QTE_D_CONST(vgKwaVideoClip);
  FrameMap::const_iterator iter = this->iterAt(pos, direction);
  return d->metadataAt(iter.key());
}

//-----------------------------------------------------------------------------
vgKwaVideoClip* vgKwaVideoClip::subClip(
  double startTime, double endTime, double padding) const
{
  // Resolve padding request; this will also normalize the start and end time,
  // even if padding == 0
  if (!this->resolvePadding(startTime, endTime, padding))
    {
    // Padding resolution failed... this might happen if the requested limits
    // are bad (e.g. start > end), or don't overlap us at all
    return 0;
    }

  // Seems like we have something to do; return new clip constructed from us,
  // with the requested temporal limits
  return new vgKwaVideoClip(*this, startTime, endTime);
}

//-----------------------------------------------------------------------------
bool vgKwaVideoClip::resolvePadding(
  double& startTime, double& endTime, double padding) const
{
  QTE_D_CONST(vgKwaVideoClip);

  if (d->frames.isEmpty())
    {
    // If our frame range is empty, this is a futile exercise...
    return false;
    }

  CHECK_ARG(startTime <= endTime, false);

  // Fix up arguments
  if (startTime == -1 && endTime == -1)
    {
    startTime = std::numeric_limits<qint64>::min();
    endTime = std::numeric_limits<qint64>::max();
    }
  padding = qMax(padding, 0.0);

  // Check for some overlap
  const double myStartTime = d->frames.begin().key().Time;
  const double myEndTime = (d->frames.end() - 1).key().Time;
  CHECK_ARG(myStartTime < endTime && myEndTime > startTime, false);

  if (d->metadata.isEmpty() || padding == 0.0)
    {
    // If we have no metadata, or no padding was requested, just limit the
    // bounds to our actual time range
    startTime = qMax(startTime, myStartTime);
    endTime   = qMin(endTime,   myEndTime);
    }
  else
    {
    startTime = d->applyPadding(qMax(startTime, myStartTime), padding, -1);
    endTime   = d->applyPadding(qMin(endTime,   myEndTime),   padding, +1);
    }

  return true;
}

//END vgKwaVideoClip
