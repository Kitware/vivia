/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsVideoSourcePrivate_h
#define __vsVideoSourcePrivate_h

#include <QHash>

#include <qtThread.h>

#include <vgExport.h>

#include <vgNamespace.h>

#include "vsVideoSource.h"

class vsVideoAnimation;

class VSP_DATA_EXPORT vsVideoSourcePrivate : public qtThread
{
  Q_OBJECT

public:
  virtual ~vsVideoSourcePrivate();

protected:
  QTE_DECLARE_PUBLIC_PTR(vsVideoSource)

  vsVideoSourcePrivate(vsVideoSource* q);

  virtual void run() QTE_OVERRIDE;

  virtual void requestFrame(const vgVideoSeekRequest&) = 0;

  vsDataSource::Status Status;
  bool Streaming;
  vtkVgTimeStamp FirstAvailableFrame;
  vtkVgTimeStamp LastAvailableFrame;
  QHash<QObject*, vgVideoSeekRequest> CurrentRequests;

protected slots:
  virtual void findTime(vtkVgTimeStamp* result, unsigned int frameNumber,
                        vg::SeekMode) = 0;
  virtual void queueFrameRequest(vgVideoSeekRequest);
  virtual void flushFrameRequests();
  virtual void clearLastRequest(vgVideoSourceRequestor*) = 0;

  void updateFrameRange(vtkVgTimeStamp first, vtkVgTimeStamp last);
  void emitMetadata(QList<vtkVgVideoFrameMetaData> metadata);

private:
  QTE_DECLARE_PUBLIC(vsVideoSource)
  QTE_DISABLE_COPY(vsVideoSourcePrivate)
};

#endif
