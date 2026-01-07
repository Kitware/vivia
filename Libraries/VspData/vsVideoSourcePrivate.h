// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
