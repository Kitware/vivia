/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfTrackOracleTrackDataSource_h
#define __vdfTrackOracleTrackDataSource_h

#include <vdfThreadedArchiveSource.h>

namespace vidtk
{
  class file_format_base;
}

class vdfTrackOracleTrackDataSourcePrivate;

class vdfTrackOracleTrackDataSource : public vdfThreadedArchiveSource
{
  Q_OBJECT

public:
  vdfTrackOracleTrackDataSource(
    const QUrl& archiveUri, vidtk::file_format_base* format,
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
