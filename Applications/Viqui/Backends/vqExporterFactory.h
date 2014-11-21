/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
