// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqExporterFactory_h
#define __vqExporterFactory_h

#include <QList>
#include <QString>

#include "vqExporter.h"

class vqExporterFactory
{
public:
  struct Identifier
    {
    Identifier(QString id_, QString displayString_)
      : id(id_), displayString(displayString_) {}
    QString id;
    QString displayString;
    };

  static QList<Identifier> exporters();
  static vqExporter* createExporter(QString id);
  static vqExporter* createNativeFileExporter();
};

#endif // __vqExporterFactory_h
