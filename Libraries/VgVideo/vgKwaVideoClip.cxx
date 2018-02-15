/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgKwaVideoClip.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>

#include <vgCheckArg.h>

#include <vgDebug.h>

#include <vcl_vector.h>
#include <vil/file_formats/vil_jpeg.h>
#include <vil/vil_stream_core.h>

#include <vil/io/vil_io_image_view.h>
#include <vsl/vsl_binary_io.h>
#include <vsl/vsl_stream.h>
#include <vsl/vsl_vector_io.h>
#include <vsl/vsl_vector_io.hxx>

#include <limits>

#include "vgIStream.h"
#include "vgKwaFrameMetadata.h"
#include "vgKwaUtil.h"
#include "vgVideoFramePtrPrivate.h"
#include "vgVideoPrivate.h"

VSL_VECTOR_IO_INSTANTIATE(char);

#define die(...) do { \
  qDebug() << "vgKwaVideoClip:" << __VA_ARGS__; \
  return; \
  } while (0)

//-----------------------------------------------------------------------------
uint qHash(const vgTimeStamp& ts)
{
  return qHash(qRound64(ts.Time));
}

namespace // anonymous
{

//-----------------------------------------------------------------------------
void vilCleanup(void* data)
{
  // Destroy our memory chunk pointer, which will cleanly release our reference
  // to the memory chunk
  vil_memory_chunk_sptr* chunk =
    reinterpret_cast<vil_memory_chunk_sptr*>(data);
  delete chunk;
}

} // namespace <anonymous>

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
  // Seek stream to appropriate location
  if (!this->dataStore->seek(this->dataOffset))
    {
    qDebug() << "vgKwaVideoFramePtr: failed to seek to position"
             << this->dataOffset << "for timestamp" << this->time;
    return vgImage();
    }

  vxl_int_64 ts;
  vil_image_view<vxl_byte> vilImage;

  // Read timestamp and image from data stream
  this->dataStream->clear_serialisation_records();
  vsl_b_read(*this->dataStream, ts);
  if (this->dataVersion < 3)
    {
    vsl_b_read(*this->dataStream, vilImage);
    }
  else
    {
    QScopedPointer<vil_file_format> format;
    char formatMarker;
    vsl_b_read(*this->dataStream, formatMarker);
    switch (formatMarker)
      {
      case 'j':
      case 'J':
        format.reset(new vil_jpeg_file_format);
        break;
      default:
        qDebug() << "vgKwaVideoFramePtr: unknown compression format"
                 << formatMarker;
        return vgImage();
      }
    vil_stream* memoryStream = new vil_stream_core();
    memoryStream->ref();
    vcl_vector<char> bytes;
    vsl_b_read(*this->dataStream, bytes);
    memoryStream->write(&bytes[0], bytes.size());
    vil_image_resource_sptr imageResource =
      format->make_input_image(memoryStream);
    if (!imageResource)
      {
      qDebug() << "vgKwaVideoFramePtr: failed to read frame image at"
               << this->dataOffset;
      memoryStream->unref();
      return vgImage();
      }
    vilImage = imageResource->get_view();
    memoryStream->unref();
    }

  // Sanity check timestamp
  vgTimeStamp dataTime, indexTime = this->time;
  indexTime.FrameNumber = vgTimeStamp::InvalidFrameNumber();
  dataTime.Time = ts;
  if (dataTime != indexTime)
    {
    qDebug() << "vgKwaVideoFramePtr: warning: time" << dataTime
             << "in data does not match time" << this->time << "from index";
    }

  if (!vilImage.is_contiguous())
    {
    qDebug() << "vgKwaVideoFramePtr: cannot convert non-contiguous image";
    return vgImage();
    }

  // Convert to vgImage
  vil_memory_chunk_sptr* chunk =
    new vil_memory_chunk_sptr(vilImage.memory_chunk());
  vgImage::Closure cleanup(&vilCleanup, chunk);
  return vgImage(vilImage.top_left_ptr(),
                 vilImage.ni(), vilImage.nj(), vilImage.nplanes(),
                 vilImage.istep(), vilImage.jstep(), vilImage.planestep(),
                 cleanup);
}

//END vgKwaVideoFramePtr

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgKwaVideoClip

//-----------------------------------------------------------------------------
vgKwaVideoClip::vgKwaVideoClip(const QUrl& uri) :
  vgVideo(new vgKwaVideoClipPrivate)
{
  QTE_D(vgKwaVideoClip);

  QUrl indexUri = uri;
  bool okay;

  // Get temporal limits (if any) from URI
  qint64 startTime = std::numeric_limits<qint64>::min();
  qint64 endTime = std::numeric_limits<qint64>::max();
  if (indexUri.hasQueryItem("StartTime"))
    {
    startTime = indexUri.queryItemValue("StartTime").toLongLong(&okay);
    if (!okay)
      {
      qWarning() << "vgKwaVideoClip: Ignoring invalid start time"
                 << indexUri.queryItemValue("StartTime");
      startTime = std::numeric_limits<qint64>::min();
      }
    indexUri.removeAllQueryItems("StartTime");
    }
  if (indexUri.hasQueryItem("EndTime"))
    {
    endTime = indexUri.queryItemValue("EndTime").toLongLong(&okay);
    if (!okay)
      {
      qWarning() << "vgKwaVideoClip: Ignoring invalid end time"
                 << indexUri.queryItemValue("EndTime");
      startTime = std::numeric_limits<qint64>::min();
      }
    indexUri.removeAllQueryItems("EndTime");
    }

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
