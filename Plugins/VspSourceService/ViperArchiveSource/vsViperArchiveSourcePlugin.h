/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsViperArchiveSourcePlugin_h
#define __vsViperArchiveSourcePlugin_h

#include <QObject>

#include <qtGlobal.h>

#include <vsArchiveSourceInterface.h>

class vsViperArchiveSourcePlugin : public QObject,
                                   public vsArchiveSourceInterface
{
  Q_OBJECT
  Q_INTERFACES(vsArchiveSourceInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsArchiveSourceInterface")

public:
  vsViperArchiveSourcePlugin();
  virtual ~vsViperArchiveSourcePlugin();

  virtual vsArchiveSourceTypes archiveTypes() const QTE_OVERRIDE;
  virtual vsArchivePluginInfo archivePluginInfo(
    vsArchiveSourceType) const QTE_OVERRIDE;
  virtual vsSimpleSourceFactory* createArchiveSource(
    vsArchiveSourceType, const QUrl&, SourceCreateMode) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsViperArchiveSourcePlugin)
};

#endif
