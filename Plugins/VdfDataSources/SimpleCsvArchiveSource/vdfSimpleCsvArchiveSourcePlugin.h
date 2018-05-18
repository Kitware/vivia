/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfSimpleCsvArchiveSourcePlugin_h
#define __vdfSimpleCsvArchiveSourcePlugin_h

#include <vdfArchiveSourceInterface.h>

#include <qtGlobal.h>

#include <QObject>

class vdfSimpleCsvArchiveSourcePluginPrivate;

class vdfSimpleCsvArchiveSourcePlugin : public QObject,
                                        public vdfArchiveSourceInterface
{
  Q_OBJECT
  Q_INTERFACES(vdfArchiveSourceInterface)

public:
  vdfSimpleCsvArchiveSourcePlugin();
  virtual ~vdfSimpleCsvArchiveSourcePlugin();

  virtual vdfArchivePluginInfo archivePluginInfo() const QTE_OVERRIDE;
  virtual vdfDataSource* createArchiveSource(
    const QUrl&, SourceCreateMode) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vdfSimpleCsvArchiveSourcePlugin)
};

#endif
