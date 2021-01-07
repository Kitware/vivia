// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfTrackOracleArchiveSourcePlugin_h
#define __vdfTrackOracleArchiveSourcePlugin_h

#include <vdfArchiveSourceInterface.h>

#include <qtGlobal.h>

#include <QObject>

class vdfTrackOracleArchiveSourcePluginPrivate;

class vdfTrackOracleArchiveSourcePlugin : public QObject,
                                          public vdfArchiveSourceInterface
{
  Q_OBJECT
  Q_INTERFACES(vdfArchiveSourceInterface)
  Q_PLUGIN_METADATA(IID "org.visgui.framework.data.archiveSourceInterface")

public:
  vdfTrackOracleArchiveSourcePlugin();
  virtual ~vdfTrackOracleArchiveSourcePlugin();

  virtual vdfArchivePluginInfo archivePluginInfo() const QTE_OVERRIDE;
  virtual vdfDataSource* createArchiveSource(
    const QUrl&, SourceCreateMode) QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfTrackOracleArchiveSourcePlugin)

private:
  QTE_DECLARE_PRIVATE(vdfTrackOracleArchiveSourcePlugin)
  QTE_DISABLE_COPY(vdfTrackOracleArchiveSourcePlugin)
};

#endif
