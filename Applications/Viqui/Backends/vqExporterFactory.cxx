/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqExporterFactory.h"

#include "vqVqrExporter.h"
#include "vqNoaaCsvExporter.h"

//-----------------------------------------------------------------------------
QList<vqExporterFactory::Identifier> vqExporterFactory::exporters()
{
  QList<vqExporterFactory::Identifier> list;
  list.append(Identifier("file-vqr", "VisGUI Query Results"));
  list.append(Identifier("file-noaa-csv", "NOAA CSV"));
  return list;
}

//-----------------------------------------------------------------------------
vqExporter* vqExporterFactory::createExporter(QString type)
{
  if (type == "file-vqr")
    return new vqVqrExporter();
  else if (type == "file-noaa-csv")
    return new vqNoaaCsvExporter();
  return 0;
}

//-----------------------------------------------------------------------------
vqExporter* vqExporterFactory::createNativeFileExporter()
{
  return new vqVqrExporter();
}
