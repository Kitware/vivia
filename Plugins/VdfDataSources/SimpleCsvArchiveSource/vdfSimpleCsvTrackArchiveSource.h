/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfSimpleCsvTrackDataSource_h
#define __vdfSimpleCsvTrackDataSource_h

#include <vdfThreadedArchiveSource.h>

template <typename T> class QSharedPointer;

class QFile;

class vdfSimpleCsvTrackDataSourcePrivate;

class vdfSimpleCsvTrackDataSource : public vdfThreadedArchiveSource
{
  Q_OBJECT

public:
  vdfSimpleCsvTrackDataSource(
    const QUrl& archiveUri, const QSharedPointer<QFile>& file,
    QObject* parent = nullptr);
  virtual ~vdfSimpleCsvTrackDataSource();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vdfSimpleCsvTrackDataSource)

  virtual bool processArchive(const QUrl& uri);

private:
  QTE_DECLARE_PRIVATE(vdfSimpleCsvTrackDataSource)
  QTE_DISABLE_COPY(vdfSimpleCsvTrackDataSource)
};

#endif
