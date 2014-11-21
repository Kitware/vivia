/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackOracleTrackArchiveSource_h
#define __vsTrackOracleTrackArchiveSource_h

#include <vsTrackSource.h>

#include <vsArchiveSource.h>

namespace vidtk
{
  class file_format_base;
}

class vsTrackOracleTrackArchiveSourcePrivate;

class vsTrackOracleTrackArchiveSource : public vsArchiveSource<vsTrackSource>
{
  Q_OBJECT

public:
  vsTrackOracleTrackArchiveSource(const QUrl& archiveUri,
                                  vidtk::file_format_base* format);
  virtual ~vsTrackOracleTrackArchiveSource();

private:
  QTE_DECLARE_PRIVATE(vsTrackOracleTrackArchiveSource)
  QTE_DISABLE_COPY(vsTrackOracleTrackArchiveSource)

  typedef vsArchiveSource<vsTrackSource> super;
};

#endif
