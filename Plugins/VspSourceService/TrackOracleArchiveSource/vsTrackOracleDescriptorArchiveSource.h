/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackOracleDescriptorArchiveSource_h
#define __vsTrackOracleDescriptorArchiveSource_h

#include <vsDescriptorSource.h>

#include <vsArchiveSource.h>

namespace vidtk
{
  class file_format_base;
}

class vsTrackOracleDescriptorArchiveSourcePrivate;

class vsTrackOracleDescriptorArchiveSource :
  public vsArchiveSource<vsDescriptorSource>
{
  Q_OBJECT

public:
  vsTrackOracleDescriptorArchiveSource(const QUrl& archiveUri,
                                       vidtk::file_format_base* format);
  virtual ~vsTrackOracleDescriptorArchiveSource();

private:
  QTE_DECLARE_PRIVATE(vsTrackOracleDescriptorArchiveSource)
  QTE_DISABLE_COPY(vsTrackOracleDescriptorArchiveSource)

  typedef vsArchiveSource<vsDescriptorSource> super;
};

#endif
