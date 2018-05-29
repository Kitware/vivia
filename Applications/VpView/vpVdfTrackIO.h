/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVdfTrackIO_h
#define __vpVdfTrackIO_h

#include "vpTrackIO.h"

#include <qtGlobal.h>

#include <QScopedPointer>

class QUrl;

class vpVdfIO;

class vpVdfTrackIOPrivate;

class vpVdfTrackIO : public vpTrackIO
{
public:
  vpVdfTrackIO(
    vpVdfIO* base, vtkVpTrackModel* trackModel,
    TrackStorageMode storageMode, TrackTimeStampMode timeStampMode,
    vtkVgTrackTypeRegistry* trackTypes, vtkMatrix4x4* geoTransform,
    vpFrameMap* frameMap);

  virtual ~vpVdfTrackIO();

  void SetTracksUri(const QUrl&);

  virtual bool ReadTracks() QTE_OVERRIDE;
  virtual bool WriteTracks(const QString&, bool) const QTE_OVERRIDE
    { return false; }

  virtual QStringList GetSupportedFormats() const QTE_OVERRIDE;
  virtual QString GetDefaultFormat() const QTE_OVERRIDE;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpVdfTrackIO);

  virtual unsigned int GetImageHeight() const;

private:
  QTE_DECLARE_PRIVATE(vpVdfTrackIO);
  QTE_DISABLE_COPY(vpVdfTrackIO);
};

#endif
