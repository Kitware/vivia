/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
