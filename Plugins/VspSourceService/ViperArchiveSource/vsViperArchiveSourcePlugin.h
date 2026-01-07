// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
