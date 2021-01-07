// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpVdfIO_h
#define __vpVdfIO_h

#include "vpModelIO.h"

#include <qtGlobal.h>

#include <QScopedPointer>

class QUrl;

class vpVdfIOPrivate;

class vpVdfIO : public vpModelIO
{
public:
  vpVdfIO();
  virtual ~vpVdfIO();

  virtual void SetTrackModel(vtkVpTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             bool interpolateToGround,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vgAttributeSet* trackAttributes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap) QTE_OVERRIDE;

  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes) QTE_OVERRIDE;

  virtual void SetActivityModel(vtkVgActivityManager* activityManager,
                                vpActivityConfig* activityConfig) QTE_OVERRIDE;

  virtual void SetImageHeight(unsigned int imageHeight) QTE_OVERRIDE;
  virtual unsigned int GetImageHeight() const QTE_OVERRIDE;

  void SetTracksUri(const QUrl& uri);
  void SetTrackTraitsFilePath(const QString& filePath);
  void SetTrackClassifiersFilePath(const QString& filePath);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpVdfIO);

private:
  QTE_DECLARE_PRIVATE(vpVdfIO);
  QTE_DISABLE_COPY(vpVdfIO);
};

#endif
