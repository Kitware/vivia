/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsMainWindow.h"
#include "vsMainWindowPrivate.h"

#include <QComboBox>
#include <QFileInfo>
#include <QMessageBox>
#include <QSignalMapper>
#include <QTextStream>
#include <QTimer>

#ifdef ENABLE_QTTESTING
#include <pqCoreTestUtility.h>
#endif

#include <qtDockController.h>
#include <qtDoubleInputDialog.h>
#include <qtKstReader.h>
#include <qtPrioritizedToolBarProxy.h>
#include <qtScopedValueChange.h>
#include <qtUtil.h>

#include <vgCheckArg.h>

#include <vgFileDialog.h>
#include <vgUnixTime.h>

#include <vgAboutAction.h>

#include <vtkVgVideoMetadata.h>

#include <vvMakeId.h>
#include <vvWriter.h>

#include <vsSourceFactory.h>
#include <vsSourceService.h>

#include "vsAlertEditor.h"
#include "vsApplication.h"
#include "vsCore.h"
#include "vsQfDialog.h"
#include "vsQfVideoSource.h"
#include "vsScene.h"
#include "vsSettings.h"
#include "vsTrackColorDialog.h"
#include "vsTrackInfo.h"
#include "vsUiExtensionInterface.h"

QTE_IMPLEMENT_D_FUNC(vsMainWindow)

//BEGIN miscellaneous helper functions

namespace // anonymous
{

#define connectSceneDisplayToggle(_a_, _s_) do { \
  connect(_a_, SIGNAL(toggled(bool)), scene, SLOT(_s_(bool))); \
  scene->_s_(_a_->isChecked()); \
  } while (0)

//-----------------------------------------------------------------------------
void mapIntAction(QSignalMapper* mapper, QAction* action, int id)
{
  mapper->setMapping(action, id);
  action->connect(action, SIGNAL(toggled(bool)), mapper, SLOT(map()));
}

//-----------------------------------------------------------------------------
void copyBoolOption(QAction* from, QAction* to)
{
  to->setChecked(from->isChecked());
}

//-----------------------------------------------------------------------------
void copyBoolOption(QCheckBox* from, QCheckBox* to)
{
  to->setChecked(from->isChecked());
}

QString makeGsdText(double gsd, double unitThreshold)
{
  static const QString invalid = "(invalid)";

  // Negative GSD is invalid; also 'conservatively' assume it is invalid if
  // more than 10,000 km/px (in which case the entire planet would be just over
  // 1 px)
  if (gsd >= 0.0 && gsd < 1e7)
    {
    return (gsd > unitThreshold
            ? QString().sprintf("%.3f m/px", gsd)
            : QString().sprintf("%.1f cm/px", gsd * 100.0));
    }
  return invalid;
}

} // namespace <anonymous>

//END miscellaneous helper functions

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsMainWindow

