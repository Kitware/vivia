/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqExporterFactory.h"

#include "vqVqrExporter.h"

//-----------------------------------------------------------------------------
QList<vqExporterFactory::Identifier> vqExporterFactory::exporters()
{
  QList<vqExporterFactory::Identifier> list;
  list.append(Identifier("file-vqr", "VisGUI Query Results"));
  return list;
}

//-----------------------------------------------------------------------------
vqExporter* vqExporterFactory::createExporter(QString type)
{
  if (type == "file-vqr")
    return new vqVqrExporter();
  return 0;
}

//-----------------------------------------------------------------------------
vqExporter* vqExporterFactory::createNativeFileExporter()
{
  return new vqVqrExporter();
}
