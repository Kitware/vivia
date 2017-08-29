/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