//-----------------------------------------------------------------------------
vsMainWindow::vsMainWindow(vsCore* core, vsMainWindow* invokingView)
  : d_ptr(new vsMainWindowPrivate(this))
{
  QTE_D(vsMainWindow);

  d->Core = core;
  vsScene* scene = d->Scene = new vsScene(core, this);

  // Set up UI
  d->UI.setupUi(this);
  d->AM.setupActions(d->UI, this);
  connect(d->UI.actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

  this->pushViewCursor(d->UI.renderFrame->cursor(), 0);

  scene->setupUi(d->UI.renderFrame);
  scene->setupFilterWidget(d->UI.filter);
  scene->setupRegionList(d->UI.regionTree);
  scene->setupAlertList(d->UI.alertTree);
  scene->setupEventTree(d->UI.eventTree,
                        d->UI.verifiedEventTree,
                        d->UI.rejectedEventTree);
  scene->setupTrackTree(d->UI.trackTree);

  d->UI.noteTree->connectToScene(scene);
  d->UI.noteTree->setModel(scene->eventDataModel());

  d->UI.regionTree->addContextMenuAction(d->UI.actionRegionRemoveSelected);
  d->UI.regionTree->setEventTypes(d->Core->manualEventTypes());
  connect(d->UI.regionTree, SIGNAL(selectionStatusChanged(bool)),
          d->UI.actionRegionRemoveSelected, SLOT(setEnabled(bool)));

  d->UI.eventInfo->setEventTypeRegistry(core->eventTypeRegistry());
  connect(scene, SIGNAL(selectedEventsChanged(QList<vtkVgEvent*>)),
          d->UI.eventInfo, SLOT(setEvents(QList<vtkVgEvent*>)));
  connect(core, SIGNAL(eventChanged(vtkVgEvent*)),
          d->UI.eventInfo, SLOT(updateEvent(vtkVgEvent*)));
  connect(core, SIGNAL(eventRatingChanged(vtkVgEvent*, int)),
          d->UI.eventInfo, SLOT(updateEvent(vtkVgEvent*)));
  connect(core, SIGNAL(eventNoteChanged(vtkVgEvent*, QString)),
          d->UI.eventInfo, SLOT(updateEvent(vtkVgEvent*)));

  // Create placeholders for tools menu and tool bar, and add built-in tools
  d->ToolsMenu.reset(new qtPrioritizedMenuProxy(d->UI.menuTools));
  d->ToolsToolBar.reset(new qtPrioritizedToolBarProxy(d->UI.toolsToolBar));
  d->ToolsMenu->insertAction(d->UI.actionFormulateQuery, 400);
  d->ToolsToolBar->insertAction(d->UI.actionFormulateQuery, 400);

  // Set window title (i.e. add version number) and icon
  QString title("%1 %3");
  const QString version = QApplication::applicationVersion();
  title = title.arg(this->windowTitle()).arg(version);
  this->setWindowTitle(title);
  qtUtil::setApplicationIcon("vsPlay", this);

  // Set up Help menu
  vgApplication::setupHelpMenu(d->UI.menuHelp);

  // Set up docks
  d->UI.filterDock->hide();
  d->UI.regionDock->hide();
  d->UI.eventsDock->hide();
  d->UI.verifiedEventsDock->hide();
  d->UI.rejectedEventsDock->hide();
  d->UI.eventInfoDock->hide();
  d->UI.notesDock->hide();
  d->DockController = new qtDockController(this);

  d->DockController->addToggleAction(
    d->UI.actionWindowFilterShow,
    d->UI.filterDock);
  d->DockController->addToggleAction(
    d->UI.actionWindowRegionShow,
    d->UI.regionDock);
  d->DockController->addToggleAction(
    d->UI.actionWindowAlertShow,
    d->UI.alertDock);
  d->DockController->addToggleAction(
    d->UI.actionWindowEventsShow,
    d->UI.eventsDock);
  d->DockController->addToggleAction(
    d->UI.actionWindowVerifiedEventsShow,
    d->UI.verifiedEventsDock);
  d->DockController->addToggleAction(
    d->UI.actionWindowRejectedEventsShow,
    d->UI.rejectedEventsDock);
  d->DockController->addToggleAction(
    d->UI.actionWindowEventInfoShow,
    d->UI.eventInfoDock);
  d->DockController->addToggleAction(
    d->UI.actionWindowTracksShow,
    d->UI.tracksDock);
  d->DockController->addToggleAction(
    d->UI.actionWindowNoteShow,
    d->UI.notesDock);

  // Set up status bar
  d->setupStatusBar();

  this->setVideoSourceStatus(vsDataSource::NoSource);
  this->setTrackSourceStatus(vsDataSource::NoSource);
  this->setDescriptorSourceStatus(vsDataSource::NoSource);

  // Set up video scrubber
  d->UI.scrubber->setRange(vgTimeStamp::fromTime(-1.0),
                           vgTimeStamp::fromTime(-1.0));
  d->UI.scrubber->setSingleStep(vgTimeStamp(1e4, 1u));
  d->UI.scrubber->setWheelStep(vgTimeStamp(0.0875e6, 1u));
  d->UI.scrubber->setPageStep(vgTimeStamp(1e6 /* 1 second */, 10u));

  // Set up region related UI
  d->RegionType = new QComboBox(d->UI.drawingToolBar);
  d->RegionType->setObjectName("regionType");
  vsContour::populateTypeWidget(d->RegionType);
  d->UI.drawingToolBar->insertWidget(d->UI.actionRegionClose, d->RegionType);
  connect(d->RegionType, SIGNAL(currentIndexChanged(int)),
          d, SLOT(updateDrawingType()));
  d->updateDrawingType();

  d->UI.regionTree->sortItems(0, Qt::AscendingOrder);

  // Set up source factory actions
  d->SourceMapper = new QSignalMapper(this);
  qtPrioritizedMenuProxy videoMenu(d->UI.menuVideo, d->UI.actionQuit,
                                   qtUtil::AddSeparatorAfter);
  qtPrioritizedMenuProxy trackMenu(d->UI.menuTracks);
  qtPrioritizedMenuProxy descriptorMenu(d->UI.menuDescriptors);
  vsSourceService::createActions(this, d->SourceMapper,
                                 videoMenu, trackMenu, descriptorMenu);
  connect(d->SourceMapper, SIGNAL(mapped(QString)),
          this, SLOT(createSource(QString)));

  // Connect to scene inputs
  connect(d->UI.scrubber, SIGNAL(valueChanged(vgTimeStamp, vg::SeekMode)),
          this, SLOT(seekVideo(vgTimeStamp, vg::SeekMode)));
  connect(d->UI.actionViewReset, SIGNAL(triggered()),
          scene, SLOT(resetView()));

  d->VideoSamplingMapper = new QSignalMapper(this);
  connect(d->VideoSamplingMapper, SIGNAL(mapped(int)),
          d->Scene, SLOT(setVideoSamplingMode(int)));
  mapIntAction(d->VideoSamplingMapper, d->UI.actionVideoSampleNearest, 0);
  mapIntAction(d->VideoSamplingMapper, d->UI.actionVideoSampleLinear, 1);
  mapIntAction(d->VideoSamplingMapper, d->UI.actionVideoSampleBicubic, 2);

  d->Scene->setVideoSamplingMode(
    d->resampleMode(d->AM.videoSampling->checkedAction()));

  connectSceneDisplayToggle(d->UI.actionTracksShow,
                            setTracksVisible);
  connectSceneDisplayToggle(d->UI.actionTracksShowBoxes,
                            setTrackBoxesVisible);
  connectSceneDisplayToggle(d->UI.actionTracksShowId,
                            setTrackIdsVisible);
  connectSceneDisplayToggle(d->UI.actionTracksShowPvo,
                            setTrackPvoScoresVisible);
  connectSceneDisplayToggle(d->UI.actionEventsTracksShow,
                            setEventTracksVisible);
  connectSceneDisplayToggle(d->UI.actionEventsBoxesShow,
                            setEventBoxesVisible);
  connectSceneDisplayToggle(d->UI.actionEventsLabelsShow,
                            setEventLabelsVisible);
  connectSceneDisplayToggle(d->UI.actionEventsProbabilityShow,
                            setEventProbabilityVisible);
  connectSceneDisplayToggle(d->UI.actionNotesShow,
                            setNotesVisible);
  connectSceneDisplayToggle(d->UI.groundTruthVisible,
                            setGroundTruthVisible);
  connectSceneDisplayToggle(d->UI.actionVideoMaskShowTracking,
                            setTrackingMaskVisible);
  connectSceneDisplayToggle(d->UI.actionVideoMaskShowFiltering,
                            setFilteringMaskVisible);
  connectSceneDisplayToggle(d->UI.actionViewFocusOnTarget,
                            setFocusOnTarget);

  connect(d->UI.actionRegionClose, SIGNAL(triggered()),
          scene, SLOT(closeContour()));
  connect(d->UI.actionCancelFollowing, SIGNAL(triggered()),
          core, SLOT(cancelFollowing()));

  // Connect correlated display toggles
  connect(d->UI.actionTracksShow, SIGNAL(toggled(bool)),
          this, SLOT(updateTrackLabelActionsEnabled()));
  connect(d->UI.actionTracksShowBoxes, SIGNAL(toggled(bool)),
          this, SLOT(updateTrackLabelActionsEnabled()));

  // Connect to scene outputs
  connect(scene, SIGNAL(updated()), d->UI.renderFrame, SLOT(update()));
  connect(scene, SIGNAL(videoSeekRequestDiscarded(qint64, vtkVgTimeStamp)),
          this, SLOT(updateVideoPosition(qint64, vtkVgTimeStamp)));
  connect(scene, SIGNAL(videoMetadataUpdated(vtkVgVideoFrameMetaData, qint64)),
          this, SLOT(updateVideoMetadata(vtkVgVideoFrameMetaData, qint64)));
  connect(scene, SIGNAL(playbackStatusChanged(vgVideoPlayer::PlaybackMode, qreal)),
          this, SLOT(setPlaybackStatus(vgVideoPlayer::PlaybackMode, qreal)));
  connect(scene, SIGNAL(locationTextUpdated(QString)),
          this, SLOT(setCursorLocationText(QString)));
  connect(scene, SIGNAL(statusMessageAvailable(QString)),
          this, SLOT(setStatusText(QString)));
  connect(scene, SIGNAL(contourStarted()), d, SLOT(enableRegionClose()));
  connect(scene, SIGNAL(contourClosed()), d, SLOT(disableRegionClose()));
  connect(scene, SIGNAL(drawingEnded()), d, SLOT(completeDrawing()));
  connect(scene, SIGNAL(interactionCanceled()), d, SLOT(cancelDrawing()));

  // Connect to core outputs
  connect(core, SIGNAL(statusMessageAvailable(QString)),
          this, SLOT(setStatusText(QString)));
  connect(core, SIGNAL(videoSourceChanged(vsVideoSource*)),
          this, SLOT(changeVideoSource(vsVideoSource*)));
  connect(core, SIGNAL(videoSourceChanged(vsVideoSource*)),
          this, SLOT(resetVideo()));
  connect(core, SIGNAL(videoTimeRangeAvailableUpdated(vtkVgTimeStamp,
                                                      vtkVgTimeStamp)),
          this, SLOT(updateVideoAvailableTimeRange(vtkVgTimeStamp,
                                                   vtkVgTimeStamp)));
  connect(core, SIGNAL(videoSourceStatusChanged(vsDataSource::Status)),
          this, SLOT(setVideoSourceStatus(vsDataSource::Status)));
  connect(core, SIGNAL(trackSourceStatusChanged(vsDataSource::Status)),
          this, SLOT(setTrackSourceStatus(vsDataSource::Status)));
  connect(core, SIGNAL(descriptorSourceStatusChanged(vsDataSource::Status)),
          this, SLOT(setDescriptorSourceStatus(vsDataSource::Status)));
  connect(core, SIGNAL(acceptedInputsChanged(vsDescriptorInput::Types)),
          d, SLOT(enableInputs(vsDescriptorInput::Types)));
  d->enableInputs(core->acceptedInputs());

  // Connect playback actions
  connect(d->UI.actionVideoPlayLive, SIGNAL(triggered()),
          this, SLOT(setVideoPlaybackLive()));
  connect(d->UI.actionVideoPlay, SIGNAL(triggered()),
          this, SLOT(setVideoPlaybackNormal()));
  connect(d->UI.actionVideoPause, SIGNAL(triggered()),
          this, SLOT(setVideoPlaybackPaused()));
  connect(d->UI.actionVideoResume, SIGNAL(triggered()),
          this, SLOT(setVideoPlaybackResumed()));
  connect(d->UI.actionVideoStop, SIGNAL(triggered()),
          this, SLOT(setVideoPlaybackStopped()));
  connect(d->UI.actionVideoPlayReversed, SIGNAL(triggered()),
          this, SLOT(setVideoPlaybackReversed()));
  connect(d->UI.actionVideoFastForward, SIGNAL(triggered()),
          this, SLOT(setVideoPlaybackFastForward()));
  connect(d->UI.actionVideoFastBackward, SIGNAL(triggered()),
          this, SLOT(setVideoPlaybackFastBackward()));
  connect(d->UI.actionVideoSpeedDecrease, SIGNAL(triggered()),
          this, SLOT(decreaseVideoPlaybackSpeed()));
  connect(d->UI.actionVideoSpeedIncrease, SIGNAL(triggered()),
          this, SLOT(increaseVideoPlaybackSpeed()));
  connect(d->UI.actionVideoFrameBackward, SIGNAL(triggered()),
          this, SLOT(stepVideoBackward()));
  connect(d->UI.actionVideoFrameForward, SIGNAL(triggered()),
          this, SLOT(stepVideoForward()));
  connect(d->UI.actionVideoSkipBackward, SIGNAL(triggered()),
          this, SLOT(skipVideoBackward()));
  connect(d->UI.actionVideoSkipForward, SIGNAL(triggered()),
          this, SLOT(skipVideoForward()));
  d->VideoPlaybackMode = vgVideoPlayer::Stopped;
  d->VideoPlaybackRate = 0.0;
  this->setPlaybackStatus(vgVideoPlayer::Stopped, 0.0);

  connect(d->UI.frame, SIGNAL(valueChanged(int, vg::SeekMode)),
          this, SLOT(seekVideoFrame(int, vg::SeekMode)));
  connect(d->UI.frame, SIGNAL(editingFinished()),
          d, SLOT(updateFrameSpinBox()));

  // Connect other internal actions
  connect(d->UI.actionVideoLiveOffsetSet, SIGNAL(triggered()),
          this, SLOT(setVideoLiveOffset()));
  connect(d->UI.actionTracksColor, SIGNAL(triggered()),
          this, SLOT(setTrackColoring()));
  connect(d->UI.actionTracksTrailLengthSet, SIGNAL(triggered()),
          this, SLOT(setTrackTrailLength()));
  connect(d->UI.actionEventFiltersLoad, SIGNAL(triggered()),
          this, SLOT(loadFilterSettings()));
  connect(d->UI.actionEventFiltersSave, SIGNAL(triggered()),
          this, SLOT(saveFilterSettings()));
  connect(d->UI.actionEventProbabilityThresholdSet, SIGNAL(triggered()),
          this, SLOT(setEventDisplayThreshold()));
  connect(d->UI.actionEventsPersonEnableAll, SIGNAL(triggered()),
          this, SLOT(showEventsAllPerson()));
  connect(d->UI.actionEventsVehicleEnableAll, SIGNAL(triggered()),
          this, SLOT(showEventsAllVehicle()));
  connect(d->UI.actionEventsPersonDisableAll, SIGNAL(triggered()),
          this, SLOT(hideEventsAllPerson()));
  connect(d->UI.actionEventsVehicleDisableAll, SIGNAL(triggered()),
          this, SLOT(hideEventsAllVehicle()));
  connect(d->UI.actionRegionDraw, SIGNAL(toggled(bool)),
          this, SLOT(toggleDrawing(bool)));
  connect(d->UI.actionAlertCreate, SIGNAL(triggered()),
          this, SLOT(createAlert()));
  connect(d->UI.actionAlertLoad, SIGNAL(triggered()),
          this, SLOT(loadAlert()));
  connect(d->UI.actionFormulateQuery, SIGNAL(triggered()),
          this, SLOT(formulateQuery()));

  connect(d->UI.actionWriteRenderedImages, SIGNAL(triggered(bool)),
          this, SLOT(writeRenderedImages(bool)));
  connect(d->UI.actionSaveScreenshot, SIGNAL(triggered()),
          this, SLOT(saveScreenShot()));

  // Set up UI extensions
  foreach (vsUiExtensionInterface* extension, vsApplication::uiExtensions())
    {
    // Register this window with the extension
    extension->createInterface(this, d->Scene);
    }

  // Set up UI state persistence (after UI extensions, so they can leverage
  // geometry persistence, if applicable)
  d->UiState.mapState("Window/state", this);
  d->UiState.mapGeometry("Window/geometry", this);

  d->UiState.mapChecked("TracksVisible",
                        d->UI.actionTracksShow);
  d->UiState.mapChecked("TrackBoxesVisible",
                        d->UI.actionTracksShowBoxes);
  d->UiState.mapChecked("TrackIdsVisible",
                        d->UI.actionTracksShowId);
  d->UiState.mapChecked("TrackPvoVisible",
                        d->UI.actionTracksShowPvo);
  d->UiState.mapChecked("EventTracksVisible",
                        d->UI.actionEventsTracksShow);
  d->UiState.mapChecked("EventBoxesVisible",
                        d->UI.actionEventsBoxesShow);
  d->UiState.mapChecked("EventLabelsVisible",
                        d->UI.actionEventsLabelsShow);
  d->UiState.mapChecked("EventProbabilityVisible",
                        d->UI.actionEventsProbabilityShow);
  d->UiState.mapChecked("NotesVisible",
                        d->UI.actionNotesShow);
  d->UiState.mapChecked("GroundTruthVisible",
                        d->UI.groundTruthVisible);

  if (invokingView)
    {
    // Copy window state (except position)
    this->restoreState(invokingView->saveState());
    this->restoreGeometry(invokingView->saveGeometry());
    this->move(0, 0);

    // Copy other settings
    // \TODO probably should tell other window to save its state, and then
    //       use our regular state restore code, rather than this separate code
    //       to copy everything
    vsMainWindowPrivate* id = invokingView->d_func();
    Ui::MainWindow& myUi = d->UI;
    Ui::MainWindow& ivUi = id->UI;

    int resampleMode = id->resampleMode(id->AM.videoSampling->checkedAction());
    d->resampleAction(resampleMode)->setChecked(true);

    copyBoolOption(myUi.actionTracksShow,
                   ivUi.actionTracksShow);
    copyBoolOption(myUi.actionTracksShowBoxes,
                   ivUi.actionTracksShowBoxes);
    copyBoolOption(myUi.actionTracksShowId,
                   ivUi.actionTracksShowId);
    copyBoolOption(myUi.actionTracksShowPvo,
                   ivUi.actionTracksShowPvo);

    // \TODO copy trail length

    copyBoolOption(myUi.actionEventsTracksShow,
                   ivUi.actionEventsTracksShow);
    copyBoolOption(myUi.actionEventsBoxesShow,
                   ivUi.actionEventsBoxesShow);
    copyBoolOption(myUi.actionEventsLabelsShow,
                   ivUi.actionEventsLabelsShow);
    copyBoolOption(myUi.actionEventsProbabilityShow,
                   ivUi.actionEventsProbabilityShow);

    copyBoolOption(myUi.groundTruthVisible,
                   ivUi.groundTruthVisible);
    }
  else
    {
    // Restore previous window state and managed settings
    d->UiState.restore();

    // Restore other settings
    QSettings settings;
    int resampleMode = d->resampleMode(d->AM.videoSampling->checkedAction());
    resampleMode = settings.value("VideoResampling", resampleMode).toInt();
    d->resampleAction(resampleMode)->setChecked(true);

    scene->setVideoLiveOffset(
      settings.value("VideoLiveOffset",
                     scene->videoLiveOffset()).toDouble());

    scene->setTrackTrailLength(
      settings.value("TrackTrailLength",
                     scene->trackTrailLength()).toDouble());

    scene->setSelectionColor(vsSettings().selectionPenColor());
    }

  // Set dock corner affiliation
  this->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
  this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
}

//-----------------------------------------------------------------------------
vsMainWindow::~vsMainWindow()
{
}

//-----------------------------------------------------------------------------
void vsMainWindow::closeEvent(QCloseEvent* e)
{
  QTE_D(vsMainWindow);

  vsApplication::instance()->viewCloseEvent(this, e);

  // Save window settings and managed UI state
  d->UiState.save();

  // Save other settings
  QSettings settings;
  settings.setValue("VideoResampling",
                    d->resampleMode(d->AM.videoSampling->checkedAction()));

  settings.setValue("TrackTrailLength",
                    d->Scene->trackTrailLength());

  settings.setValue("VideoLiveOffset",
                    d->Scene->videoLiveOffset());

  // Hand off to default handler to accept the event and close the window
  QWidget::closeEvent(e);
}

//-----------------------------------------------------------------------------
qtPrioritizedMenuProxy* vsMainWindow::toolsMenu()
{
  QTE_D(vsMainWindow);
  return d->ToolsMenu.data();
}

//-----------------------------------------------------------------------------
qtPrioritizedToolBarProxy* vsMainWindow::toolsToolBar()
{
  QTE_D(vsMainWindow);
  return d->ToolsToolBar.data();
}

//-----------------------------------------------------------------------------
QVTKWidget* vsMainWindow::view()
{
  QTE_D(vsMainWindow);
  return d->UI.renderFrame;
}

//-----------------------------------------------------------------------------
void vsMainWindow::pushViewCursor(const QCursor& c, QObject* owner)
{
  QTE_D(vsMainWindow);

  d->ViewCursorOwners.removeAll(owner);
  d->ViewCursorOwners.append(owner);
  d->ViewCursorStacks[owner].push(c);

  d->UI.renderFrame->setCursor(c);
}

//-----------------------------------------------------------------------------
void vsMainWindow::popViewCursor(QObject* owner)
{
  QTE_D(vsMainWindow);

  QStack<QCursor>* stack = &d->ViewCursorStacks[owner];
  if (stack->isEmpty())
    {
    d->ViewCursorStacks.remove(owner);
    return;
    }

  stack->pop();
  if (stack->isEmpty())
    {
    d->ViewCursorOwners.removeAll(owner);
    d->ViewCursorStacks.remove(owner);
    }

  stack = &d->ViewCursorStacks[d->ViewCursorOwners.last()];
  d->UI.renderFrame->setCursor(stack->top());
}

//-----------------------------------------------------------------------------
void vsMainWindow::addDockWidget(QDockWidget* dock, Qt::DockWidgetArea area,
                                 QAction* toggleDockAction)
{
  QTE_D(vsMainWindow);

  this->addDockWidget(area, dock);
  d->DockController->addToggleAction(toggleDockAction, dock);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoPlaybackLive()
{
  QTE_D(vsMainWindow);
  d->Scene->setVideoPlaybackSpeed(vgVideoPlayer::Live, d->VideoPlaybackRate);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoPlaybackNormal()
{
  QTE_D(vsMainWindow);
  d->OldVideoPlaybackLive = false;
  d->setVideoPlaybackSlow(1.0);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoPlaybackReversed()
{
  QTE_D(vsMainWindow);
  d->OldVideoPlaybackLive = false;
  d->setVideoPlaybackSlow(-1.0);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoPlaybackPaused()
{
  QTE_D(vsMainWindow);

  if (!qFuzzyIsNull(d->VideoPlaybackRate))
    d->OldVideoPlaybackRate = d->VideoPlaybackRate;
  d->VideoPlaybackRate = 0.0;

  if (d->VideoPlaybackMode != vgVideoPlayer::Playing &&
      d->VideoPlaybackMode != vgVideoPlayer::Buffering &&
      d->VideoPlaybackMode != vgVideoPlayer::Live)
    {
    if (d->OldVideoPlaybackLive)
      {
      d->UI.actionVideoPlayLive->setChecked(true);
      this->setVideoPlaybackLive();
      }
    else
      {
      d->UI.actionVideoPlay->setChecked(true);
      this->setVideoPlaybackNormal();
      }

    return;
    }

  d->OldVideoPlaybackLive = (d->VideoPlaybackMode == vgVideoPlayer::Live);
  d->Scene->setVideoPlaybackSpeed(vgVideoPlayer::Paused);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoPlaybackResumed()
{
  QTE_D(vsMainWindow);

  if (d->VideoPlaybackMode != vgVideoPlayer::Playing &&
      d->VideoPlaybackMode != vgVideoPlayer::Buffering &&
      d->VideoPlaybackMode != vgVideoPlayer::Live)
    {
    d->VideoPlaybackRate = d->OldVideoPlaybackRate;
    if (d->OldVideoPlaybackLive)
      {
      d->UI.actionVideoPlayLive->setChecked(true);
      this->setVideoPlaybackLive();
      }
    else
      {
      d->UI.actionVideoPlay->setChecked(true);
      d->Scene->setVideoPlaybackSpeed(vgVideoPlayer::Playing,
                                      d->VideoPlaybackRate);
      }
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoPlaybackStopped()
{
  QTE_D(vsMainWindow);
  d->OldVideoPlaybackRate = 1.0;
  d->OldVideoPlaybackLive = false;
  d->VideoPlaybackRate = 0.0;
  d->Scene->setVideoPlaybackSpeed(vgVideoPlayer::Stopped);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoPlaybackFastBackward()
{
  QTE_D(vsMainWindow);
  d->OldVideoPlaybackLive = false;
  d->setVideoPlaybackFast(-1.0);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoPlaybackFastForward()
{
  QTE_D(vsMainWindow);
  d->OldVideoPlaybackLive = false;
  d->setVideoPlaybackFast(1.0);
}

//-----------------------------------------------------------------------------
void vsMainWindow::decreaseVideoPlaybackSpeed()
{
  QTE_D(vsMainWindow);
  d->OldVideoPlaybackLive = false;
  d->setVideoPlaybackSpeed(d->VideoPlaybackRate * 0.5);
}

//-----------------------------------------------------------------------------
void vsMainWindow::increaseVideoPlaybackSpeed()
{
  QTE_D(vsMainWindow);
  d->OldVideoPlaybackLive = false;
  d->setVideoPlaybackSpeed(d->VideoPlaybackRate * 2.0);
}

//-----------------------------------------------------------------------------
void vsMainWindow::stepVideo(int stepBy)
{
  QTE_D(vsMainWindow);

  const unsigned int frame
    = static_cast<unsigned int>(d->UI.frame->value());
  const bool haveFrameNumbers = (frame != vgTimeStamp::InvalidFrameNumber());

  vgTimeStamp ts;
  vg::SeekMode direction;

  if (abs(stepBy) > 1)
    {
    ts = (haveFrameNumbers
          ? vgTimeStamp::fromFrameNumber(frame + stepBy)
          : vgTimeStamp::fromTime(d->UI.scrubber->value().Time + stepBy));
    direction = (stepBy > 0 ? vg::SeekLowerBound : vg::SeekUpperBound);
    }
  else
    {
    ts = d->UI.scrubber->value();
    direction = (stepBy > 0 ? vg::SeekNext : vg::SeekPrevious);
    }

  d->OldVideoPlaybackLive = false;
  d->Scene->seekVideo(ts, direction, ++d->NextSeekRequestId);

  // Update our reference control to show the change, and also tag it against
  // the request so our reference doesn't get changed by a stale response
  if (haveFrameNumbers)
    {
    qtScopedBlockSignals bs(d->UI.frame);
    d->UI.frame->setValue(ts.FrameNumber);
    d->LastSpinBoxSeek = d->NextSeekRequestId;
    }
  else
    {
    qtScopedBlockSignals bs(d->UI.scrubber);
    d->UI.scrubber->setValue(ts);
    d->LastScrubberSeek = d->NextSeekRequestId;
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::stepVideoBackward()
{
  this->stepVideo(-1);
}
//-----------------------------------------------------------------------------
void vsMainWindow::stepVideoForward()
{
  this->stepVideo(+1);
}

//-----------------------------------------------------------------------------
void vsMainWindow::skipVideo(double skipBy)
{
  QTE_D(vsMainWindow);

  vgTimeStamp ts =
    vgTimeStamp::fromTime(d->UI.scrubber->value().Time + skipBy);
  vg::SeekMode direction =
    (skipBy > 0.0 ? vg::SeekLowerBound : vg::SeekUpperBound);

  d->OldVideoPlaybackLive = false;
  d->Scene->seekVideo(ts, direction, ++d->NextSeekRequestId);

  // Update our reference control to show the change, and also tag it against
  // the request so our reference doesn't get changed by a stale response
  qtScopedBlockSignals bs(d->UI.scrubber);
  d->UI.scrubber->setValue(ts);
  d->LastScrubberSeek = d->NextSeekRequestId;
}

//-----------------------------------------------------------------------------
void vsMainWindow::skipVideoBackward()
{
  QSettings settings;
  const double skip = settings.value("VideoSkipAmount", 10.0).toDouble();
  this->skipVideo(-1e6 * skip);
}
//-----------------------------------------------------------------------------
void vsMainWindow::skipVideoForward()
{
  QSettings settings;
  const double skip = settings.value("VideoSkipAmount", 10.0).toDouble();
  this->skipVideo(+1e6 * skip);
}

//-----------------------------------------------------------------------------
void vsMainWindow::seekVideoFrame(int frame, vg::SeekMode direction)
{
  QTE_D(vsMainWindow);

  if (direction == vg::SeekNearest)
    {
    // Don't change text while user is typing, even if frame is not valid
    d->BlockFrameUpdates = true;
    }

  d->Scene->seekVideo(vgTimeStamp::fromFrameNumber(frame),
                      vg::SeekLowerBound, ++d->NextSeekRequestId);
  d->LastSpinBoxSeek = d->NextSeekRequestId;
}

//-----------------------------------------------------------------------------
void vsMainWindow::seekVideo(vgTimeStamp ts, vg::SeekMode direction)
{
  QTE_D(vsMainWindow);

  d->Scene->seekVideo(ts, direction, ++d->NextSeekRequestId);
  d->LastScrubberSeek = d->NextSeekRequestId;
}

//-----------------------------------------------------------------------------
void vsMainWindow::setPendingSeek(const vtkVgTimeStamp& request)
{
  QTE_D(vsMainWindow);
  d->PendingSeek = request;
}

//-----------------------------------------------------------------------------
void vsMainWindow::setPlaybackStatus(
  vgVideoPlayer::PlaybackMode mode, qreal rate)
{
  QTE_D(vsMainWindow);

  QString s;
  QToolBar* vtb = d->UI.videoToolBar;

  switch (mode)
    {
    case vgVideoPlayer::Playing:
    case vgVideoPlayer::Buffering:
    case vgVideoPlayer::Live:
      d->VideoPlaybackRate = rate;

      if (vtb->actions().contains(d->UI.actionVideoSkipBackward))
        {
        qtScopedBlockUpdates bu(vtb);
        vtb->insertAction(d->UI.actionVideoSkipBackward,
                          d->UI.actionVideoFastBackward);
        vtb->insertAction(d->UI.actionVideoSkipForward,
                          d->UI.actionVideoFastForward);
        vtb->insertAction(d->UI.actionVideoFrameBackward,
                          d->UI.actionVideoSpeedDecrease);
        vtb->insertAction(d->UI.actionVideoFrameForward,
                          d->UI.actionVideoSpeedIncrease);
        vtb->removeAction(d->UI.actionVideoSkipBackward);
        vtb->removeAction(d->UI.actionVideoSkipForward);
        vtb->removeAction(d->UI.actionVideoFrameBackward);
        vtb->removeAction(d->UI.actionVideoFrameForward);
        }

      if (mode == vgVideoPlayer::Live)
        {
        d->UI.actionVideoPlayLive->setChecked(true);
        }
      else if (rate > 1.1)
        {
        d->UI.actionVideoFastForward->setChecked(true);
        }
      else if (rate < -1.1)
        {
        d->UI.actionVideoFastBackward->setChecked(true);
        }
      else if (rate < 0.0)
        {
        d->UI.actionVideoPlayReversed->setChecked(true);
        }
      else
        {
        d->UI.actionVideoPlay->setChecked(true);
        }

      if (mode == vgVideoPlayer::Live)
        {
        if (!qFuzzyIsNull(d->Scene->videoLiveOffset()))
          {
          s = QString("<b>LIVE</b> (-%1)").arg(d->Scene->videoLiveOffset());
          }
        else
          {
          s = "<b>LIVE</b>";
          }
        }
      else if (!qFuzzyIsNull(rate) && fabs(rate) < 0.9)
        {
        const int frac = qRound(1.0 / fabs(rate));
        const char* sc = (rate < 0.0 ? "-" : "");
        s = QString("%1<sup>1</sup>/<sub>%2</sub>x").arg(sc).arg(frac);
        }
      else
        {
        s = QString("%1x").arg(qRound(rate));
        }

      if (mode == vgVideoPlayer::Buffering)
        s = '(' + s + ')';

      break;
    default:
      d->VideoPlaybackRate = 0.0;

      if (vtb->actions().contains(d->UI.actionVideoFastBackward))
        {
        qtScopedBlockUpdates bu(vtb);
        vtb->insertAction(d->UI.actionVideoFastBackward,
                          d->UI.actionVideoSkipBackward);
        vtb->insertAction(d->UI.actionVideoFastForward,
                          d->UI.actionVideoSkipForward);
        vtb->insertAction(d->UI.actionVideoSpeedDecrease,
                          d->UI.actionVideoFrameBackward);
        vtb->insertAction(d->UI.actionVideoSpeedIncrease,
                          d->UI.actionVideoFrameForward);
        vtb->removeAction(d->UI.actionVideoFastBackward);
        vtb->removeAction(d->UI.actionVideoFastForward);
        vtb->removeAction(d->UI.actionVideoSpeedDecrease);
        vtb->removeAction(d->UI.actionVideoSpeedIncrease);
        }

      if (mode == vgVideoPlayer::Paused)
        {
        d->UI.actionVideoPause->setChecked(true);
        s = "<i>P</i>";
        }
      else
        {
        d->UI.actionVideoStop->setChecked(true);
        s = "<b>S</b>";
        }
      break;
    }

  if (d->VideoPlaybackMode != mode)
    {
    d->OldVideoPlaybackLive = (d->VideoPlaybackMode == vgVideoPlayer::Live);
    d->VideoPlaybackMode = mode;
    }
  d->UI.playbackMode->setText(s);
  d->updatePlaybackControlsState(d->UI.scrubber->value());
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoSourceStatus(vsDataSource::Status status)
{
  QTE_D(vsMainWindow);

  QStringList textList, toolTipList;
  QList<vsDataSource::Status> statusList;

  statusList.append(status);
  if (status == vsDataSource::NoSource)
    {
    d->VideoPlaybackMode = vgVideoPlayer::Stopped;
    d->UI.playbackMode->hide();
    d->setStatusVisible(vsMainWindowPrivate::StatusFrameGsd, false);
    d->setStatusVisible(vsMainWindowPrivate::StatusFrameTimestamp, false);
    d->setStatusVisible(vsMainWindowPrivate::StatusCursorLocation, false);
    d->AM.playbackActions->setEnabled(false);
    foreach (QAction* a, d->AM.playbackActions->actions())
      a->setChecked(false);
    d->UI.actionRegionDraw->setEnabled(false);
    d->UI.actionRegionDraw->setChecked(false);
    d->UI.actionRegionClose->setEnabled(false);
    textList.append("(none)");
    toolTipList.append("(no video source)");
    }
  else
    {
    d->UI.playbackMode->show();
    bool liveEnabled = false;
    switch (status)
      {
      case vsDataSource::StreamingActive:
      case vsDataSource::StreamingIdle:
        liveEnabled = true;
      case vsDataSource::StreamingStopped:
      case vsDataSource::ArchivedActive:
      case vsDataSource::ArchivedSuspended:
      case vsDataSource::ArchivedIdle:
        d->AM.playbackActions->setEnabled(true);
        d->UI.actionRegionDraw->setEnabled(true);
        d->UI.actionFormulateQuery->setEnabled(true);
        d->UI.actionVideoPlayLive->setEnabled(liveEnabled);
        d->UI.actionVideoLiveOffsetSet->setEnabled(liveEnabled);
        break;
      default:
        d->AM.playbackActions->setEnabled(false);
        foreach (QAction* a, d->AM.playbackActions->actions())
          a->setChecked(false);
        d->UI.actionRegionDraw->setEnabled(false);
        d->UI.actionRegionDraw->setChecked(false);
        d->UI.actionRegionClose->setEnabled(false);
        break;
      }
    textList.append(d->Core->videoSourceText());
    toolTipList.append(d->Core->videoSourceToolTip());
    }

  d->setSourceStatus(vsMainWindowPrivate::StatusVideoIcon,
                     vsMainWindowPrivate::StatusVideoText,
                     "video", statusList, textList, toolTipList);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setTrackSourceStatus(vsDataSource::Status status)
{
  QTE_D(vsMainWindow);

  QStringList textList, toolTipList;
  QList<vsDataSource::Status> statusList;

  for (int i = 0; ; ++i)
    {
    status = d->Core->trackSourceStatus(i);
    if (status == vsDataSource::NoSource)
      break;

    statusList.append(status);
    textList.append(d->Core->trackSourceText(i));
    toolTipList.append(d->Core->trackSourceToolTip(i));
    }
  if (!statusList.size())
    {
    statusList.append(vsDataSource::NoSource);
    textList.append("(none)");
    toolTipList.append("(no track source)");
    }

  d->setSourceStatus(vsMainWindowPrivate::StatusTrackIcon,
                     vsMainWindowPrivate::StatusTrackText,
                     "track", statusList, textList, toolTipList);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setDescriptorSourceStatus(vsDataSource::Status status)
{
  QTE_D(vsMainWindow);

  QStringList textList, toolTipList;
  QList<vsDataSource::Status> statusList;

  for (int i = 0; ; ++i)
    {
    status = d->Core->descriptorSourceStatus(i);
    if (status == vsDataSource::NoSource)
      break;

    statusList.append(status);
    textList.append(d->Core->descriptorSourceText(i));
    toolTipList.append(d->Core->descriptorSourceToolTip(i));
    }
  if (!statusList.size())
    {
    statusList.append(vsDataSource::NoSource);
    textList.append("(none)");
    toolTipList.append("(no descriptor source)");
    }

  d->setSourceStatus(vsMainWindowPrivate::StatusDescriptorIcon,
                     vsMainWindowPrivate::StatusDescriptorText,
                     "event", statusList, textList, toolTipList);
}

//-----------------------------------------------------------------------------
void vsMainWindow::resetVideo()
{
  QTE_D(vsMainWindow);

  // Clear any old video range, so that a: the controls do not reflect the
  // state of a previous video while we wait on the new video's range to
  // update, and b: we will correctly detect the new range to start automatic
  // playback
  qtScopedBlockSignals bsf(d->UI.frame), bss(d->UI.scrubber);
  d->UI.frame->setRange(-1, -1);
  d->UI.scrubber->setRange(vgTimeStamp(), vgTimeStamp());
  d->AvailableVideoRange = vgRange<vgTimeStamp>();

  d->updatePlaybackControlsState(d->UI.scrubber->value());

  // \TODO disable playback controls?
}

//-----------------------------------------------------------------------------
void vsMainWindow::updateVideoAvailableTimeRange(
  vtkVgTimeStamp first, vtkVgTimeStamp last)
{
  QTE_D(vsMainWindow);

  const bool valid = !d->UI.scrubber->isRangeEmpty();

  qtScopedBlockSignals bsf(d->UI.frame), bss(d->UI.scrubber);
  d->UI.frame->setRange(first.GetFrameNumber(), last.GetFrameNumber());
  d->UI.scrubber->setRange(first.GetRawTimeStamp(), last.GetRawTimeStamp());
  d->AvailableVideoRange.lower = first.GetRawTimeStamp();
  d->AvailableVideoRange.upper = last.GetRawTimeStamp();

  d->updatePlaybackControlsState(d->UI.scrubber->value());

  // If we had no video before, but do now...
  if (!valid && !d->UI.scrubber->isRangeEmpty())
    {
    // ..and have a pending seek request...
    if (d->PendingSeek.IsValid())
      {
      // ...then try to fulfill the request (and clear it)...
      if (first <= d->PendingSeek && d->PendingSeek <= last)
        {
        // Issue the pending seek (which will pause automatically), queued,
        // as the animation may not have updated yet
        QMetaObject::invokeMethod(
          this, "seekVideo", Qt::QueuedConnection,
          Q_ARG(vgTimeStamp, d->PendingSeek.GetRawTimeStamp()),
          Q_ARG(vg::SeekMode, vg::SeekNearest));
        }
      d->PendingSeek.Reset();
      }
    else
      {
      // ...otherwise start playing automatically, from beginning
      this->seekVideo(first.GetRawTimeStamp(), vg::SeekNearest);
      QMetaObject::invokeMethod(d->UI.actionVideoPlay, "trigger",
                                Qt::QueuedConnection);
      }
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::updateVideoMetadata(
  vtkVgVideoFrameMetaData metaData, qint64 seekRequestId)
{
  QTE_D(vsMainWindow);

  // If we have video, show the pane for cursor location (which isn't updated
  // from this slot)
  d->setStatusVisible(vsMainWindowPrivate::StatusCursorLocation, true);

  // Set GSD
  const double threshold =
    QSettings().value("GsdMetersThreshold", 0.5).toDouble();
  const QString& text = makeGsdText(metaData.Gsd, threshold);
  d->setStatusVisible(vsMainWindowPrivate::StatusFrameGsd, true);
  d->setStatusText(vsMainWindowPrivate::StatusFrameGsd, text);

  // Set time stamp
  vgUnixTime unixTime(metaData.Time.GetTime());
  d->setStatusVisible(vsMainWindowPrivate::StatusFrameTimestamp, true);
  d->setStatusText(vsMainWindowPrivate::StatusFrameTimestamp,
                   unixTime.timeString());

  // Update video seek controls
  this->updateVideoPosition(seekRequestId, metaData.Time);
}

//-----------------------------------------------------------------------------
void vsMainWindow::updateVideoPosition(
  qint64 seekRequestId, vtkVgTimeStamp vtkTimeStamp)
{
  QTE_D(vsMainWindow);

  vgTimeStamp ts = vtkTimeStamp.GetRawTimeStamp();

  // Check if there are pending seek requests
  bool mayUpdateScrubber = true;
  bool mayUpdateSpinBox = true;
  if (d->NextSeekRequestId > 0)
    {
    if (seekRequestId >= d->NextSeekRequestId)
      {
      // The last pending seek request has been fulfilled; we can update the
      // video position controls, and also since there are no outstanding
      // requests, we can reset the request counter (so, less likely that it
      // will wrap)
      d->NextSeekRequestId = 0;
      d->LastScrubberSeek = -1;
      d->LastSpinBoxSeek = -1;
      d->SeekFlushLoop.quit();
      }
    else
      {
      // There are outstanding seek requests... don't update the video position
      // controls responsible for outstanding seek requests; since they have
      // been moved by the user since we sent the request whose response we are
      // processing, doing so could cause visual "jerking" or - in the case of
      // the frame spin box - could cause new changes to be calculated relative
      // to the wrong frame
      mayUpdateScrubber = (seekRequestId >= d->LastScrubberSeek);
      mayUpdateSpinBox = (seekRequestId >= d->LastSpinBoxSeek);
      }
    }

  if (mayUpdateSpinBox)
    {
    // Set frame number (unless user is currently typing one)
    if (!d->BlockFrameUpdates)
      {
      qtScopedBlockSignals bs(d->UI.frame);
      d->UI.frame->setValue(ts.FrameNumber);
      }
    }

  if (mayUpdateScrubber)
    {
    // Set scrubber position
    qtScopedBlockSignals bs(d->UI.scrubber);
    d->UI.scrubber->setValue(ts);
    }

  // Enable/disable frame step controls
  d->updatePlaybackControlsState(ts);
}

//-----------------------------------------------------------------------------
void vsMainWindow::flushPendingSeeks(int timeout)
{
  QTE_D(vsMainWindow);

  if (d->LastScrubberSeek >= 0 || d->LastSpinBoxSeek >= 0)
    {
    // Set a single shot timer to cancel the internal event loop when the
    // timeout expires; we don't use QTimer::singleShot because we need the
    // timer and associated connection to go away if we finish before the time
    // out expires
    QTimer timer;
    timer.setInterval(timeout);
    connect(&timer, SIGNAL(timeout()), &d->SeekFlushLoop, SLOT(quit()));

    // Run internal event loop until all pending seeks flush, or we time out
    d->SeekFlushLoop.exec();
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::setCursorLocationText(QString text)
{
  QTE_D(vsMainWindow);
  d->setStatusText(vsMainWindowPrivate::StatusCursorLocation, text);
}

//-----------------------------------------------------------------------------
void vsMainWindow::createSource(QString identifier)
{
  QTE_D(vsMainWindow);

  vsSourceFactoryPtr factory = vsSourceService::createFactory(identifier);
  if (!factory)
    return;

  if (factory->initialize(this))
    d->Core->addSources(factory);
}

//-----------------------------------------------------------------------------
void vsMainWindow::loadFilterSettings(QString fileName)
{
  QTE_D(vsMainWindow);

  if (fileName.isEmpty())
    {
    fileName = vgFileDialog::getOpenFileName(
                 this, "Load filter settings...", QString(),
                 "Filter Settings (*.vpefs);;"
                 "All files (*)");
    if (fileName.isEmpty())
      return;
    }
  else
    {
    QFileInfo fi(fileName);
    if (!fi.exists())
      return;
    fileName = fi.canonicalFilePath();
    }

  qtKstReader reader(QUrl::fromLocalFile(fileName));
  if (!reader.isValid())
    {
    d->UI.statusbar->showMessage("Unable to load filter settings file");
    return;
    }

  // Check header
  QString header;
  if (!reader.readString(header)
      || header != "FILTERS"
      || !reader.nextRecord())
    {
    d->UI.statusbar->showMessage("Unable to load filter settings file");
    return;
    }

  // Read filter settings
  while (!reader.isEndOfFile())
    {
    int type;
    int visibility;
    double threshold;
    if (reader.readInt(type, 0) && reader.readInt(visibility, 1))
      {
      if (type == vsCore::GroundTruth)
        {
        d->UI.groundTruthVisible->setChecked(!!visibility);
        }
      else if (reader.readReal(threshold, 2))
        {
        d->UI.filter->setState(type, !!visibility);
        d->UI.filter->setValue(type, threshold);
        }
      }
    reader.nextRecord();
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::saveFilterSettings(QString fileName)
{
  QTE_D(vsMainWindow);

  if (fileName.isEmpty())
    {
    fileName = vgFileDialog::getSaveFileName(
                 this, "Save filter settings...", QString(),
                 "Filter Settings (*.vpefs);;"
                 "All files (*)");
    if (fileName.isEmpty())
      return;
    }
  else
    {
    QFileInfo fi(fileName);
    if (!fi.dir().exists())
      return;
    fileName = fi.canonicalFilePath();
    }

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
    {
    d->UI.statusbar->showMessage("Error writing file");
    return;
    }

  QTextStream out(&file);
  out << "FILTERS;\n# Type, Visibility, Threshold\n";
  out << vsCore::GroundTruth
      << (d->UI.groundTruthVisible->isChecked() ? 1 : 0);
  foreach (int type, d->UI.filter->keys())
    {
    out << type << ", "
        << (d->UI.filter->state(type) ? 1 : 0) << ", "
        << d->UI.filter->value(type) << ";\n";
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::loadMaskImage(QString fileName)
{
  QTE_D(vsMainWindow);

  QFileInfo fi(fileName);
  if (!fi.exists())
    {
    return;
    }

  if (d->Scene->setMaskImage(fi.canonicalFilePath()))
    {
    d->UI.actionVideoMaskShowTracking->setEnabled(true);
    d->UI.actionVideoMaskShowTracking->setChecked(true);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::setTrackColoring()
{
  QTE_D(vsMainWindow);

  vsTrackColorDialog dlg;
  dlg.setDynamicDataSets(d->Core->dynamicDataSets());
  switch (d->Scene->trackColorMode())
    {
    case vtkVgTrackRepresentationBase::TCM_Scalars:
      dlg.setMode(vsTrackColorDialog::ColorByDynamicData);
      dlg.setDynamicDataSet(d->Scene->trackColorDataId());
      break;
    case vtkVgTrackRepresentationBase::TCM_TOC:
      dlg.setMode(vsTrackColorDialog::ColorByClassification);
      break;
    default:
      dlg.setMode(vsTrackColorDialog::SingleColor);
      break;
    }

  if (dlg.exec() == QDialog::Accepted)
    {
    vtkVgTrackRepresentationBase::TrackColorMode mode;
    switch(dlg.mode())
      {
      case vsTrackColorDialog::ColorByClassification:
        mode = vtkVgTrackRepresentationBase::TCM_TOC;
        break;
      case vsTrackColorDialog::ColorByDynamicData:
        mode = vtkVgTrackRepresentationBase::TCM_Scalars;
        break;
      case vsTrackColorDialog::SingleColor:
      default:
        mode = vtkVgTrackRepresentationBase::TCM_Default;
        break;
      }

    d->Scene->updateTrackColors(mode);
    d->Scene->setTrackColorMode(mode, dlg.dynamicDataSet());
    // TODO update tree
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::setTrackTrailLength()
{
  QTE_D(vsMainWindow);

  double currentLength = d->Scene->trackTrailLength();

  bool okay = false;
  double newLength = qtDoubleInputDialog::getDouble(
                       this, "Set Track Trail Length", "Trail Length (seconds)",
                       currentLength, 0.0, 1e6, "Unlimited", 1, 0.1, &okay);

  if (okay)
    {
    d->Scene->setTrackTrailLength(newLength);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::setVideoLiveOffset()
{
  QTE_D(vsMainWindow);

  const double currentOffset = d->Scene->videoLiveOffset();

  bool okay = false;
  double newOffset = qtDoubleInputDialog::getDouble(
                       this, "Set Live Offset", "Live Offset (seconds)",
                       currentOffset, 0.0, 1e6, 1, 0.1, &okay);

  if (okay)
    {
    d->Scene->setVideoLiveOffset(newOffset);
    this->setPlaybackStatus(d->VideoPlaybackMode, d->VideoPlaybackRate);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::setEventDisplayThreshold()
{
  bool okay = false;
  double t = qtDoubleInputDialog::getDouble(
               this, "Set Event Threshold", "Threshold",
               0.0, 0.0, 1.0, 2, 0.01, &okay);

  if (okay)
    {
    QTE_D(vsMainWindow);
    d->Scene->setEventGroupThreshold(vsEventInfo::All, t);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::showEventsAllPerson()
{
  QTE_D(vsMainWindow);
  d->Scene->setEventGroupVisibility(vsEventInfo::Person, true);
}

//-----------------------------------------------------------------------------
void vsMainWindow::showEventsAllVehicle()
{
  QTE_D(vsMainWindow);
  d->Scene->setEventGroupVisibility(vsEventInfo::Vehicle, true);
}

//-----------------------------------------------------------------------------
void vsMainWindow::hideEventsAllPerson()
{
  QTE_D(vsMainWindow);
  d->Scene->setEventGroupVisibility(vsEventInfo::Person, false);
}

//-----------------------------------------------------------------------------
void vsMainWindow::hideEventsAllVehicle()
{
  QTE_D(vsMainWindow);
  d->Scene->setEventGroupVisibility(vsEventInfo::Vehicle, false);
}

//-----------------------------------------------------------------------------
void vsMainWindow::setStatusText(QString text)
{
  QTE_D(vsMainWindow);
  d->setStatusText(vsMainWindowPrivate::StatusText, text);
}

//-----------------------------------------------------------------------------
void vsMainWindow::toggleDrawing(bool state)
{
  QTE_D(vsMainWindow);
  if (state)
    {
    this->pushViewCursor(Qt::CrossCursor, this);
    qtScopedValueChange<bool> ic(d->IgnoreCancelDrawing, true);
    d->Scene->beginDrawing(d->currentDrawingType());
    }
  else
    {
    d->Scene->interruptDrawing();
    this->popViewCursor(this);
    d->UI.actionRegionClose->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::updateTrackLabelActionsEnabled()
{
  QTE_D(vsMainWindow);

  bool enabled = d->UI.actionTracksShow->isChecked() ||
                 d->UI.actionTracksShowBoxes->isChecked();
  d->UI.actionTracksShowId->setEnabled(enabled);
  d->UI.actionTracksShowPvo->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void vsMainWindow::createAlert()
{
  vsAlertEditor editor;
  if (editor.exec() == QDialog::Accepted)
    {
    QTE_D(vsMainWindow);
    d->Core->addAlert(editor.alert());
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::loadAlert()
{
  QTE_D(vsMainWindow);

  QStringList paths = vgFileDialog::getOpenFileNames(
                        this, "Load alert(s)...", QString(),
                        "VisGUI Saved Alert (*.vsa *.vsax);;"
                        "All files (*)");

  vsAlertEditor editor;
  foreach (QString path, paths)
    {
    if (editor.loadAlert(path))
      {
      d->Core->addAlert(editor.alert());
      }
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::formulateQuery()
{
  QTE_D(vsMainWindow);

  // Get padding for QF
  QSettings settings;
  settings.beginGroup("QueryFormulation");
  const double padBefore = settings.value("PaddingBefore", 10.0).toDouble();
  const double padAfter = settings.value("PaddingAfter", 10.0).toDouble();
  const double currentTime = d->UI.scrubber->value().Time;

  const vgTimeStamp start =
    vgTimeStamp::fromTime(currentTime - (padBefore * 1e6));
  const vgTimeStamp end =
    vgTimeStamp::fromTime(currentTime + (padAfter * 1e6));

  // Create video source for QF
  // \TODO: Use vgVideoSource directly
  vsQfVideoSource* vs = vsQfVideoSource::New();
  vs->SetSource(d->VideoSource);
  vs->SetMetadata(d->Core->allMetadata());
  vs->SetTimeRange(start.Time, end.Time);

  // Get relevant tracks and descriptors
  QList<vvTrack> tracks;
  QList<vvDescriptor> descriptors;
  bool canceled;
  d->Core->getQueryFormulationData(start, end, tracks, descriptors,
                                   &canceled);

  // Check for canceled, or no descriptors found
  CHECK_ARG(!canceled);
  if (descriptors.isEmpty())
    {
    QMessageBox::information(
      this, "Query Formulation",
      "No descriptors are available at this temporal location");
    return;
    }

  // Prepare and show formulation dialog
  vsQfDialog qf(vs);
  qf.initialize(vgTimeStamp::fromTime(currentTime));
  qf.setQueryTracksAndDescriptors(descriptors, tracks);
  qf.exec();

  vs->Delete();

  CHECK_ARG(!qf.selectedDescriptors().empty());

  // Create query plan
  vvQueryInstance qi(vvDatabaseQuery::Similarity);
  vvSimilarityQuery* query = qi.similarityQuery();

  query->QueryId = vvMakeId("VSPLAY");
  query->Descriptors = qf.selectedDescriptors();
  query->SimilarityThreshold = 0.0;

  QString fileName = vgFileDialog::getSaveFileName(
                       this, "Saved query...", QString(),
                       "VisGUI Query Plans (*.vqp);;"
                       "All files (*)");
  CHECK_ARG(!fileName.isEmpty());

  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly))
    {
    // Write out the query plan
    vvWriter writer(file);
    writer << vvHeader::QueryPlan << qi;
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::initializeTesting(const qtCliArgs* args)
{
#ifdef ENABLE_QTTESTING
  QTE_D(vsMainWindow);
  d->TestUtility = new pqCoreTestUtility(this, d->UI.renderFrame, this);
  d->TestUtility->SetDataRootEnvironment("VISGUI_DATA_ROOT");
  d->TestUtility->AddTestingMenu(this->menuBar(), "s", d->UI.menuHelp);

  d->TestUtility->ParseCommandLine(*args);
  d->TestUtility->ProcessCommandLine();
#endif

  Q_UNUSED(args);
}

//-----------------------------------------------------------------------------
void vsMainWindow::changeVideoSource(vsVideoSource* vs)
{
  QTE_D(vsMainWindow);
  d->VideoSource = vs;
}

//-----------------------------------------------------------------------------
void vsMainWindow::writeRenderedImages(bool state)
{
  QTE_D(vsMainWindow);

  // if turning "on", ask for an output directory (required)
  if (state == true)
    {
    QFileDialog fileDialog(0, tr("Image Output Directory"));
    fileDialog.setObjectName("WriteRenderedImagesDialog");
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::ShowDirsOnly);
    if (fileDialog.exec() == QDialog::Accepted)
      {
      d->Scene->setWriteRenderedImages(
        state, &fileDialog.selectedFiles().front());
      }
    else
      {
      d->UI.actionWriteRenderedImages->setChecked(false);
      }
    }
  else
    {
    d->Scene->setWriteRenderedImages(false);
    }
}

//-----------------------------------------------------------------------------
void vsMainWindow::saveScreenShot()
{
  QTE_D(vsMainWindow);

  QString filePath = vgFileDialog::getSaveFileName(
                       this, "Save screenshot...",
                       QString(), "PNG File (*.png);;");

  if (!filePath.isEmpty())
    {
    d->Scene->saveScreenShot(&filePath);
    }
}

//END vsMainWindow
