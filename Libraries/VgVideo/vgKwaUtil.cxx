/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgKwaUtilPrivate.h"

#include "vgKwaFrameMetadata.h"

#include <vgDebug.h>

#include <vil/file_formats/vil_jpeg.h>
#include <vil/vil_stream_core.h>

#include <vil/io/vil_io_image_view.h>
#include <vsl/vsl_binary_io.h>
#include <vsl/vsl_stream.h>
#include <vsl/vsl_vector_io.h>
#include <vsl/vsl_vector_io.hxx>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPair>

#include <vector>

VSL_VECTOR_IO_INSTANTIATE(char);

#define die(...) do { \
  qDebug() << "vgKwaUtil:" << __VA_ARGS__; \
  return false; \
  } while (0)

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
bool vgKwaUtil::resolvePath(
  QString& name, const QString& relativeTo, const char* what)
{
  // Look for the file in four places:
  // 0. absolute path (if <name> is absolute)
  // 1. relative to the current working directory
  // 2. relative to <relativeTo>
  // 3. relative to the canonical location of <relativeTo>
  //
  // Most likely, <name> is given as a path relative to the canonical location
  // of <relativeTo>. Since the path we received may be a symlink, we should
  // try (3) as well as (2).
  QFileInfo fi(name);
  if (fi.isAbsolute())
    {
    if (fi.exists())
      {
      return true;
      }
    die("Unable to find" << what << "file" << name);
    }

  fi = QFileInfo(QDir(), name);
  QString try1 = fi.absoluteFilePath();
  if (fi.exists())
    {
    name = try1;
    return true;
    }

  QFileInfo rfi(relativeTo);
  fi = QFileInfo(rfi.path(), name);
  QString try2 = fi.absoluteFilePath();
  if (fi.exists())
    {
    name = try2;
    return true;
    }

  fi = QFileInfo(rfi.canonicalPath(), name);
  QString try3 = fi.absoluteFilePath();
  if (fi.exists())
    {
    name = try3;
    return true;
    }

  die("Unable to find" << what << "file" << name
      << "\n  Tried:" << try1
      << "\n        " << try2
      << "\n        " << try3);
}

//-----------------------------------------------------------------------------
QPair<long long, vgImage> vgKwaUtil::readFrame(
    vgIStream* store, vsl_b_istream* stream, int version, long long offset,
    const char* warningPrefix, const vgTimeStamp& expectedTime)
{
  // Seek stream to appropriate location
  if (!store->seek(offset))
    {
    if (expectedTime.IsValid())
      {
      qDebug() << warningPrefix << "failed to seek to position"
               << offset << "for timestamp" << expectedTime;
      }
    else
      {
      qDebug() << warningPrefix << "failed to seek to position" << offset;
      }
    return {};
    }

  vxl_int_64 ts;
  vil_image_view<vxl_byte> vilImage;

  // Read timestamp and image from data stream
  stream->clear_serialisation_records();
  vsl_b_read(*stream, ts);
  if (version < 3)
    {
    vsl_b_read(*stream, vilImage);
    }
  else
    {
    QScopedPointer<vil_file_format> format;
    char formatMarker;
    vsl_b_read(*stream, formatMarker);
    switch (formatMarker)
      {
      case 'j':
      case 'J':
        format.reset(new vil_jpeg_file_format);
        break;
      default:
        qDebug() << warningPrefix << "unknown compression format"
                 << formatMarker;
        return {};
      }
    vil_stream* memoryStream = new vil_stream_core();
    memoryStream->ref();
    std::vector<char> bytes;
    vsl_b_read(*stream, bytes);
    memoryStream->write(&bytes[0], static_cast<vil_streampos>(bytes.size()));
    vil_image_resource_sptr imageResource =
      format->make_input_image(memoryStream);
    if (!imageResource)
      {
      qDebug() << warningPrefix << "failed to read frame image at" << offset;
      memoryStream->unref();
      return {};
      }
    vilImage = imageResource->get_view();
    memoryStream->unref();
    }

  if (!vilImage.is_contiguous())
    {
    qDebug() << warningPrefix << "cannot convert non-contiguous image";
    return {};
    }

  // Convert to vgImage
  vil_memory_chunk_sptr* chunk =
    new vil_memory_chunk_sptr(vilImage.memory_chunk());
  vgImage::Closure cleanup(&vilCleanup, chunk);

  auto image = vgImage{vilImage.top_left_ptr(),
                       static_cast<int>(vilImage.ni()),
                       static_cast<int>(vilImage.nj()),
                       static_cast<int>(vilImage.nplanes()),
                       static_cast<int>(vilImage.istep()),
                       static_cast<int>(vilImage.jstep()),
                       static_cast<int>(vilImage.planestep()),
                       cleanup};

  return {ts, image};
}

//-----------------------------------------------------------------------------
QPair<long long, vgImage> vgKwaUtil::readFrame(
    const QString& dataPath, long long offset)
{
  static const char* warningPrefix = "vgKwaUtil::readFrame:";
  vgIStream store;

  if (!store.open(dataPath))
    {
    qDebug() << warningPrefix << "failed to open KWA data file" << dataPath;
    return {};
    }

  // Read data version
  vsl_b_istream stream(store);

  int dataVersion;
  vsl_b_read(stream, dataVersion);

  if (dataVersion < 1 ||
      dataVersion > vgKwaFrameMetadata::SupportedDataVersion)
    {
    qDebug() << warningPrefix << "KWA version" << dataVersion
             << "is not supported";
    return {};
    }

  return readFrame(&store, &stream, dataVersion, offset, warningPrefix);
}
