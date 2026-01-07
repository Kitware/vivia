// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgVideoBuffer.h"

#include "vgVideoFramePtrPrivate.h"
#include "vgVideoPrivate.h"

#include <vgDebug.h>

#include <qtTemporaryFile.h>
#include <qtUtil.h>

#include <QBuffer>
#include <QDir>
#include <QImage>

//BEGIN vgVideoDataStreamFramePtr

//-----------------------------------------------------------------------------
class vgVideoDataStreamFramePtr : public vgVideoFramePtrPrivate
{
public:
  explicit vgVideoDataStreamFramePtr(
    vgTimeStamp ts, QSharedPointer<QIODevice> store,
    quint64 offset, const QByteArray& compressionFormat);

  vgImage data() const;

protected:
  const QSharedPointer<QIODevice> DataStore;
  const quint64 DataOffset;
  const QByteArray CompressionFormat;
};

//-----------------------------------------------------------------------------
vgVideoDataStreamFramePtr::vgVideoDataStreamFramePtr(
  vgTimeStamp ts, QSharedPointer<QIODevice> store,
  quint64 offset, const QByteArray& compressionFormat) :
  DataStore(store),
  DataOffset(offset),
  CompressionFormat(compressionFormat)
{
  this->time = ts;
}

//-----------------------------------------------------------------------------
vgImage vgVideoDataStreamFramePtr::data() const
{
  if (this->DataStore->seek(this->DataOffset))
    {
    if (this->CompressionFormat.isEmpty())
      {
      vgImage image;
      QDataStream stream(this->DataStore.data());
      stream >> image;
      return image;
      }
    else
      {
      QImage qi;
      if (qi.load(this->DataStore.data(), this->CompressionFormat.constData()))
        {
        return vgImage(qi);
        }
      }
    }

  return vgImage();
}

//END vgVideoDataStreamFramePtr

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoBufferPrivate

//-----------------------------------------------------------------------------
class vgVideoBufferPrivate : public vgVideoPrivate
{
public:
  vgVideoBufferPrivate(const char* cf) : CompressionFormat(cf) {}

  void insertFrame(const vgTimeStamp& pos, qint64 offset,
                   const QByteArray& compressionFormat);

  void insert(const vgTimeStamp& pos, const vgImage& image);
  void insert(const vgTimeStamp& pos, const QByteArray& data,
              const QByteArray& compressionFormat);
  void insert(const vgTimeStamp& pos, const QImage& image,
              const QByteArray& compressionFormat);

  QSharedPointer<QIODevice> Store;
  QByteArray CompressionFormat;
};

QTE_IMPLEMENT_D_FUNC(vgVideoBuffer)

//-----------------------------------------------------------------------------
void vgVideoBufferPrivate::insertFrame(
  const vgTimeStamp& pos, qint64 offset, const QByteArray& compressionFormat)
{
  vgVideoFramePtr frame(
    new vgVideoDataStreamFramePtr(pos, this->Store, offset, compressionFormat));
  this->pos = this->frames.insert(pos, frame);
}

//-----------------------------------------------------------------------------
void vgVideoBufferPrivate::insert(const vgTimeStamp& pos, const vgImage& image)
{
  // Write image to backing store
  qint64 offset = this->Store->size();
  this->Store->seek(offset);

  QDataStream stream(this->Store.data());
  stream << image;

  // Create frame pointer and insert into map
  this->insertFrame(pos, offset, QByteArray());
}

//-----------------------------------------------------------------------------
void vgVideoBufferPrivate::insert(
  const vgTimeStamp& pos, const QByteArray& data,
  const QByteArray& compressionFormat)
{
  // Write image to backing store
  qint64 offset = this->Store->size();
  this->Store->seek(offset);

  this->Store->write(data);

  // Create frame pointer and insert into map
  this->insertFrame(pos, offset, compressionFormat);
}

