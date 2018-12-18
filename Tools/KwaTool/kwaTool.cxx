/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vgKwaFrameMetadata.h>
#include <vgKwaVideoClip.h>
#include <vgVideoBuffer.h>

#include <vgDebug.h>

#include <qtCliArgs.h>

#include <QApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QLabel>
#include <QTextStream>
#include <QUrl>

#include <cmath>

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
void dump(vgKwaVideoClip& clip, const vgKwaVideoClip::MetadataMap& md,
          QSet<QString> operations)
{
  if (operations.contains("help"))
    {
    qWarning() << "Available information items:";
    qWarning() << "  gsd      Frame ground sample distance";
    qWarning() << "  corners  Frame geodetic corner points";
    qWarning() << "  layout   Image data layout";
    return;
    }

  const auto dumpGsd = (operations.contains("gsd"));
  const auto dumpCorners = (operations.contains("corners"));
  const auto dumpLayout = (operations.contains("layout"));

  if (!(dumpGsd || dumpCorners || dumpLayout))
    {
    qWarning() << "dump: nothing to do!";
    return;
    }

  // Dump metadata first
  if (dumpGsd || dumpCorners)
    {
    foreach_iter (vgKwaVideoClip::MetadataMap::const_iterator, iter, md)
      {
      qDebug() << "at" << iter.key();
      if (dumpGsd)
        {
        qDebug() << "  gsd         :" << iter->gsd();
        }
      if (dumpCorners)
        {
        qDebug() << "  GCS         :" << iter->worldCornerPoints().GCS;
        qDebug() << "  upper left  :" << iter->worldCornerPoints().UpperLeft;
        qDebug() << "  upper right :" << iter->worldCornerPoints().UpperRight;
        qDebug() << "  lower left  :" << iter->worldCornerPoints().LowerLeft;
        qDebug() << "  lower right :" << iter->worldCornerPoints().LowerRight;
        }
      }
    }

  // Dump frame information
  if (dumpLayout)
    {
    clip.rewind();
    while (clip.currentTimeStamp().IsValid())
      {
      vgImage frame = clip.currentImage();
      qDebug() << "at" << clip.currentTimeStamp();
      qDebug() << "  dimensions  :"
               << frame.iCount() << frame.jCount() << frame.planeCount();
      qDebug() << "  strides     :"
               << frame.iStep() << frame.jStep() << frame.planeStep();
      clip.advance();
      }
    }
}

//-----------------------------------------------------------------------------
void profile(vgKwaVideoClip& clip, bool recode, const QByteArray& bufferFormat)
{
  const auto count = clip.frameCount();
  QElapsedTimer timer;

  qDebug() << "Beginning performance tests";
  timer.start();

  // Raw read test
  clip.rewind();
  while (clip.currentTimeStamp().IsValid())
    {
    // Force read of pixel data
    clip.currentImage().constData();
    clip.advance();
    }
  qDebug() << "  Read: took" << timer.restart() << "ms";

  if (recode)
    {
    clip.rewind();
    while (clip.currentTimeStamp().IsValid())
      {
      clip.currentImage().toQImage();
      clip.advance();
      }
    qDebug() << "  Recode: took" << timer.restart() << "ms";
    }

  if (!bufferFormat.isEmpty())
    {
    const auto f = (bufferFormat == "raw" ? 0 : bufferFormat.constData());
    vgVideoBuffer buffer(f);

    qDebug() << "Beginning buffer performance tests using"
             << bufferFormat << "format";
    timer.start();

    clip.rewind();
    while (clip.currentTimeStamp().IsValid())
      {
      buffer.insert(clip.currentTimeStamp(), clip.currentImage());
      clip.advance();
      }
    qDebug() << "  Buffer write: took" << timer.restart() << "ms";

    buffer.rewind();
    while (buffer.currentTimeStamp().IsValid())
      {
      // Force read of pixel data
      buffer.currentImage().constData();
      buffer.advance();
      }
    qDebug() << "  Buffer read: took" << timer.restart() << "ms";
    }
}

