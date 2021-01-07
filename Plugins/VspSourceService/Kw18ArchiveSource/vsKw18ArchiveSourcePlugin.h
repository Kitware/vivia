// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsKw18ArchiveSourcePlugin_h
#define __vsKw18ArchiveSourcePlugin_h

#include <QObject>

#include <qtGlobal.h>

#include <vsArchiveSourceInterface.h>

class vsKw18ArchiveSourcePlugin : public QObject,
                                  public vsArchiveSourceInterface
{
  Q_OBJECT
  Q_INTERFACES(vsArchiveSourceInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.vsArchiveSourceInterface")

public:
  vsKw18ArchiveSourcePlugin();
  virtual ~vsKw18ArchiveSourcePlugin();

  virtual vsArchiveSourceTypes archiveTypes() const QTE_OVERRIDE;
  virtual vsArchivePluginInfo archivePluginInfo(
    vsArchiveSourceType) const QTE_OVERRIDE;
  virtual vsSimpleSourceFactory* createArchiveSource(
    vsArchiveSourceType, const QUrl&, SourceCreateMode) QTE_OVERRIDE;

private:
  QTE_DISABLE_COPY(vsKw18ArchiveSourcePlugin)
};

#endif
