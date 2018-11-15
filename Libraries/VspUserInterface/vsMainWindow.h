/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsMainWindow_h
#define __vsMainWindow_h

#include <QMainWindow>

#include <vgExport.h>

#include <vtkVgVideoFrameMetaData.h>

#include <vgVideoPlayer.h>

#include <vsDataSource.h>
#include <vsDescriptorInput.h>

class QVTKWidget;

class qtCliArgs;
class qtPrioritizedMenuProxy;
class qtPrioritizedToolBarProxy;

class vsCore;
class vsVideoSource;

class vsMainWindowPrivate;

class VSP_USERINTERFACE_EXPORT vsMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit vsMainWindow(vsCore* core, vsMainWindow* invokingView = 0);
  virtual ~vsMainWindow();

  void setPendingSeek(const vtkVgTimeStamp&);

  qtPrioritizedMenuProxy* toolsMenu();
  qtPrioritizedToolBarProxy* toolsToolBar();

  QVTKWidget* view();

  void pushViewCursor(const QCursor&, QObject* owner);
  void popViewCursor(QObject* owner);

  using QMainWindow::addDockWidget;
  void addDockWidget(QDockWidget* dock, Qt::DockWidgetArea area,
                     QAction* toggleDockAction);

public slots:
  void createSource(QString identifier);

  void loadFilterSettings(QString fileName = QString());
  void saveFilterSettings(QString fileName = QString());

  void loadMaskImage(QString fileName);

  void setStatusText(QString);

  void setVideoPlaybackLive();
  void setVideoPlaybackNormal();
  void setVideoPlaybackPaused();
  void setVideoPlaybackResumed();
  void setVideoPlaybackStopped();
  void setVideoPlaybackReversed();
  void setVideoPlaybackFastForward();
  void setVideoPlaybackFastBackward();
  void stepVideoForward();
  void stepVideoBackward();
  void skipVideoForward();
  void skipVideoBackward();

  void decreaseVideoPlaybackSpeed();
  void increaseVideoPlaybackSpeed();

  void writeRenderedImages(bool);
  void saveScreenShot();

  void initializeTesting(const qtCliArgs*);

protected slots:
  void toggleDrawing(bool);

  void setCursorLocationText(QString);

  void updateTrackLabelActionsEnabled();

  void seekVideoFrame(int, vg::SeekMode);
  void seekVideo(vgTimeStamp, vg::SeekMode);

  void stepVideo(int stepBy);
  void skipVideo(double skipBy);

  void flushPendingSeeks(int timeoutMilliseconds);

  void setPlaybackStatus(vgVideoPlayer::PlaybackMode, qreal);
  void setVideoSourceStatus(vsDataSource::Status);
  void setTrackSourceStatus(vsDataSource::Status);
  void setDescriptorSourceStatus(vsDataSource::Status);

  void updateVideoMetadata(vtkVgVideoFrameMetaData, qint64 seekRequestId);
  void updateVideoPosition(qint64 seekRequestId, vtkVgTimeStamp position);

  void changeVideoSource(vsVideoSource*);
  void resetVideo();
  void updateVideoAvailableTimeRange(vtkVgTimeStamp first, vtkVgTimeStamp last);

  void setVideoLiveOffset();
  void setTrackColoring();
  void setTrackTrailLength();
  void setEventDisplayThreshold();
  void showEventsAllFish();
  void showEventsAllScallop();
  void hideEventsAllFish();
  void hideEventsAllScallop();

  void createAlert();
  void loadAlert();

  void formulateQuery();

protected:
  QTE_DECLARE_PRIVATE_PTR(vsMainWindow)

  virtual void closeEvent(QCloseEvent*);

private:
  QTE_DECLARE_PRIVATE(vsMainWindow)
  Q_DISABLE_COPY(vsMainWindow)
};

#endif
