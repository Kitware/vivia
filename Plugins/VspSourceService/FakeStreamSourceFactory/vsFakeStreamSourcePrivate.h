// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsFakeStreamSourcePrivate_h
#define __vsFakeStreamSourcePrivate_h

#include <QHash>
#include <QMap>

#include <qtKstReader.h>

#include <vvTrack.h>

#include <vsTrackClassifier.h>
#include <vsTrackId.h>

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

  void trackUpdated(vsTrackId trackId, vvTrackState state);
  void trackClosed(vsTrackId trackId);

  void tocAvailable(vsTrackId trackId, vsTrackObjectClassifier toc);

protected slots:
  void releaseFrame();
  void sourceStatusChanged();
  void setAvailableFrames(QList<vtkVgVideoFrameMetaData>);
  void queueTrackUpdate(vsTrackId, vvTrackState);
  void queueTrackUpdate(vsTrackId, QList<vvTrackState>);
  void queueTrackClassifier(vsTrackId, vsTrackObjectClassifier);
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
  QHash<vsTrackId, TrackStateMap> TrackUpdates;
  QHash<vsTrackId, vsTrackObjectClassifier> Tocs;
  QMap<vtkVgTimeStamp, vsEvent> Events;
  vtkIdType NextEventId;

  double StreamRate, StreamJitter, StreamMaxBurstTime;

private:
  QTE_DISABLE_COPY(vsFakeStreamSourcePrivate)
};

#endif
