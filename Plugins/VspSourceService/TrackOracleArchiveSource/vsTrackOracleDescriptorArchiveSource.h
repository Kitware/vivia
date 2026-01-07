// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsTrackOracleDescriptorArchiveSource_h
#define __vsTrackOracleDescriptorArchiveSource_h

#include <vsDescriptorSource.h>

#include <vsArchiveSource.h>

#include "track_oracle_file_format_fwd.h"

class vsTrackOracleDescriptorArchiveSourcePrivate;

class vsTrackOracleDescriptorArchiveSource :
  public vsArchiveSource<vsDescriptorSource>
{
  Q_OBJECT

public:
  vsTrackOracleDescriptorArchiveSource(const QUrl& archiveUri,
                                       track_oracle::file_format_base* format);
  virtual ~vsTrackOracleDescriptorArchiveSource();

private:
  QTE_DECLARE_PRIVATE(vsTrackOracleDescriptorArchiveSource)
  QTE_DISABLE_COPY(vsTrackOracleDescriptorArchiveSource)

  typedef vsArchiveSource<vsDescriptorSource> super;
};

#endif
