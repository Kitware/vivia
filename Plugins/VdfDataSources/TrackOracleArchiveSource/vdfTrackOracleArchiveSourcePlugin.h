/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
