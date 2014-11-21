/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsScene_h
#define __vsScene_h

#include <QColor>
#include <QMatrix4x4>
#include <QObject>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vgVideoPlayer.h>
#include <vgVtkVideoFrame.h>

#include <vtkVgTrackRepresentationBase.h>

#include <vsContour.h>
#include <vsEventInfo.h>
#include <vsVideoSource.h>

#include "vsAlert.h"

class vgMixerWidget;

class vsAlertList;
class vsCore;
class vsEventTreeWidget;
class vsEventUserInfo;
class vsRegionList;
class vsTrackTreeWidget;

class vsScenePrivate;

class QVTKWidget;

class vtkVgEvent;
class vtkVgTrack;

class VSP_USERINTERFACE_EXPORT vsScene : public QObject
{
  Q_OBJECT

public:
  vsScene(vsCore*, QObject* parent = 0);
  virtual ~vsScene();

  virtual void setupUi(QVTKWidget*);
  virtual void setupFilterWidget(vgMixerWidget*);
  virtual void setupRegionList(vsRegionList*);
  virtual void setupAlertList(vsAlertList*);
  virtual void setupEventTree(
    vsEventTreeWidget* eventTree,
    vsEventTreeWidget* verifiedTree,
    vsEventTreeWidget* rejectedTree);
  virtual void setupTrackTree(vsTrackTreeWidget* tree);

  int contourState() const;

  double videoLiveOffset() const;
  double trackTrailLength() const;

  QColor eventSelectedColor() const;

  // TODO move below to UI plugin
  void setWriteRenderedImages(bool state, QString* outputDir = 0);
  void saveScreenShot(QString* filePath);

  bool setMaskImage(const QString& fileName);

  bool trackDisplayState(vtkIdType trackId);
  bool eventDisplayState(vtkIdType eventId);

  bool trackVisibility(vtkIdType trackId);
  bool eventVisibility(vtkIdType eventId);

  vtkVgTrackRepresentationBase::TrackColorMode trackColorMode();
  QString trackColorDataId();

  QList<vsEventUserInfo> eventList() const;

  qreal videoPlaybackSpeed() const;
  vgVideoPlayer::PlaybackMode videoPlaybackStatus() const;
  vtkVgTimeStamp currentVideoTime() const;

  QPointF viewToFrame(const QPointF&);
  vgGeocodedCoordinate viewToLatLon(const QPointF& in);

  QMatrix4x4 currentTransform() const;

  const vtkVgVideoFrameMetaData& currentFrameMetaData() const;

  vtkRenderer* renderer();

signals:
  void updated();
  void playbackStatusChanged(vgVideoPlayer::PlaybackMode, qreal rate);
  void videoMetadataUpdated(vtkVgVideoFrameMetaData, qint64 seekRequestId);
  void videoSeekRequestDiscarded(qint64 id,
                                 vtkVgTimeStamp lastFrameAvailable);

  void transformChanged(const QMatrix4x4& newTransform);

  void locationTextUpdated(QString location);

  void statusMessageAvailable(QString message);

  void interactionCanceled();

  void contourStarted();
  void contourClosed();
  void contourCompleted(vsContour);

  void drawingCanceled();
  void drawingEnded();

  void pickedEvent(vtkIdType);
  void pickedTrack(vtkIdType);

  void jumpToItem();

  void selectedEventsChanged(QSet<vtkIdType> selectedEventIds);
  void selectedEventsChanged(QList<vtkVgEvent*> selectedEvents);

  void trackSceneUpdated();
  void eventSceneUpdated();

  void alertThresholdChanged(int id, double);

public slots:
  void postUpdate();

  void resetView();
  void panTo(double x, double y);

  void jumpToEvent(vtkIdType eventId, bool jumpToEnd = false);
  void jumpToTrack(vtkIdType trackId, bool jumpToEnd = false);

  void setEventSelection(QSet<vtkIdType> eventIds, vtkIdType currentId = -1);

  void cancelInteraction();

  void beginDrawing(vsContour::Type type);
  void interruptDrawing();
  void setDrawingType(vsContour::Type);
  void closeContour();
  void beginContourManipulation();
  void finalizeContour();

  bool setContourEnabled(int, bool);
  bool convertContourToEvent(int, int eventType);

  void setEventStart(vtkIdType);
  void setEventEnd(vtkIdType);

  void setVideoPlaybackSpeed(vgVideoPlayer::PlaybackMode, qreal rate = 1.0);
  void seekVideo(vtkVgTimeStamp, vg::SeekMode, qint64);

  void setVideoLiveOffset(double);
  void setVideoSamplingMode(int mode);

  void setTracksVisible(bool);
  void setTrackBoxesVisible(bool);
  void setTrackIdsVisible(bool);
  void setTrackPvoScoresVisible(bool);
  void setTrackTrailLength(double);

  void setEventTracksVisible(bool);
  void setEventBoxesVisible(bool);
  void setEventLabelsVisible(bool);
  void setEventProbabilityVisible(bool);

  void showEventGroupFilter(vsEventInfo::Group);
  void hideEventGroupFilter(vsEventInfo::Group);
  void setEventGroupFilterVisible(vsEventInfo::Group, bool);

  void setEventGroupVisibility(vsEventInfo::Group, bool);
  void setEventGroupThreshold(vsEventInfo::Group, double);

  void setEventSelectedColor(QColor c);

  void setGroundTruthVisible(bool);

  void setFilteringMaskVisible(bool);
  void setTrackingMaskVisible(bool);

  void setEventStatus(vtkVgEvent*, int);

  void setFollowEvent(bool);

  void setTrackDisplayState(vtkIdType trackId, bool state);
  void setEventDisplayState(vtkIdType eventId, bool state);

  void setTrackColorMode(vtkVgTrackRepresentationBase::TrackColorMode,
                         const QString& dataId = QString());

  void updateTrackColors(vtkVgTrackRepresentationBase::TrackColorMode);

protected slots:
  void update();

  void resetVideo();
  void resetVideo(vsVideoSource*);
  void updateVideoFrame(vtkVgVideoFrame, qint64 seekRequestId);
  void removeVideoSource(QObject*);

  void setSourceStreaming(bool);
  void setSourceFrameRange(vtkVgTimeStamp, vtkVgTimeStamp);

  void setEventVisibility(int, bool);
  void setEventThreshold(int, double);

  void updateEventSelection(QSet<vtkIdType> eventIds);

  void onLeftClick();
  void onRightClick();
  void vtkSceneMouseMoveEvent();
  void vtkSceneLeaveEvent();

  void addContour(vsContour);
  void setContourType(int id, vsContour::Type);
  void setContourPoints(int id, QPolygonF);
  void removeContour(int id);

  void addUserEventType(int id, vsEventInfo, double initialThreshold);

  void addAlert(int id, vsAlert);
  void updateAlert(int id, vsAlert);
  void updateAlertThreshold(int id, vsAlert);
  void removeAlert(int id, bool unregister);

  void startQuickNoteTimer(vtkIdType eventId);

  void writeRenderedImages();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsScene)

  void postUpdate(vtkVgTimeStamp);

  void setupEventTree(vsEventTreeWidget* tree, int type);

  void jumpToEvent();
  void jumpToTrack();
  void jumpToTrack(vtkVgTrack*);

  vtkIdType tryPick(vtkVgRepresentationBase& labelRep,
                    vtkVgRepresentationBase& headRep,
                    vtkVgRepresentationBase& lineRep, int x, int y);

private:
  QTE_DECLARE_PRIVATE(vsScene)
  Q_DISABLE_COPY(vsScene)
};

#endif
