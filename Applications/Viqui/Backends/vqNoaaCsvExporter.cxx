/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqNoaaCsvExporter.h"

#include <vvQueryResult.h>

#include <vgFileDialog.h>

#include <qtGet.h>
#include <qtStlUtil.h>

#include <QApplication>
#include <QDebug>
#include <QHash>
#include <QTextStream>
#include <QMessageBox>

namespace // anonymous
{
using ImageNameMap = QHash<QString, QHash<unsigned int, QString>>;

//-----------------------------------------------------------------------------
QString getImageName(
  ImageNameMap& map, const std::string& streamId, unsigned int frameNumber)
{
  const auto& imageList = qtString(streamId);

  auto* fmap = qtGet(map, imageList);
  if (!fmap)
  {
    fmap = &map[imageList];
    QFile file{imageList};
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      unsigned int n = 0;
      while (!file.atEnd())
      {
        fmap->insert(++n, QString::fromLocal8Bit(file.readLine()));
      }
    }
    else
    {
      qWarning() << "Unable to open image list" << imageList;
    }
  }

  if (const auto* const imageName = qtGet(*fmap, frameNumber))
  {
    return *imageName;
  }

  qWarning() << "No match for frame" << frameNumber
             << "in image list" << imageList;
  return imageList;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
bool vqNoaaCsvExporter::exportResults(const QList<vvQueryResult>& results)
{
  // Get name of file to which results should be written
  const auto& fileName = vgFileDialog::getSaveFileName(
    qApp->activeWindow(), "Save results...", {},
    "NOAA CSV File (*.csv);;"
    "All files (*)");

  if (fileName.isEmpty())
  {
    return false;
  }

  // Open output file
  QFile file{fileName};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    auto msgTemplate = QString{"Unable to open file \"%1\" for writing: %2"};
    QMessageBox::critical(0, "Error writing file",
                          msgTemplate.arg(fileName, file.errorString()));
    return false;
  }
  QTextStream s{&file};

  // Write the file
  s << "#track-id,"
        "file-name,"
        "image-index,"
        "TL-x,"
        "TL-y,"
        "BR-x,"
        "BR-y,"
        "confidence,"
        "fish-length,"
        "{class-name,score},..."
        "\n";

  ImageNameMap imageNameMap;
  foreach (const auto result, results)
  {
    foreach (const auto track, result.Tracks)
    {
      bool first = true;
      foreach (const auto trackState, track.Trajectory)
      {
        const auto& imageName =
          getImageName(imageNameMap, result.StreamId,
                       trackState.TimeStamp.FrameNumber);

        s << track.Id.SerialNumber << ','
          << imageName << ','
          << trackState.TimeStamp.FrameNumber << ','
          << trackState.ImageBox.TopLeft.X << ','
          << trackState.ImageBox.TopLeft.Y << ','
          << trackState.ImageBox.BottomRight.X << ','
          << trackState.ImageBox.BottomRight.Y << ','
          << result.RelevancyScore << ','
          << "-1"/*scalar*/;

        if (first)
        {
          foreach (const auto classification, track.Classification)
          {
            s << ',' << qtString(classification.first)
              << ',' << classification.second;
          }
          first = false;
        }
      }

      s << '\n';
    }
  }

  // Done
  return true;
}