//-----------------------------------------------------------------------------
void writeHomographies(const vgKwaVideoClip::MetadataMap& md, QTextStream& s)
{
  s.setRealNumberNotation(QTextStream::SmartNotation);
  s.setRealNumberPrecision(16);

  int counter = 0;
  foreach_iter (auto, iter, md)
    {
    const auto& ts = iter.key();
    const auto& hm = iter->homography();
    s << counter++ << ' ' << QString::number(ts.Time, 'f', 0) << '\n';
    s << hm(0, 0) << ' ' << hm(0, 1) << ' ' << hm(0, 2) << '\n';
    s << hm(1, 0) << ' ' << hm(1, 1) << ' ' << hm(1, 2) << '\n';
    s << hm(2, 0) << ' ' << hm(2, 1) << ' ' << hm(2, 2) << '\n';
    s << '\n';
    }
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // Set application information
  QCoreApplication::setApplicationName("VisGUI track conversion utility");
  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");

  // Set up command line options
  qtCliArgs args(argc, argv);

  qtCliOptions options;

  options.add("dump <info>",
              "Dump the requested information items to stdout "
              "(comma separated; 'help' to see available items)")
         .add("d", qtCliOption::Short);

  options.add("profile",
              "Enable performance tests (with no other options, "
              "just test how long it takes to read the entire video)")
         .add("p", qtCliOption::Short);

  options.add("recode",
              "Enable conversion-to-QImage performance test")
         .add("r", qtCliOption::Short);

  options.add("buffer <format>",
              "Enable buffer read/write performance test, "
              "using the specified image format for storage "
              "(e.g. 'RAW', 'JPG', 'PNG')")
         .add("b", qtCliOption::Short);

  options.add("write-homographies <file>",
              "Write frame homographies to the specified file "
              "(use '-' to write to stdout)")
         .add("wh", qtCliOption::Short);

  options.add("write-frames <template>",
              "Write frame images to files using the specified "
              "file name template (must contain a '%1' placeholder "
              "that will be replaced by the frame number); "
              "the output format is guessed from the file extension")
         .add("wf", qtCliOption::Short);

  options.add("show-frames", "Display the video frames on screen")
         .add("s", qtCliOption::Short);

  args.addOptions(options);

  qtCliOptions namedArgs;
  namedArgs.add("video", "Path to KWA video index file", qtCliOption::Required);
  args.addNamedArguments(namedArgs);

  // Parse arguments
  args.parseOrDie();

  QScopedPointer<QCoreApplication> app;
  if (args.isSet("show-frames"))
    {
    app.reset(new QApplication(args.qtArgc(), args.qtArgv()));
    }
  else
    {
    app.reset(new QCoreApplication(args.qtArgc(), args.qtArgv()));
    }

  // Load the video file
  const auto& indexName = args.value("video");
  vgKwaVideoClip clip(QUrl::fromUserInput(indexName));
  const auto& md = clip.metadata();

  qDebug() << "Loaded video with" << md.count() << "frames from" << indexName;

  // Consider various operations that may be requested
  if (args.isSet("dump"))
    {
    const auto& ops = args.value("dump").toLower().split(",").toSet();
    dump(clip, md, ops);
    }

  if (args.isSet("profile"))
    {
    const auto recode = args.isSet("recode");
    const auto bufferFormat = args.value("buffer").toLocal8Bit().toLower();
    profile(clip, recode, bufferFormat);
    }

  if (args.isSet("write-homographies"))
    {
    const auto& filename = args.value("write-homographies");
    if (filename == "-")
      {
      QTextStream s(stdout, QIODevice::WriteOnly);
      writeHomographies(md, s);
      }
    else
      {
      QFile f(filename);
      if (f.open(QIODevice::WriteOnly))
        {
        QTextStream s(&f);
        writeHomographies(md, s);
        }
      else
        {
        qWarning() << "Failed to open homography file" << filename
                  << "-" << f.errorString();
        }
      }
    }

  const auto showFrames = args.isSet("show-frames");
  const auto writeFrames = args.isSet("write-frames");
  if (showFrames || writeFrames)
    {
    const auto& filenameTemplate = args.value("write-frames");
    int counter = 0;
    int digits = static_cast<int>(floor(log10(clip.frameCount() - 1))) + 1;

    QScopedPointer<QLabel> label;
    if (showFrames)
      {
      label.reset(new QLabel);
      label->show();
      }

    clip.rewind();
    while (clip.currentTimeStamp().IsValid())
      {
      const auto& frame = clip.currentImage();
      const auto& image = frame.toQImage();

      if (label)
        {
        label->setPixmap(QPixmap::fromImage(image));
        label->setFixedSize(image.size());
        label->update();
        app->processEvents();
        }

      if (writeFrames)
        {
        const auto& filename =
          filenameTemplate.arg(counter, digits, 10, QChar('0'));

        if (!image.save(filename))
          {
          qWarning() << "Failed to write frame" << filename;
          }
        }

      clip.advance();
      ++counter;
      }
    }

  return 0;
}
