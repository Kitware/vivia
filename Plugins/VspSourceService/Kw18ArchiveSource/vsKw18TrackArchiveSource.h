// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
