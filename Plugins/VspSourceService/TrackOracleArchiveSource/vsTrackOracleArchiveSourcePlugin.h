/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackOracleArchiveSourcePlugin_h
#define __vsTrackOracleArchiveSourcePlugin_h

#include <QObject>

#include <qtGlobal.h>

#include <vsArchiveSourceInterface.h>

class vsTrackOracleArchiveSourcePluginPrivate;

class vsTrackOracleArchiveSourcePlugin : public QObject,
                                         public vsArchiveSourceInterface
{
  Q_OBJECT
  Q_INTERFACES(vsArchiveSourceInterface)

public:
  vsTrackOracleArchiveSourcePlugin();
  virtual ~vsTrackOracleArchiveSourcePlugin();

  virtual vsArchiveSourceTypes archiveTypes() const QTE_OVERRIDE;
  virtual vsArchivePluginInfo archivePluginInfo(
    vsArchiveSourceType) const QTE_OVERRIDE;
  virtual vsSimpleSourceFactory* createArchiveSource(
    vsArchiveSourceType, const QUrl&, SourceCreateMode) QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsTrackOracleArchiveSourcePlugin)

private:
  QTE_DECLARE_PRIVATE(vsTrackOracleArchiveSourcePlugin)
  QTE_DISABLE_COPY(vsTrackOracleArchiveSourcePlugin)
};

#endif
