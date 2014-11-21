/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsKw18TrackArchiveSource_h
#define __vsKw18TrackArchiveSource_h

#include <QUrl>

#include <vsTrackSource.h>

#include <vsArchiveSource.h>

class vsKw18TrackArchiveSourcePrivate;

class vsKw18TrackArchiveSource : public vsArchiveSource<vsTrackSource>
{
  Q_OBJECT

public:
  vsKw18TrackArchiveSource(const QUrl& archiveUri);
  virtual ~vsKw18TrackArchiveSource();

private:
  QTE_DECLARE_PRIVATE(vsKw18TrackArchiveSource)
  QTE_DISABLE_COPY(vsKw18TrackArchiveSource)

  typedef vsArchiveSource<vsTrackSource> super;
};

#endif
