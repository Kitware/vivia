// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "../vgKwaFrameMetadata.h"
#include "../vgKwaVideoClip.h"
#include "../vgVideoBuffer.h"

#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QLabel>
#include <QUrl>

#include <vgDebug.h>

//-----------------------------------------------------------------------------
QDebug& operator<<(QDebug& dbg, vgGeoRawCoordinate coord)
{
  dbg.nospace()
    << "(Northing " << coord.Northing
    << ", Easting " << coord.Easting
    << ')';
  return dbg.space();
}

//-----------------------------------------------------------------------------
class Argument
{
public:
  Argument(const QString& arg);

  QString name() const { return this->Name; }
  QString value() const { return this->Value; }

  bool operator==(const QString& otherName) const
    {
    return this->name() == otherName;
    }

protected:
  QString Name;
  QString Value;
};

//-----------------------------------------------------------------------------
Argument::Argument(const QString& arg)
{
  const int n = arg.indexOf('=');
  this->Name = arg.left(n);
  this->Value = (n > 0 ? arg.mid(n + 1) : QString());
}

//-----------------------------------------------------------------------------
class PerformanceTimer
{
public:
  PerformanceTimer(const QString& msg, int frameCount);
  ~PerformanceTimer();

protected:
  const QString Message;
  const int FrameCount;
  const QDateTime StartTime;
};

//-----------------------------------------------------------------------------
PerformanceTimer::PerformanceTimer(const QString& msg, int frameCount) :
  Message(msg),
  FrameCount(frameCount),
  StartTime(QDateTime::currentDateTimeUtc())
{
}

//-----------------------------------------------------------------------------
PerformanceTimer::~PerformanceTimer()
{
  const QDateTime endTime = QDateTime::currentDateTimeUtc();
  double delta = 1e-3 * this->StartTime.msecsTo(endTime);
  double rate = this->FrameCount / delta;
  qWarning().nospace()
    << qPrintable(this->Message) << ": "
    << qPrintable(QString::number(delta, 'f', 3)) << " seconds ("
    << qPrintable(QString::number(rate, 'f', 2)) << " frames/second)";
}

//-----------------------------------------------------------------------------
void usage(const QString& argv0)
{
  QFileInfo fi(argv0);
  qWarning() << "Usage:" << qPrintable(fi.fileName())
             << "[ <options> ] <video file>";
  qWarning() << "  Available options:";
  qWarning() << "    --count        Report the number of frames in the video";
  qWarning() << "    --dump-gsd     Print the GSD of each frame";
  qWarning() << "    --dump-corners Print the corner points of each frame";
  qWarning() << "    --dump-homographies";
  qWarning() << "                   Print the homography matrix and reference";
  qWarning() << "                   frame of each frame";
  qWarning() << "    --dump-image-format";
  qWarning() << "                   Print the size and strides of each frame";
  qWarning() << "    --show-frames  Display the frame image data in a window";
  qWarning() << "    --test-performance[=<compression format>]";
  qWarning() << "                   Run some performance tests";
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  QStringList args = app.arguments();

  // Get the video file name
  QString fileName;
  for (int n = 1; n < args.count(); ++n)
    {
    if (!args[n].startsWith("--"))
      {
      fileName = args[n];
      break;
      }
    }
  if (fileName.isEmpty())
    {
    qWarning() << "Error: No video file given";
    usage(args[0]);
    return 1;
    }

  // Load the video file
  vgKwaVideoClip clip(QUrl::fromUserInput(fileName));
  vgKwaVideoClip::MetadataMap md = clip.metadata();

  // Consider options
  for (int n = 1; n < args.count(); ++n)
    {
    const Argument arg(args[n]);
    if (arg == "--count")
      {
      qDebug() << md.count();
      }
    else if (arg == "--dump-gsd")
      {
      foreach_iter (vgKwaVideoClip::MetadataMap::const_iterator, iter, md)
        qDebug() << "at" << iter.key() << "gsd" << iter.value().gsd();
      }
    else if (arg == "--dump-corners")
      {
      foreach_iter (vgKwaVideoClip::MetadataMap::const_iterator, iter, md)
        {
        qDebug() << "at" << iter.key();
        qDebug() << "  GCS         :" << iter.value().worldCornerPoints().GCS;
        qDebug() << "  upper left  :"
                 << iter.value().worldCornerPoints().UpperLeft;
        qDebug() << "  upper right :"
                 << iter.value().worldCornerPoints().UpperRight;
        qDebug() << "  lower left  :"
                 << iter.value().worldCornerPoints().LowerLeft;
        qDebug() << "  lower right :"
                 << iter.value().worldCornerPoints().LowerRight;
        }
      }
    else if (arg == "--dump-homographies")
      {
      foreach_iter (vgKwaVideoClip::MetadataMap::const_iterator, iter, md)
        {
        qDebug() << "at" << iter.key();
        if (iter.value().homographyReferenceFrameNumber() >= 0)
          {
          qDebug() << "  matrix          :" << iter.value().homography();
          qDebug() << "  reference frame :"
                   << iter.value().homographyReferenceFrameNumber();
          }
        else
          {
          qDebug() << "  no homography is available for this frame";
          }
        }
      }
    else if (arg == "--dump-image-format")
      {
      clip.rewind();
      while (clip.currentTimeStamp().IsValid())
        {
        vgImage frame = clip.currentImage();
        qDebug() << "at" << clip.currentTimeStamp()
                 << frame.iCount() << frame.jCount() << frame.planeCount()
                 << frame.iStep() << frame.jStep() << frame.planeStep();
        clip.advance();
        }
      }
    else if (arg == "--show-frames")
      {
      QLabel label;

      label.show();
      clip.rewind();
      while (clip.currentTimeStamp().IsValid())
        {
        const QImage image = clip.currentImage().toQImage();
        label.setPixmap(QPixmap::fromImage(image));
        label.setFixedSize(image.size());
        label.update();
        app.processEvents();
        clip.advance();
        }
      }
    else if (arg == "--test-performance")
      {
      const int count = clip.frameCount();
      for (int n = 0; n < 2; ++n)
        {
        PerformanceTimer timer(QString("Raw read (iteration %1)").arg(n + 1),
                               count);
        clip.rewind();
        while (clip.currentTimeStamp().IsValid())
          {
          // Force read of pixel data
          clip.currentImage().constData();
          clip.advance();
          }
        }

      if (arg.value() == "recode")
        {
        PerformanceTimer timer("Recode", count);

        clip.rewind();
        while (clip.currentTimeStamp().IsValid())
          {
          clip.currentImage().toQImage();
          clip.advance();
          }
        }
      else if (!arg.value().isEmpty())
        {
        const QString format = arg.value();
        vgVideoBuffer buffer(format == "RAW" ? 0 : qPrintable(format));

        if (1) // Scope for PerformanceTimer
          {
          PerformanceTimer timer(QString("Compress (%1)").arg(format), count);

          clip.rewind();
          while (clip.currentTimeStamp().IsValid())
            {
            buffer.insert(clip.currentTimeStamp(), clip.currentImage());
            clip.advance();
            }
          }

        if (1) // Scope for PerformanceTimer
          {
          PerformanceTimer timer(QString("Decompress (%1)").arg(format), count);

          buffer.rewind();
          while (buffer.currentTimeStamp().IsValid())
            {
            // Force read of pixel data
            buffer.currentImage().constData();
            buffer.advance();
            }
          }
        }
      }
    }

  return 0;
}
