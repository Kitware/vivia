// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
