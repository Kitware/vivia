// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsTrackOracleTrackArchiveSource_h
#define __vsTrackOracleTrackArchiveSource_h

#include <vsTrackSource.h>

#include <vsArchiveSource.h>

#include "track_oracle_file_format_fwd.h"

class vsTrackOracleTrackArchiveSourcePrivate;

class vsTrackOracleTrackArchiveSource : public vsArchiveSource<vsTrackSource>
{
  Q_OBJECT

public:
  vsTrackOracleTrackArchiveSource(const QUrl& archiveUri,
                                  track_oracle::file_format_base* format);
  virtual ~vsTrackOracleTrackArchiveSource();

private:
  QTE_DECLARE_PRIVATE(vsTrackOracleTrackArchiveSource)
  QTE_DISABLE_COPY(vsTrackOracleTrackArchiveSource)

  typedef vsArchiveSource<vsTrackSource> super;
};

#endif
