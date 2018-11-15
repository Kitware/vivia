/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvQueryVideoPlayer_h
#define __vvQueryVideoPlayer_h

#include "vvVideoPlayer.h"

#include <QHash>
#include <QList>
#include <QSet>
#include <QSharedPointer>
#include <QUrl>

#include <vtkType.h>

#include <vgExport.h>

#include <vtkVgTrack.h>

#include <vvQuery.h>

#include "vgRegionKeyframe.h"

class vtkIdList;
class vtkVgEvent;
class vtkVgEventBase;
class vtkVgEventRepresentationBase;

class vvQueryVideoPlayerPrivate;

class VV_VTKWIDGETS_EXPORT vvQueryVideoPlayer : public vvVideoPlayer
{
  Q_OBJECT

public:
  enum DisplayFlag
    {
    EventFlag       = 0x00001,
    KeyframeFlag    = 0x00002,
    UserFlag        = 0x10000
    };

  enum JumpDirection
    {
    JumpToStart,
    JumpToEnd
    };

  vvQueryVideoPlayer(QWidget* parent = 0);
  ~vvQueryVideoPlayer();

  void addEventRepresentation(
    vtkSmartPointer<vtkVgEventRepresentationBase>);

  void setDefaultEventColor(const QColor&);
  void setTrackTypeColor(vtkVgTrack::enumTrackFSOType type, const QColor&);

  QList<vtkIdType> trackIds();
  vvTrackId trackId(vtkIdType vtkId);

  QList<vtkIdType> trackEvents(vtkIdType trackId);
  QHash<vtkIdType, QSet<vtkIdType> > regionEvents();

  vtkVgEvent* event(vtkIdType eventId);
  vvDescriptor descriptor(vtkIdType eventId);

  vtkIdType descriptorEventId(const vvDescriptor& descriptor);

  bool eventVisibility(vtkIdType eventId);

  vtkVgTimeStamp currentTimeStamp();

  bool timeFromFrameNumber(vgTimeStamp& timeStamp);

  void jumpToTrack(vtkIdType trackId, JumpDirection direction);
  void jumpToEvent(vtkIdType eventId, JumpDirection direction);
  void jumpToTrack(vtkIdType trackId, const vtkVgTimeStamp& time);
  void jumpToEvent(vtkIdType eventId, const vtkVgTimeStamp& time);
  void jumpToKeyframe(vtkIdType keyframeId);

  virtual int videoHeight() = 0;

signals:
  void pickedKeyframe(vtkIdType keyframeId);

public slots:
  void setTracks(QList<vvTrack> tracks);
  void setDescriptors(QList<vvDescriptor> descriptors, bool haveTracks);
  void setKeyframes(QHash<vtkIdType, vgRegionKeyframe> keyframes);

  void refreshEvents();

  void setTrackVisibility(vtkIdType trackId, bool visibility);
  void setEventVisibility(vtkIdType eventId, bool visibility);
  void setKeyframeVisibility(bool visibility);

  void update();
  virtual void reset();

protected slots:
  void eventSelected(vtkIdType id);

  void updateScene();

protected:
  using QWidget::event; // don't shadow this with event(vtkIdType), above

  virtual void buildVideoModel() = 0;
  virtual void buildEventModel();
  virtual void buildTrackModel(bool incremental);
  virtual void buildScene();

  virtual void updateFrame();
  void finishJumpToTrack();

  void buildRegionMap();
  vtkSmartPointer<vtkVgEventBase> buildRegionEvent(bool matchKeyframes);

  void loadVideo();

protected:
  vvQueryVideoPlayerPrivate* Internal;
};

#endif
