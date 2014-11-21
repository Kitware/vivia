/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsMainWindowPrivate_h
#define __vsMainWindowPrivate_h

#include <QHash>
#include <QStack>

#include <qtUiState.h>

#include <vgRange.h>

#include <vsContour.h>
#include <vsDescriptorInput.h>

#include "vsMainWindow.h"

#include "ui_vsp.h"
#include "am_vsp.h"

class QComboBox;
class QLabel;
class QSignalMapper;

class qtDockController;

class pqCoreTestUtility;

class vsScene;

class vsMainWindow;

class vsMainWindowPrivate : public QObject
{
  Q_OBJECT

public:
  enum StatusSlot
    {
    StatusFrameGsd,
    StatusFrameTimestamp,
    StatusCursorLocation,
    StatusVideoIcon,
    StatusVideoText,
    StatusTrackIcon,
    StatusTrackText,
    StatusDescriptorIcon,
    StatusDescriptorText,
    StatusText
    };

  explicit vsMainWindowPrivate(vsMainWindow* q);
  virtual ~vsMainWindowPrivate();

  void setupStatusBar();
  void setStatusVisible(StatusSlot slot, bool visible);
  void setStatusText(StatusSlot slot, const QString& text);

  vsContour::Type currentDrawingType() const;
  int resampleMode(QAction* activeMode) const;
  QAction* resampleAction(int resampleMode);

  Ui::MainWindow UI;
  Am::MainWindow AM;

public slots:
  void enableInputs(vsDescriptorInput::Types);

  void enableRegionClose();
  void disableRegionClose();
  void completeDrawing();
  void cancelDrawing();
  void updateDrawingType();

  void updateFrameSpinBox();

protected:
  QTE_DECLARE_PUBLIC_PTR(vsMainWindow)

  void createStatusLabel(StatusSlot slot, Qt::Alignment alignment,
                         bool permanent = false, int stretch = 0);
  void createSourceStatusLabels(StatusSlot iconSlot, StatusSlot textSlot);

  void setSourceStatus(
    StatusSlot iconSlot, StatusSlot textSlot,
    const QString& icon, QList<vsDataSource::Status> status,
    const QStringList& text, const QStringList& toolTip);

  void setVideoPlaybackSlow(qreal direction);
  void setVideoPlaybackFast(qreal direction);
  void setVideoPlaybackSpeed(qreal rate);

  void updatePlaybackControlsState(vgTimeStamp currentVideoTime);

  static QString iconFromStatus(const QString& baseIcon,
                                vsDataSource::Status status);
  static QString iconHtmlFromStatus(const QString& baseIcon,
                                    vsDataSource::Status status);

  typedef QStack<QCursor> CursorStack;

  qtUiState UiState;

  vsCore* Core;
  vsScene* Scene;
  vsVideoSource* VideoSource;
  QSignalMapper* VideoSamplingMapper;
  QSignalMapper* SourceMapper;
  pqCoreTestUtility* TestUtility;

  QHash<StatusSlot, QLabel*> StatusLabels;
  QComboBox* RegionType;
  qtDockController* DockController;

  QScopedPointer<qtPrioritizedMenuProxy> ToolsMenu;
  QScopedPointer<qtPrioritizedToolBarProxy> ToolsToolBar;

  QList<QObject*> ViewCursorOwners;
  QMap<QObject*, CursorStack> ViewCursorStacks;

  bool IgnoreCancelDrawing;

  vgVideoPlayer::PlaybackMode VideoPlaybackMode;
  qreal VideoPlaybackRate;
  qreal OldVideoPlaybackRate;

  bool OldVideoPlaybackLive;

  vtkVgTimeStamp PendingSeek;

  vgRange<vgTimeStamp> AvailableVideoRange;

  bool BlockFrameUpdates;
  long long NextSeekRequestId;
  long long LastScrubberSeek;
  long long LastSpinBoxSeek;

  QEventLoop SeekFlushLoop;

private:
  QTE_DECLARE_PUBLIC(vsMainWindow)
  Q_DISABLE_COPY(vsMainWindowPrivate)
};

#endif
