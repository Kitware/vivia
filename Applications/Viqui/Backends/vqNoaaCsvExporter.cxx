/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QMessageBox>

#include <vgFileDialog.h>

#include <vvWriter.h>

#include "vqNoaaCsvExporter.h"

#include <fstream>

//-----------------------------------------------------------------------------
bool vqNoaaCsvExporter::exportResults(const QList<vvQueryResult>& results)
{
  // Get name of file to which results should be written
  QString selectedFilter;
  QString fileName = vgFileDialog::getSaveFileName(
                       0, "Save results...", QString(),
                       "NOAA CSV File (*.csv);;"
                       "All files (*)", &selectedFilter);
  if (fileName.isEmpty())
    return false;

  // Open output file
  std::ofstream file(fileName.toStdString());
  if (!file)
    {
    QString msg = "Unable to open file \"%1\" for writing";
    QMessageBox::critical(0, "Error writing file",
                          msg.arg(fileName));
    return false;
    }

  // Write the file
  file << "#image-index,"
       << "file-name,"
       << "TL-x,"
       << "TL-y,"
       << "BR-x,"
       << "BR-y,"
       << "fish-length,"
       << "confidence,"
       << "track-id,"
       << "{class-name,score},..."
       << std::endl;

  foreach (const auto result, results)
    {
    foreach (const auto track, result.Tracks)
      {
      bool first = true;
      foreach (const auto trackState, track.Trajectory)
        {
        file << trackState.TimeStamp.FrameNumber << ","
             << result.StreamId << ","
             << trackState.ImageBox.TopLeft.X << ","
             << trackState.ImageBox.TopLeft.Y << ","
             << trackState.ImageBox.BottomRight.X << ","
             << trackState.ImageBox.BottomRight.Y << ","
             << "0.0," // We don't know the fish length yet
             << result.RelevancyScore << ","
             << track.Id.SerialNumber;

        if (first)
          {
          foreach (const auto classification, track.Classification)
            {
            file << "," << classification.first << "," << classification.second;
            }

          first = false;
          }
        }

        file << std::endl;
      }
    }

  // Done
  return true;
}
