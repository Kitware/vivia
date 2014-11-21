/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsFakeStreamSourcePrivate_h
#define __vsFakeStreamSourcePrivate_h

#include <QHash>
#include <QMap>

#include <qtKstReader.h>

#include <vvTrack.h>

#include <vsTrackClassifier.h>

#include <vsSimpleSourceFactory.h>
#include <vsStreamSourcePrivate.h>

#include "vsFakeStreamSource.h"

class vsVideoArchive;

class vsFakeStreamSourcePrivate : public vsStreamSourcePrivate
{
  Q_OBJECT

protected:
  QTE_DECLARE_PUBLIC(vsFakeStreamSource)

  vsFakeStreamSourcePrivate(vsFakeStreamSource* q, const QUrl& streamUri);
  virtual ~vsFakeStreamSourcePrivate();

  virtual void run() QTE_OVERRIDE;
  virtual void findTime(vtkVgTimeStamp* result, unsigned int frameNumber,
                        vg::SeekMode) QTE_OVERRIDE;
  virtual void requestFrame(const vgVideoSeekRequest&) QTE_OVERRIDE;
  virtual void clearLastRequest(vgVideoSourceRequestor*) QTE_OVERRIDE;

  QUrl relativeUri(QString path) const;
  void processProjectLine();
  void addArchiveSourceFactory(vsSimpleSourceFactoryPtr& factory);

  using vsStreamSourcePrivate::text;
  virtual QString text(QString format) const QTE_OVERRIDE;

signals:
  void streamingFinished();

  void trackUpdated(vvTrackId trackId, vvTrackState state);
  void trackClosed(vvTrackId trackId);

  void tocAvailable(vvTrackId trackId, vsTrackObjectClassifier toc);

protected slots:
  void releaseFrame();
  void sourceStatusChanged();
  void setAvailableFrames(QList<vtkVgVideoFrameMetaData>);
  void queueTrackUpdate(vvTrackId, vvTrackState);
  void queueTrackUpdate(vvTrackId, QList<vvTrackState>);
  void queueTrackClassifier(vvTrackId, vsTrackObjectClassifier);
  void queueEvent(vsDescriptorSource* source, vsEvent event);

  void flush(vtkVgTimeStamp = vtkVgTimeStamp());
  void flushData(vtkVgTimeStamp);

  void setStreamRate(double);
  void setStreamJitter(double);
  void setStreamMaxBurstTime(double);

protected:
  qtKstReader Project;

  QScopedPointer<vsVideoArchive> VideoArchive;
  QList<QSharedPointer<vsDataSource> > CurrentDataSources;

  QMap<vtkVgTimeStamp, vtkVgVideoFrameMetaData> VideoFrames;
  vtkVgTimeStamp LastFrameReleased;
  vtkVgTimeStamp NextFrame;

  typedef QMap<vtkVgTimeStamp, vvTrackState> TrackStateMap;
  QHash<vvTrackId, TrackStateMap> TrackUpdates;
  QHash<vvTrackId, vsTrackObjectClassifier> Tocs;
  QMap<vtkVgTimeStamp, vsEvent> Events;
  vtkIdType NextEventId;

  double StreamRate, StreamJitter, StreamMaxBurstTime;

private:
  QTE_DISABLE_COPY(vsFakeStreamSourcePrivate)
};

#endif
