// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfTrackOracleTrackDataSource_h
#define __vdfTrackOracleTrackDataSource_h

#include <vdfThreadedArchiveSource.h>

#include "track_oracle_file_format_fwd.h"

class vdfTrackOracleTrackDataSourcePrivate;

class vdfTrackOracleTrackDataSource : public vdfThreadedArchiveSource
{
  Q_OBJECT

public:
  vdfTrackOracleTrackDataSource(
    const QUrl& archiveUri, track_oracle::file_format_base* format,
    QObject* parent = 0);
  virtual ~vdfTrackOracleTrackDataSource();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfTrackOracleTrackDataSource)

  virtual bool processArchive(const QUrl& uri);

private:
  QTE_DECLARE_PRIVATE(vdfTrackOracleTrackDataSource)
  QTE_DISABLE_COPY(vdfTrackOracleTrackDataSource)
};

#endif
