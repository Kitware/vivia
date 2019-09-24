/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfTrackOracleEventDataSource_h
#define __vdfTrackOracleEventDataSource_h

#include <vdfThreadedArchiveSource.h>

#include "track_oracle_file_format_fwd.h"

class vdfTrackOracleEventDataSourcePrivate;

class vdfTrackOracleEventDataSource : public vdfThreadedArchiveSource
{
  Q_OBJECT

public:
  vdfTrackOracleEventDataSource(
    const QUrl& archiveUri, track_oracle::file_format_base* format,
    QObject* parent = 0);
  virtual ~vdfTrackOracleEventDataSource();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfTrackOracleEventDataSource)

  virtual bool processArchive(const QUrl& uri);

private:
  QTE_DECLARE_PRIVATE(vdfTrackOracleEventDataSource)
  QTE_DISABLE_COPY(vdfTrackOracleEventDataSource)
};

#endif
