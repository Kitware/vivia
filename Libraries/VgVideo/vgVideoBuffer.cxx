/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgVideoBuffer.h"

#include <QDir>
#include <QImage>

#include <qtTemporaryFile.h>
#include <qtUtil.h>

#include <vgDebug.h>

#include "vgVideoFramePtrPrivate.h"
#include "vgVideoPrivate.h"

namespace // anonymous
{

//-----------------------------------------------------------------------------
void qiCleanup(void* data)
{
  QImage* qi = reinterpret_cast<QImage*>(data);
  delete qi;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vgVideoBufferPrivate : public vgVideoPrivate
{
public:
  vgVideoBufferPrivate(const char* cf) : CompressionFormat(cf) {}

  QSharedPointer<QFile> Store;
  QString CompressionFormat;
};

QTE_IMPLEMENT_D_FUNC(vgVideoBuffer)

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoDataStreamFramePtr

//-----------------------------------------------------------------------------
class vgVideoDataStreamFramePtr : public vgVideoFramePtrPrivate
{
public:
  explicit vgVideoDataStreamFramePtr(
    vgTimeStamp ts, QSharedPointer<QIODevice> store,
    quint64 offset, const QString& compressionFormat);

  vgImage data() const;

protected:
  const QSharedPointer<QIODevice> DataStore;
  const quint64 DataOffset;
  const QString CompressionFormat;
};

//-----------------------------------------------------------------------------
vgVideoDataStreamFramePtr::vgVideoDataStreamFramePtr(
  vgTimeStamp ts, QSharedPointer<QIODevice> store,
  quint64 offset, const QString& compressionFormat) :
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
      const char* const cf = qPrintable(this->CompressionFormat);
      QScopedPointer<QImage> qi(new QImage);
      if (qi->load(this->DataStore.data(), cf))
        {
        int planes = 0;
        int iStep = 0;
        int planeStep = 0;
        ptrdiff_t offset = 0;
        switch (qi->format())
          {
          case QImage::Format_RGB888:
            planes = 3;
            iStep = 3;
            planeStep = 1;
            offset = 0;
            break;
          case QImage::Format_RGB32:
            planes = 3;
            iStep = 4;
            planeStep = -1;
            offset = 2;
            break;
          default:
            qDebug() << "vgVideoDataStreamFramePtr(" << this << ')'
                     << "failed to read compressed image:"
                     << "unsupported QImage format" << qi->format();
            return vgImage();
          }

        QImage* qip = qi.take();
        unsigned char* topLeft = qip->bits() + offset;
        vgImage::Closure cleanup(&qiCleanup, qip);
        return vgImage(topLeft, qip->width(), qip->height(), planes,
                       iStep, qip->bytesPerLine(), planeStep, cleanup);
        }
      }
    }

  return vgImage();
}

//END vgVideoDataStreamFramePtr

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgVideoBuffer

//-----------------------------------------------------------------------------
vgVideoBuffer::vgVideoBuffer(const char* compressionFormat) :
  vgVideo(new vgVideoBufferPrivate(compressionFormat))
{
  QTE_D(vgVideoBuffer);

  // Generate path for temporary file for backing store
  QString ft = ".vg_video_buffer_XXXXXX";
  const QByteArray tdv = qgetenv("VG_VIDEO_TMPDIR");
  if (!tdv.isEmpty())
    {
    const QDir td(QString::fromLocal8Bit(tdv));
    const QString ctd = (td.exists() ? td.canonicalPath() : td.absolutePath());
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

//-----------------------------------------------------------------------------
vgVideoBuffer::~vgVideoBuffer()
{
}

//-----------------------------------------------------------------------------
bool vgVideoBuffer::insert(vgTimeStamp pos, vgImage image)
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
  qint64 offset = d->Store->size();
  d->Store->seek(offset);
  if (d->CompressionFormat.isEmpty())
    {
    QDataStream stream(d->Store.data());
    stream << image;
    }
  else
    {
    image.toQImage().save(d->Store.data(), qPrintable(d->CompressionFormat));
    }

  // Create frame pointer and insert into map
  vgVideoFramePtrPrivate* p =
    new vgVideoDataStreamFramePtr(pos, d->Store, offset, d->CompressionFormat);
  vgVideoFramePtr frame(p);
  d->pos = d->frames.insert(pos, frame);
  return true;
}

//END vgVideoBuffer