//-----------------------------------------------------------------------------
void vgVideoBufferPrivate::insert(const vgTimeStamp& pos, const QImage& image,
                                  const QByteArray& compressionFormat)
{
  // Write image to backing store
  qint64 offset = this->Store->size();
  this->Store->seek(offset);

  image.save(this->Store.data(), compressionFormat.constData());

  // Create frame pointer and insert into map
  this->insertFrame(pos, offset, compressionFormat);
}

//END vgVideoBufferPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoBuffer

//-----------------------------------------------------------------------------
vgVideoBuffer::vgVideoBuffer(const char* compressionFormat) :
  vgVideo(new vgVideoBufferPrivate(compressionFormat))
{
  QTE_D(vgVideoBuffer);

  // Determine if we are using a file or memory for the buffer
  const auto& tdv = qgetenv("VG_VIDEO_TMPDIR");
  if (tdv == "RAMDEV")
    {
    // Create a new memory buffer
    QSharedPointer<QBuffer> b(new QBuffer);
    b->open(QIODevice::ReadWrite);

    // Assign buffer to ourselves
    d->Store = b;
    }
  else
    {
    // Generate path for temporary file for backing store
    QString ft = ".vg_video_buffer_XXXXXX";
    if (!tdv.isEmpty())
      {
      const QDir td(QString::fromLocal8Bit(tdv));
      const auto& ctd = (td.exists() ? td.canonicalPath() : td.absolutePath());
      ft = QString("%2/%1").arg(ft).arg(ctd);
      }

    // Create temporary file for backing store
    QSharedPointer<qtTemporaryFile> f(new qtTemporaryFile(ft));

    // Open file
    if (!f->open())
      {
      qDebug() << "vgVideoBuffer(" << this << ')'
               << "failed to open backing store:"
               << qPrintable(f->errorString());
      return;
      }

    // Assign file handle to ourselves
    d->Store = f;
    }
}

//-----------------------------------------------------------------------------
vgVideoBuffer::~vgVideoBuffer()
{
}

//-----------------------------------------------------------------------------
bool vgVideoBuffer::insert(const vgTimeStamp& pos, const vgImage& image)
{
  QTE_D(vgVideoBuffer);

  // Disallow duplicates
  if (d->frames.contains(pos))
    {
    qDebug() << "vgVideoBuffer(" << this << ')'
             << "tried to insert frame at timestamp" << pos
             << "which already exists";
    return false;
    }

  // Fail if backing store does not exist
  if (!d->Store)
    {
    return false;
    }

  // Write image to backing store
  if (d->CompressionFormat.isEmpty())
    {
    d->insert(pos, image);
    }
  else
    {
    d->insert(pos, image.toQImage(), d->CompressionFormat);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vgVideoBuffer::insert(const vgTimeStamp& pos, const QByteArray& imageData,
                           const QByteArray& imageFormat)
{
  QTE_D(vgVideoBuffer);

  // Disallow duplicates
  if (d->frames.contains(pos))
    {
    qDebug() << "vgVideoBuffer(" << this << ')'
             << "tried to insert frame at timestamp" << pos
             << "which already exists";
    return false;
    }

  // Fail if backing store does not exist
  if (!d->Store)
    {
    return false;
    }

  // Check if compression format matches
  if (imageFormat.toLower() == d->CompressionFormat.toLower())
    {
    // Yes; insert the raw data directly
    d->insert(pos, imageData, imageFormat);
    }
  else
    {
    // No match; try to decode the image data
    QImage qi = QImage::fromData(imageData, "JPG");
    if (qi.isNull())
      {
      qWarning() << "vgVideoBuffer(" << this << ')'
                 << "failed to decode" << imageFormat << "image";
      return false;
      }

    // Are we using compression?
    if (d->CompressionFormat.isEmpty())
      {
      // No; insert the raw image
      d->insert(pos, vgImage(qi));
      }
    else
      {
      // Yes; insert the image, with compression
      d->insert(pos, qi, d->CompressionFormat);
      }
    }

  return true;
}

//END vgVideoBuffer
