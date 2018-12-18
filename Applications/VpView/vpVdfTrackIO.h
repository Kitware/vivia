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

class vgAttributeSet;

class vpVdfIO;

class vpVdfTrackIOPrivate;

class vpVdfTrackIO : public vpTrackIO
{
public:
  vpVdfTrackIO(
    vpVdfIO* base, vtkVpTrackModel* trackModel,
    TrackStorageMode storageMode, bool interpolateToGround,
    TrackTimeStampMode timeStampMode, vtkVgTrackTypeRegistry* trackTypes,
    vgAttributeSet* trackAttributes, vtkMatrix4x4* geoTransform,
    vpFrameMap* frameMap);

  virtual ~vpVdfTrackIO();

  void SetTracksUri(const QUrl&);

  virtual bool ReadTracks(int frameOffset) QTE_OVERRIDE;
  virtual bool WriteTracks(const char*, int, QPointF, bool) const QTE_OVERRIDE
    { return false; }

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpVdfTrackIO);

  virtual unsigned int GetImageHeight() const;

private:
  QTE_DECLARE_PRIVATE(vpVdfTrackIO);
  QTE_DISABLE_COPY(vpVdfTrackIO);
};

#endif
