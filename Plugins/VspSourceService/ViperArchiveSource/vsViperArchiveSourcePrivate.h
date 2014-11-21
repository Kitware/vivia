/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsViperArchiveSourcePrivate_h
#define __vsViperArchiveSourcePrivate_h

#include "vsViperArchiveSource.h"

#include <QMap>
#include <QSet>
#include <QUrl>

#include <qtThread.h>

#include <vvDescriptor.h>

#include <track_oracle/track_oracle.h>

#include <vsDescriptorInput.h>
#include <vsEvent.h>

class QRectF;

class vsViperArchiveTrackSource;

class vsViperArchiveSourcePrivate : public qtThread
{
  Q_OBJECT

public:
  vsViperArchiveSourcePrivate(vsViperArchiveSource* q, QUrl archiveUri);
  virtual ~vsViperArchiveSourcePrivate();

  QString text() const;
  QString toolTip(const QString& sourceTypeSingular,
                  const QString& sourceTypePlural) const;

signals:
  void eventAvailable(vsEvent);
  void eventRevoked(vtkIdType);

  void tocAvailable(vvTrackId trackId, vsTrackObjectClassifier toc);

public slots:
  void dispatch(QUrl metaDataSource, int frameOffset, double frameRate,
                bool importEvents);

protected slots:
  void injectInput(qint64 id, vsDescriptorInputPtr input);

protected:
  QTE_DECLARE_PUBLIC_PTR(vsViperArchiveSource)
  friend class vsViperArchiveTrackSource;

  struct ViperEvent
    {
    int Type;
    double Probability;
    uint StartFrame;
    uint EndFrame;
    };

  struct ViperTrack
    {
    vsTrackObjectClassifier Type;
    QMap<uint, QRectF> Region;
    QList<ViperEvent> Events;
    };

  struct Event
    {
    Event();

    QMap<uint, QRectF> OutstandingFrames;
    vvTrackId Track;
    vsEvent Object;
    bool Emitted;
    };

  typedef QHash<vvTrackId, ViperTrack> TrackMap;
  typedef TrackMap::iterator TrackIterator;

  vvTrackId mapTrackId(uint externalId, const QString& type,
                       vvTrackId& newId);

  virtual void run() QTE_OVERRIDE;

  void processExternalTimingInfo(const QUrl& source);

  bool resolve(vtkVgTimeStamp fmdTimeStamp);
  void updateEvent(vtkIdType eventId, vtkVgTimeStamp fmdTimeStamp);
  void checkClosures();
  void activateClosureTimer();

  void updateStatus(vsDataSource::Status);
  void suicide();

  virtual void timerEvent(QTimerEvent*) QTE_OVERRIDE;

  const QUrl ArchiveUri;

  vsDataSource::Status Status;

  int StatusTimerId;
  int StatusTtl;
  bool StatusResetNeeded;

  QSharedPointer<vsViperArchiveTrackSource> TrackSource;

  TrackMap ViperTracks;
  QHash<QString, QHash<uint, vvTrackId> > ViperTrackIdMap;

  QHash<vtkIdType, Event> Events;

  vtkVgTimeStamp LastTime;
  int ClosureTimerId;
  QSet<vvTrackId> OpenTracks;
  QHash<vtkIdType, int> OverdueEvents;

  QHash<uint, QList<vvTrackId> > PendingTrackFrames;
  QHash<uint, QList<vtkIdType> > PendingEventFrames;

private:
  QTE_DECLARE_PUBLIC(vsViperArchiveSource)
  QTE_DISABLE_COPY(vsViperArchiveSourcePrivate)
};

#endif
