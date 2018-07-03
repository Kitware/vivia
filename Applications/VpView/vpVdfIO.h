/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVdfIO_h
#define __vpVdfIO_h

#include "vpModelIO.h"

#include <qtGlobal.h>

#include <QUrl>

class vpVdfIOPrivate;

class vpVdfIO : public vpModelIO
{
public:
  vpVdfIO();
  virtual ~vpVdfIO();

  virtual void SetTrackModel(vtkVpTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vtkMatrix4x4* geoTransform,
                             vpFileDataSource* imageDataSource,
                             vpFrameMap* frameMap) QTE_OVERRIDE;

  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes) QTE_OVERRIDE;

  virtual void SetActivityModel(vtkVgActivityManager* activityManager,
                                vpActivityConfig* activityConfig) QTE_OVERRIDE;

  virtual void SetImageHeight(unsigned int imageHeight) QTE_OVERRIDE;
  virtual unsigned int GetImageHeight() const QTE_OVERRIDE;

  void SetTracksUri(const QUrl& uri);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpVdfIO);

private:
  QTE_DECLARE_PRIVATE(vpVdfIO);
  QTE_DISABLE_COPY(vpVdfIO);
};

#endif
