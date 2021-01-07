// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsVideoSource_h
#define __vsVideoSource_h

#include <QAbstractAnimation>

#include <vgExport.h>

#include <vgNamespace.h>
#include <vgRange.h>

#include <vgVideoSource.h>

#include <vtkVgTimeStamp.h>

#include <vtkVgVideoFrameMetaData.h>

#include "vsDataSource.h"

class vsVideoSourcePrivate;

class VSP_DATA_EXPORT vsVideoSource : public vsDataSource, public vgVideoSource
{
  Q_OBJECT

public:
  virtual ~vsVideoSource();

  virtual void start() QTE_OVERRIDE;

  vsDataSource::Status status() const QTE_OVERRIDE;

  virtual bool isStreaming() const;
  vgRange<vtkVgTimeStamp> frameRange() const;

  virtual vtkVgTimeStamp findTime(unsigned int frameNumber, vg::SeekMode);
  virtual void requestFrame(vgVideoSeekRequest);
  virtual void clearLastRequest(vgVideoSourceRequestor*);

signals:
  void frameRangeAvailable(vtkVgTimeStamp first, vtkVgTimeStamp last);
  void metadataAvailable(QList<vtkVgVideoFrameMetaData> metadata);
  void streamingChanged(bool);

protected slots:
  virtual void setStreaming(bool);
  virtual void setFrameRange(vtkVgTimeStamp first, vtkVgTimeStamp last);
  virtual void updateStatus(vsDataSource::Status);

protected:
  QTE_DECLARE_PRIVATE_PTR(vsVideoSource)

  vsVideoSource(vsVideoSourcePrivate*);

private:
  QTE_DECLARE_PRIVATE(vsVideoSource)
  QTE_DISABLE_COPY(vsVideoSource)
};

#endif
