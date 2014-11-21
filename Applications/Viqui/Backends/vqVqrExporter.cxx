/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QFile>
#include <QMessageBox>

#include <vgFileDialog.h>

#include <vvWriter.h>

#include "vqVqrExporter.h"

//-----------------------------------------------------------------------------
bool vqVqrExporter::exportResults(const QList<vvQueryResult>& results)
{
  // Get name of file to which results should be written
  QString selectedFilter;
  QString fileName = vgFileDialog::getSaveFileName(
                       0, "Save results...", QString(),
                       "VisGUI Query Results (*.vqr *.vqrx);;"
                       "VisGUI Query Results - KST (*.vqr);;"
                       "VisGUI Query Results - XML (*.vqrx *.xml);;"
                       "All files (*)", &selectedFilter);
  if (fileName.isEmpty())
    return false;

  // Open output file
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
    QString msg = "Unable to open file \"%1\" for writing: %2";
    QMessageBox::critical(0, "Error writing file",
                          msg.arg(fileName).arg(file.errorString()));
    return false;
    }

  // Choose format from extension
  const QString ext = QFileInfo(fileName).suffix();
  const bool saveAsXml =
    (ext == "xml" || ext == "vqrx" || selectedFilter.contains("XML"));
  vvWriter::Format format = (saveAsXml ? vvWriter::Xml : vvWriter::Kst);

  // Write results
  vvWriter writer(file, format);
  writer << vvHeader::QueryResults;
  foreach (const vvQueryResult& result, results)
    writer << result;

  // Done
  return true;
}
