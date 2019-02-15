/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqApplication.h"

#include <QActionGroup>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QSettings>
#include <QSignalMapper>
#include <QToolButton>
#include <QUrl>

#include <qtCliArgs.h>
#include <qtDockController.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <vgCheckArg.h>

#include <vgFileDialog.h>

#include <vgAboutAction.h>

#include <vvGenerateReportDialog.h>
#include <vvQueryFormulation.h>

#include "vtkVgVideoNode.h"
#include "vtkVgViewerBase.h"

#include "vqCore.h"
#include "vqVersion.h"

#include "vqConfigureDialog.h"
#include "vqEventInfo.h"
#include "vqPredefinedQueryCache.h"
#include "vqQueryDialog.h"
#include "vqResultFilterDialog.h"
#include "vqSettings.h"
#include "vqTrackingClipViewer.h"
#include "vqUserActions.h"
#include "vqVideoQueryDialog.h"

#include "Backends/vqExporterFactory.h"

#ifdef ENABLE_QTTESTING
#include "pqCoreTestUtility.h"
#endif

//-----------------------------------------------------------------------------
#define connectCoreDisplayToggle(_a_, _s_) do { \
  connect(_a_, SIGNAL(toggled(bool)), this->Core, SLOT(_s_(bool))); \
  this->Core->_s_(_a_->isChecked()); \
  } while (0)

#define connectDisplayToggle(_a_, _o2_, _s_) do { \
  connect(_a_, SIGNAL(toggled(bool)), _o2_, SLOT(_s_(bool))); \
  _o2_->_s_(_a_->isChecked()); \
  } while (0)

//-----------------------------------------------------------------------------
void vqApplication::connectTreeWidget(vqTreeView* tree)
{
  connect(tree, SIGNAL(NodesSelected(QList<vtkVgNodeBase*>)),
          this->Core, SLOT(selectNodes(QList<vtkVgNodeBase*>)));
  connect(tree, SIGNAL(NodeActivated(vtkVgNodeBase&)),
          this->Core, SLOT(activateNode(vtkVgNodeBase&)));

  connect(this->Core, SIGNAL(selected(QList<vtkVgNodeBase*>)),
          tree, SLOT(SelectNodes(QList<vtkVgNodeBase*>)));

  connect(tree, SIGNAL(ItemsChanged()),
          this->Core, SLOT(updateResultScoreIndicators()));
  connect(tree, SIGNAL(ItemsChanged()),
          this->Core, SLOT(updateLayoutStacks()));
  connect(tree, SIGNAL(ItemsChanged()),
          this->Core, SLOT(updateTrackVisibility()));
  connect(tree, SIGNAL(ItemsChanged()),
          this->UI.timeline, SLOT(Update()));
  connect(tree, SIGNAL(ShowTrackingClips(QList<vtkVgVideoNode*>)),
          this, SLOT(showTrackingClips(QList<vtkVgVideoNode*>)));

  connect(tree, SIGNAL(UserScoreChanged(long long, int)),
          this, SLOT(setUserScore(long long, int)));
  connect(tree, SIGNAL(NoteChanged(long long, const QString&)),
          this, SLOT(setResultNote(long long, const QString&)));
  connect(tree, SIGNAL(StarredChanged(long long, bool)),
          this, SLOT(setResultStarred(long long, bool)));

  // Only update activated when we are playing a video node.
  connect(this->UserActions, SIGNAL(Play(vtkVgVideoNode&)),
          tree, SLOT(UpdateActivated(vtkVgVideoNode&)));

  connect(this->Core, SIGNAL(displayResultsReset()),
          tree, SLOT(Reset()));
}

//-----------------------------------------------------------------------------
void vqApplication::setEnabledSignal(
  QAction* action, QObject* sender, const char* signal, bool initiallyEnabled)
{
  connect(sender, signal, action, SLOT(setEnabled(bool)));
  action->setEnabled(initiallyEnabled);
}

//-----------------------------------------------------------------------------
vqApplication::vqApplication(UIMode uiMode) :
  InterfaceMode(uiMode), Core(new vqCore), ResultPageOffset(0),
  NewQueryDialog(0), TestUtility(0)
{
  this->UI.setupUi(this);
  this->Core->setupUi(this->UI.renderFrame);

  // Hide some UI if not in engineering mode
  if (!this->useAdvancedUI())
    {
    this->UI.actionLayerManagerShow->setVisible(false);
    this->UI.actionProjectOpen->setVisible(false);
    this->UI.actionProjectSave->setVisible(false);

    // This seems to be the only way to hide the submenu...
    delete this->UI.menuProjectMru;
    this->UI.menuProjectMru = 0;

    // Remove toolbar separator
    this->UI.layerToolBar->clear();
    this->UI.layerToolBar->addAction(this->UI.actionLayerAddFile);
    }

  // Set window title (i.e. add version number) and icon
  QString title("%1 %3");
  title = title.arg(this->windowTitle()).arg(VIQUI_VERSION_STR);
  this->setWindowTitle(title);
  qtUtil::setApplicationIcon("viqui", this);

  // Set up About dialog
  this->UI.menuHelp->addAction(new vgAboutAction(this));

  // Set up dock toggle actions
  this->DockController = new qtDockController(this);
  this->DockController->addToggleAction(this->UI.actionViewResults,
                                        this->UI.resultDock);
  this->DockController->addToggleAction(this->UI.actionViewResultInfo,
                                        this->UI.resultInfoDock);
  this->DockController->addToggleAction(this->UI.actionViewScore,
                                        this->UI.scoreDock);
  this->DockController->addToggleAction(this->UI.actionViewGroundTruth,
                                        this->UI.groundTruthDock);
  this->DockController->addToggleAction(this->UI.actionViewTimeline,
                                        this->UI.timelineDock);
  this->DockController->addToggleAction(this->UI.actionViewVideo,
                                        this->UI.videoPlayerDock);
  this->DockController->addToggleAction(this->UI.actionViewQueryClip,
                                        this->UI.queryClipDock);

  // Set default dock visibility (previous state, if saved, will override this)
  this->UI.queryClipDock->setVisible(false);
  this->UI.timelineDock->setVisible(false);

  this->UI.queryClip->initialize();
  this->UI.videoPlayer->initialize();

  // Set up status bar
  QLabel* statusLabel = new QLabel;
  QProgressBar* statusProgress = new QProgressBar;
  statusProgress->setMinimumWidth(170);
  this->UI.statusbar->addWidget(statusLabel, 1);
  this->UI.statusbar->addWidget(statusProgress);

  this->Core->registerStatusWidget(statusLabel);
  this->Core->registerStatusWidget(statusProgress);

  // Set up Export menus
  QSignalMapper* selectedMapper = new QSignalMapper(this);
  QSignalMapper* starredMapper = new QSignalMapper(this);
  QSignalMapper* allMapper = new QSignalMapper(this);
  foreach (vqExporterFactory::Identifier e, vqExporterFactory::exporters())
    {
    QString text = e.displayString + "...";

    QAction* selectedAction =
      this->UI.menuQueryExportSelectedResults->addAction(text);
    connect(selectedAction, SIGNAL(triggered()), selectedMapper, SLOT(map()));
    selectedMapper->setMapping(selectedAction, e.id);

    QAction* starredAction =
      this->UI.menuQueryExportStarredResults->addAction(text);
    connect(starredAction, SIGNAL(triggered()), starredMapper, SLOT(map()));
    starredMapper->setMapping(starredAction, e.id);

    QAction* allAction = this->UI.menuQueryExportAllResults->addAction(text);
    connect(allAction, SIGNAL(triggered()), allMapper, SLOT(map()));
    allMapper->setMapping(allAction, e.id);
    }

  connect(selectedMapper, SIGNAL(mapped(const QString&)),
          this, SLOT(exportSelectedResults(const QString&)));
  connect(starredMapper, SIGNAL(mapped(const QString&)),
          this, SLOT(exportStarredResults(const QString&)));
  connect(allMapper, SIGNAL(mapped(const QString&)),
          this, SLOT(exportAllResults(const QString&)));

  this->UI.menuQueryExportStarredResults->addAction(this->UI.actionExportKml);

  // Set up Export tool button
  QWidget* spacer = new QWidget;
  spacer->setMinimumSize(0, 0);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QToolButton* button = new QToolButton;
  button->setText("Export");
  button->setPopupMode(QToolButton::MenuButtonPopup);
  button->setMenu(this->UI.menuQueryExportAllResults);
  this->UI.resultToolBar->addWidget(spacer);
  this->UI.resultToolBar->addWidget(button);
  connect(button, SIGNAL(pressed()), button, SLOT(showMenu()));

  // Set up action icons
  // FIXME should be using qtAmc, which would do this for us...
  using qtUtil::standardActionIcon;
  this->UI.actionQuit->setIcon(standardActionIcon("quit"));
  this->UI.actionQueryNew->setIcon(standardActionIcon("new-query"));
  this->UI.actionQueryEdit->setIcon(standardActionIcon("edit-query"));
  this->UI.actionQueryOpen->setIcon(standardActionIcon("load-query"));
  this->UI.actionQuerySave->setIcon(standardActionIcon("save-query"));
  this->UI.actionConfigure->setIcon(standardActionIcon("configure"));

  // Query region visibility
  connect(this->UI.actionShowQueryRegion, SIGNAL(triggered(bool)),
          this->Core, SLOT(setShowQueryRegion(bool)));

  // Make coloring options mutually exclusive
  QActionGroup* colorScoringGroup = new QActionGroup(this);
  colorScoringGroup->addAction(this->UI.actionColorByRank);
  colorScoringGroup->addAction(this->UI.actionColorByRelativeScore);
  colorScoringGroup->addAction(this->UI.actionColorByAbsoluteScore);

  QActionGroup* colorMappingGroup = new QActionGroup(this);
  colorMappingGroup->addAction(this->UI.actionColorShadeDiscrete);
  colorMappingGroup->addAction(this->UI.actionColorShadeLinear);
  colorMappingGroup->addAction(this->UI.actionColorShadeCubic);

  // Set up coloring option connections
  connect(this->UI.actionColorByRank, SIGNAL(triggered()),
          this->Core, SLOT(setColorByRank()));
  connect(this->UI.actionColorByAbsoluteScore, SIGNAL(triggered()),
          this->Core, SLOT(setColorByAbsoluteScore()));
  connect(this->UI.actionColorByRelativeScore, SIGNAL(triggered()),
          this->Core, SLOT(setColorByRelativeScore()));
  connect(this->UI.actionColorShadeDiscrete, SIGNAL(triggered()),
          this->Core, SLOT(setColorShadingDiscrete()));
  connect(this->UI.actionColorShadeLinear, SIGNAL(triggered()),
          this->Core, SLOT(setColorShadingLinear()));
  connect(this->UI.actionColorShadeCubic, SIGNAL(triggered()),
          this->Core, SLOT(setColorShadingCubic()));

  // Update colors after an option changes
  connect(this->Core, SIGNAL(coloringChanged()),
          this->Core, SLOT(updateResultScoreIndicators()));
  connect(this->Core, SIGNAL(coloringChanged()),
          this->UI.resultView, SLOT(Refresh()));
  connect(this->Core, SIGNAL(coloringChanged()),
          this->UI.scoreView, SLOT(Refresh()));
  connect(this->Core, SIGNAL(coloringChanged()),
          this->UI.timeline, SLOT(UpdateColors()));

  // Set up UI action signal/slot connections
  connect(this->UI.actionQueryNew, SIGNAL(triggered()),
          this, SLOT(showQueryNewDialog()));
  connect(this->UI.actionQueryEdit, SIGNAL(triggered()),
          this, SLOT(showQueryEditDialog()));
  connect(this->UI.actionQueryOpen, SIGNAL(triggered()),
          this, SLOT(showQueryOpenDialog()));
  connect(this->UI.actionQuerySave, SIGNAL(triggered()),
          this->Core, SLOT(saveQueryPlan()));
  connect(this->UI.actionFindResult, SIGNAL(triggered()),
          this, SLOT(findResult()));
  connect(this->UI.actionQuerySaveResults, SIGNAL(triggered()),
          this->Core, SLOT(saveResults()));
  connect(this->UI.actionGroundTruthOpen, SIGNAL(triggered()),
          this, SLOT(showGroundTruthOpenDialog()));
  connect(this->UI.actionQueryRefine, SIGNAL(triggered()),
          this, SLOT(refineResults()));
  connect(this->UI.actionViewBestResults, SIGNAL(triggered()),
          this, SLOT(showBestClips()));
  connect(this->UI.actionGenerateReport, SIGNAL(triggered()),
          this, SLOT(generateReport()));
  connect(this->UI.actionExportIqrModel, SIGNAL(triggered()),
          this->Core, SLOT(exportIqrModel()));
  connect(this->UI.actionExportKml, SIGNAL(triggered()),
          this, SLOT(exportKml()));

  this->setEnabledSignal(this->UI.actionQuerySave, this->Core,
                         SIGNAL(queryPlanAvailabilityChanged(bool)));
  this->setEnabledSignal(this->UI.actionQuerySaveResults, this->Core,
                         SIGNAL(queryResultsAvailabilityChanged(bool)));
  this->setEnabledSignal(this->UI.actionViewBestResults, this->Core,
                         SIGNAL(queryResultsAvailabilityChanged(bool)));
  this->setEnabledSignal(this->UI.actionExportIqrModel, this->Core,
                         SIGNAL(queryIqrModelAvailabilityChanged(bool)));

  connect(this->UI.actionLayerAddFile, SIGNAL(triggered()),
          this, SLOT(showLayerAddFileDialog()));
  connect(this->UI.actionConfigure, SIGNAL(triggered()),
          this, SLOT(showConfigureDialog()));

  connect(this->UI.actionViewPagePrevious, SIGNAL(triggered()),
          this, SLOT(resultsPageBack()));
  connect(this->UI.actionViewPageNext, SIGNAL(triggered()),
          this, SLOT(resultsPageForward()));

  // Set up core->UI notification signal/slot connections
  connect(this->Core, SIGNAL(processingQuery(vvQueryInstance)),
          this, SLOT(showQueryClip(vvQueryInstance)));
  connect(this->Core, SIGNAL(resultsUpdated()),
          this, SLOT(refreshResults()));
  connect(this->Core, SIGNAL(resultSetComplete(bool, bool)),
          this, SLOT(acceptResultSet(bool, bool)));

  // Set up user actions
  this->UserActions = new vqUserActions(this->Core);

  connect(this->Core, SIGNAL(selected(QList<vtkVgNodeBase*>)),
          this->UserActions, SLOT(Select(QList<vtkVgNodeBase*>)));
  connect(this->Core, SIGNAL(activated(vtkVgNodeBase&)),
          this->UserActions, SLOT(Activate(vtkVgNodeBase&)));

  // Since the user actions object maintains pointers into the scene graph
  // for selection purposes, it must be told those pointers are no longer
  // valid when we finish a query and the old scene graph gets thrown away
  connect(this->Core, SIGNAL(displayResultsReset()),
          this->UserActions, SLOT(Reset()));

  // Node picking via tree view
  this->connectTreeWidget(this->UI.resultView);
  this->connectTreeWidget(this->UI.scoreView);
  this->connectTreeWidget(this->UI.groundTruthView);

  // Hook up tree visibility controls
  connect(this->UI.actionViewResultsHideAll, SIGNAL(triggered()),
          this->UI.resultView, SLOT(HideAllItems()));
  connect(this->UI.actionViewResultsShowAll, SIGNAL(triggered()),
          this->UI.resultView, SLOT(ShowAllItems()));
  connect(this->UI.actionViewResultsShowHidden, SIGNAL(toggled(bool)),
          this->UI.resultView, SLOT(SetShowHiddenItems(bool)));

  connect(this->UI.actionViewRefinementHideAll, SIGNAL(triggered()),
          this->UI.scoreView, SLOT(HideAllItems()));
  connect(this->UI.actionViewRefinementShowAll, SIGNAL(triggered()),
          this->UI.scoreView, SLOT(ShowAllItems()));
  connect(this->UI.actionViewRefinementShowHidden, SIGNAL(toggled(bool)),
          this->UI.scoreView, SLOT(SetShowHiddenItems(bool)));

  connect(this->UI.actionViewGroundTruthHideAll, SIGNAL(triggered()),
          this->UI.groundTruthView, SLOT(HideAllItems()));
  connect(this->UI.actionViewGroundTruthShowAll, SIGNAL(triggered()),
          this->UI.groundTruthView, SLOT(ShowAllItems()));
  connect(this->UI.actionViewGroundTruthShowHidden, SIGNAL(toggled(bool)),
          this->UI.groundTruthView, SLOT(SetShowHiddenItems(bool)));

  // Keep the trees in sync
  connect(this->UI.resultView, SIGNAL(ItemsChanged()),
          this->UI.scoreView, SLOT(Refresh()));
  connect(this->UI.scoreView, SIGNAL(ItemsChanged()),
          this->UI.resultView, SLOT(Refresh()));

  connect(this->UserActions, SIGNAL(UpdateView()),
          this->Core, SLOT(updateSources()));
  connect(this->UserActions, SIGNAL(UpdateView()),
          this->Core, SLOT(updateLOD()));
  connect(this->UserActions, SIGNAL(UpdateView()),
          this->Core, SLOT(updateLayoutStackBarLocations()));

  // Result info
  connect(this->Core, SIGNAL(selected(QList<vtkVgNodeBase*>)),
          this, SLOT(showResultDetails(QList<vtkVgNodeBase*>)));

  // Result filter
  connect(this->UI.actionFilterResults, SIGNAL(triggered()),
          this, SLOT(setResultFilters()));

  // Timeline selection
  connect(this->Core, SIGNAL(selected(QList<vtkVgNodeBase*>)),
          this->UI.timeline, SLOT(SelectNodes(QList<vtkVgNodeBase*>)));

  connect(this->UI.timeline, SIGNAL(SelectedNodes(QList<vtkVgNodeBase*>)),
          this->Core, SLOT(selectNodes(QList<vtkVgNodeBase*>)));
  connect(this->UI.timeline, SIGNAL(ActivatedNode(vtkVgNodeBase&)),
          this->Core, SLOT(activateNode(vtkVgNodeBase&)));

  // Video player
  connect(this->Core, SIGNAL(selected(QList<vtkVgNodeBase*>)),
          this->UI.videoPlayer,
          SLOT(onExternalSelect(QList<vtkVgNodeBase*>)));

  connect(this->UserActions, SIGNAL(Play(vtkVgVideoNode&)),
          this->UI.videoPlayer, SLOT(onExternalPlay(vtkVgVideoNode&)));
  connect(this->UserActions, SIGNAL(Stop(vtkVgVideoNode&)),
          this->UI.videoPlayer, SLOT(onExternalStop(vtkVgVideoNode&)));
  connect(this->UserActions, SIGNAL(Pause(vtkVgVideoNode&)),
          this->UI.videoPlayer, SLOT(onExternalPause(vtkVgVideoNode&)));

  connect(this->Core, SIGNAL(scrolledForward(vtkVgNodeBase&, bool*)),
          this->UserActions, SLOT(ScrollForward(vtkVgNodeBase&, bool*)));
  connect(this->Core, SIGNAL(scrolledBackward(vtkVgNodeBase&, bool*)),
          this->UserActions, SLOT(ScrollBackward(vtkVgNodeBase&, bool*)));

  connect(this->Core, SIGNAL(FocusChanged(vtkVgNodeBase&)),
          this->UserActions, SLOT(FocusTo(vtkVgNodeBase&)));
  connect(this->Core, SIGNAL(FocusChanged(vtkVgVideoNode&)),
          this->UserActions, SLOT(FocusTo(vtkVgVideoNode&)));

  connect(this->UserActions, SIGNAL(Prev(vtkVgVideoNode&)),
          this->UI.videoPlayer, SLOT(onActionPrev()));
  connect(this->UserActions, SIGNAL(Next(vtkVgVideoNode&)),
          this->UI.videoPlayer, SLOT(onActionNext()));
  connect(this->UserActions, SIGNAL(Prev(vtkVgVideoNode&)),
          this->Core, SLOT(update()));
  connect(this->UserActions, SIGNAL(Next(vtkVgVideoNode&)),
          this->Core, SLOT(update()));

  connect(this->Core, SIGNAL(displayResultsReset()),
          this->UI.videoPlayer, SLOT(reset()));

  connect(this->UserActions, SIGNAL(Play(vtkVgVideoNode&)),
          this->Core, SIGNAL(playedOrStopped(vtkVgVideoNode&)));
  connect(this->UserActions, SIGNAL(Stop(vtkVgVideoNode&)),
          this->Core, SIGNAL(playedOrStopped(vtkVgVideoNode&)));

  connect(this->UserActions, SIGNAL(Play(vtkVgVideoNode&)),
          this->Core, SLOT(update()));
  connect(this->UserActions, SIGNAL(Play(vtkVgVideoNode&)),
          this->Core, SLOT(renderLoopOn()));
  connect(this->UserActions, SIGNAL(Stop(vtkVgVideoNode&)),
          this->Core, SLOT(update()));
  connect(this->UserActions, SIGNAL(Stop(vtkVgVideoNode&)),
          this->Core, SLOT(renderLoopOff()));
  connect(this->UserActions, SIGNAL(Pause(vtkVgVideoNode&)),
          this->Core, SLOT(update()));
  connect(this->UserActions, SIGNAL(Pause(vtkVgVideoNode&)),
          this->Core, SLOT(renderLoopOff()));

  connect(this->UserActions, SIGNAL(Play(vtkVgVideoNode&)),
          this->UserActions, SLOT(CenterAndZoomTo(vtkVgVideoNode&)));
  connect(this->UserActions, SIGNAL(Stop(vtkVgVideoNode&)),
          this->UserActions, SLOT(UnZoom()));

  connect(this->UI.videoPlayer, SIGNAL(played(vtkVgVideoNode&)),
          this->Core, SIGNAL(playedOrStopped(vtkVgVideoNode&)));
  connect(this->UI.videoPlayer, SIGNAL(stopped(vtkVgVideoNode&)),
          this->Core, SIGNAL(playedOrStopped(vtkVgVideoNode&)));

  connect(this->UI.videoPlayer, SIGNAL(played(vtkVgVideoNode&)),
          this->Core, SLOT(update()));
  connect(this->UI.videoPlayer, SIGNAL(played(vtkVgVideoNode&)),
          this->Core, SLOT(renderLoopOn()));
  connect(this->UI.videoPlayer, SIGNAL(paused(vtkVgVideoNode&)),
          this->Core, SLOT(update()));
  connect(this->UI.videoPlayer, SIGNAL(paused(vtkVgVideoNode&)),
          this->Core, SLOT(renderLoopOff()));
  connect(this->UI.videoPlayer, SIGNAL(stopped(vtkVgVideoNode&)),
          this->Core, SLOT(update()));
  connect(this->UI.videoPlayer, SIGNAL(stopped(vtkVgVideoNode&)),
          this->Core, SLOT(renderLoopOff()));

  connect(this->UI.videoPlayer, SIGNAL(played(vtkVgVideoNode&)),
          this->UserActions, SLOT(CenterAndZoomTo(vtkVgVideoNode&)));
  connect(this->UI.videoPlayer, SIGNAL(stopped(vtkVgVideoNode&)),
          this->UserActions, SLOT(UnZoom()));

  connect(this->UI.videoPlayer, SIGNAL(videoPlaying(vtkVgNodeBase&)),
          this->Core,
          SLOT(updateStackLayoutForVideoPlaying(vtkVgNodeBase&)));
  connect(this->UI.videoPlayer, SIGNAL(videoStopped(vtkVgNodeBase&)),
          this->Core, SLOT(videoStopped(vtkVgNodeBase&)));

  connect(this->UI.videoPlayer,
          SIGNAL(externalOpenRequested(QUrl, QString, double)),
          this->Core, SLOT(openExternal(QUrl, QString, double)));

  // Fill ground truth event type combo (0 == all types)
  foreach (const vqEventInfo& ei, vqEventInfo::types())
    this->UI.groundTruthEventType->addItem(ei.Name, ei.Type);

  connect(this->UI.groundTruthEventType,
          SIGNAL(currentIndexChanged(int)),
          this, SLOT(setGroundTruthEventType(int)));

  connect(this->Core, SIGNAL(queryResultsAvailabilityChanged(bool)),
          this, SLOT(updateTrackingClipViewer(bool)));

  // Control visibility of items in context view.
  connectCoreDisplayToggle(this->UI.actionShowVideoClipsOnContext,
                           setShowVideoClipsOnContext);
  connectCoreDisplayToggle(this->UI.actionShowOutlinesOnContext,
                           setShowVideoOutlinesOnContext);
  // Visibility of tracks on context
  connectCoreDisplayToggle(this->UI.actionShowTracksOnContext,
                           setShowTracksOnContext);

  // Set if we should zoom when playing a video clip
  connectDisplayToggle(this->UI.actionAutoZoomOnPlay,
                       this->UserActions, SetAutoZoom);

  // Update video player title.
  connect(this->UI.videoPlayer, SIGNAL(currentVideoChanged(vtkVgVideoNode*)),
          this, SLOT(updateVideoPlayerTitle(vtkVgVideoNode*)));

  connect(this->Core, SIGNAL(initialRefinementRequested()),
          this->UI.actionQueryRefine, SIGNAL(triggered()));

  // Connect video player Qf
  connect(this->UI.videoPlayer,
          SIGNAL(queryFormulationRequested(vvProcessingRequest, long long)),
          this, SLOT(formulateQuery(vvProcessingRequest, long long)));

  // Load application settings
  this->reloadConfiguration();

  // Set UI dock widget corner bindings
#define SET_DOCK_CORNER(_c, _d) do { \
  const int v = settings.value(#_c, _d).toInt(); \
  this->setCorner(Qt::_c##Corner, static_cast<Qt::DockWidgetArea>(v)); \
  } while (0)

  QSettings settings;
  settings.beginGroup("DockCorners");
  SET_DOCK_CORNER(TopLeft, Qt::TopDockWidgetArea);
  SET_DOCK_CORNER(TopRight, Qt::TopDockWidgetArea);
  SET_DOCK_CORNER(BottomLeft, Qt::BottomDockWidgetArea);
  SET_DOCK_CORNER(BottomRight, Qt::BottomDockWidgetArea);

  // Set up UI state persistence and restore previous state
  this->UiState.mapGeometry("Window/geometry", this);
  this->UiState.mapState("Window/state", this);

  this->UiState.mapChecked("ContextView/ShowVideoFrames",
                           this->UI.actionShowVideoClipsOnContext);
  this->UiState.mapChecked("ContextView/ShowVideoOutlines",
                           this->UI.actionShowOutlinesOnContext);
  this->UiState.mapChecked("ContextView/ShowTracks",
                           this->UI.actionShowTracksOnContext);

  this->UiState.restore();

  // Start application core
  this->Core->start();
}

//-----------------------------------------------------------------------------
vqApplication::~vqApplication()
{
#ifdef ENABLE_QTTESTING
  delete this->TestUtility;
#endif
  delete this->Core;
  delete this->UserActions;
}

//-----------------------------------------------------------------------------
void vqApplication::closeEvent(QCloseEvent* event)
{
  this->UiState.save();
  QMainWindow::closeEvent(event);
}

//-----------------------------------------------------------------------------
void vqApplication::refreshResults()
{
  this->UI.resultView->SetLookupTable(this->Core->getLookupTable());
  this->UI.resultView->Initialize(this->Core->queryResultNodes(),
                                  false, true, true);

  // always update score view (in case score results have been cleared)
  this->UI.scoreView->SetLookupTable(this->Core->getLookupTable());
  this->UI.scoreView->Initialize(this->Core->scoringRequestNodes(),
                                 true, true);

  this->UI.groundTruthView->SetLookupTable(this->Core->getLookupTable());
  this->UI.groundTruthView->Initialize(this->Core->groundTruthNodes(),
                                       false, false, false, true);

  QList<vtkVgVideoNode*> nodes;
  nodes.append(this->Core->queryResultNodes());
  nodes.append(this->Core->groundTruthNodes());
  this->UI.timeline->SetLookupTable(this->Core->getLookupTable());
  this->UI.timeline->Initialize(nodes);
}

//-----------------------------------------------------------------------------
void vqApplication::acceptResultSet(bool haveSession,
                                    bool scoreRequestedResults)
{
  this->refreshResults();

  if (scoreRequestedResults)
    {
    this->UI.actionQueryRefine->setEnabled(true);

    this->ResultPageOffset = 0;
    this->updateResultPageControls();
    }
  else if (haveSession)
    {
    // Go ahead and request refinement; we don't need to actually do so
    int resultsToScore = vqSettings().iqrRefinementSetSize();
    this->Core->requestRefinement(resultsToScore);
    }
}

//-----------------------------------------------------------------------------
void vqApplication::resultsPageBack()
{
  if (this->ResultPageOffset > 0)
    {
    const int pageSize = vqSettings().resultPageCount();
    this->ResultPageOffset = qMax(0, this->ResultPageOffset - pageSize);

    this->Core->displayResults(pageSize, this->ResultPageOffset);
    this->Core->updateScene();

    this->updateResultPageControls();
    }
}

//-----------------------------------------------------------------------------
void vqApplication::resultsPageForward()
{
  const int pageSize = vqSettings().resultPageCount();
  const int newOffset = this->ResultPageOffset + pageSize;
  const int resultCount = this->Core->resultCount();
  if (newOffset < resultCount)
    {
    this->ResultPageOffset = newOffset;

    this->Core->displayResults(pageSize, this->ResultPageOffset);
    this->Core->updateScene();

    this->updateResultPageControls();
    }
}

//-----------------------------------------------------------------------------
void vqApplication::updateResultPageControls()
{
  const int total = this->Core->resultCount();
  const int last = this->ResultPageOffset + vqSettings().resultPageCount();

  this->UI.actionViewPagePrevious->setEnabled(this->ResultPageOffset > 0);
  this->UI.actionViewPageNext->setEnabled(last < total);
}

//-----------------------------------------------------------------------------
void vqApplication::reloadConfiguration()
{
  vqSettings settings;
  this->Core->setVideoProviders(settings.videoProviders());
  this->Core->setQueryServer(settings.queryServerUri());
}

//-----------------------------------------------------------------------------
void vqApplication::showQueryNewDialog()
{
  if (!this->Core->canIssueQuery())
    return;

  this->cancelNewQuery();
  this->NewQueryDialog.reset(
    new vqQueryDialog(this->Core, this->useAdvancedUI(), this));
  connect(this->NewQueryDialog.data(), SIGNAL(accepted()),
          this, SLOT(executeNewQuery()));
  connect(this->NewQueryDialog.data(), SIGNAL(rejected()),
          this, SLOT(cancelNewQuery()));
  connect(this->NewQueryDialog.data(),
          SIGNAL(readyToProcessDatabaseVideoQuery(vvQueryInstance)),
          this->Core, SLOT(processDatabaseVideoQuery(vvQueryInstance)));

  this->NewQueryDialog->open();
}

//-----------------------------------------------------------------------------
void vqApplication::showQueryEditDialog()
{
  if (!this->Core->canIssueQuery())
    return;

  this->cancelNewQuery();
  this->NewQueryDialog.reset(
    new vqQueryDialog(this->Core, this->useAdvancedUI(), this));
  connect(this->NewQueryDialog.data(), SIGNAL(accepted()),
          this, SLOT(executeNewQuery()));
  connect(this->NewQueryDialog.data(), SIGNAL(rejected()),
          this, SLOT(cancelNewQuery()));

  this->NewQueryDialog->loadPersistentQuery();
  this->NewQueryDialog->open();
}

//-----------------------------------------------------------------------------
void vqApplication::showQueryClip(vvQueryInstance query)
{
  const vvSimilarityQuery* sq = query.similarityQuery();
  if (!sq || sq->StreamIdLimit.empty())
    return;

  QList<vvDescriptor> descriptors;

  double startTime = -1;
  double endTime = -1;

  for (size_t n = 0; n < sq->Descriptors.size(); ++n)
    {
    if (sq->Descriptors[n].Region.empty())
      {
      continue;
      }

    double sTime = (*sq->Descriptors[n].Region.begin()).TimeStamp.Time;
    double eTime = (*sq->Descriptors[n].Region.rbegin()).TimeStamp.Time;

    if (n == 0 || sTime < startTime)
      {
      startTime = sTime;
      }
    if (n == 0 || eTime > endTime)
      {
      endTime = eTime;
      }

    descriptors.append(sq->Descriptors[n]);
    }

  if (startTime != -1)
    {
    vqSettings settings;
    startTime -= (1e06 * settings.resultClipPadding());
    endTime += (1e06 * settings.resultClipPadding());
    }

  if (this->UI.queryClip->setVideoUri(qtUrl(sq->StreamIdLimit)))
    {
    this->UI.queryClip->setDefaultEventColor(Qt::green);
    this->UI.queryClip->setTimeRange(startTime, endTime);

    this->UI.queryClip->setDescriptors(descriptors, false);
    }
}

//-----------------------------------------------------------------------------
void vqApplication::executeNewQuery()
{
  vvQueryInstance query = this->NewQueryDialog->query();

  // Clear query clip and user region, and release dialog
  this->UI.queryClip->reset();
  this->Core->setRegion(vgGeocodedPoly());
  this->NewQueryDialog.reset();

  // Execute the query
  this->UI.actionQueryRefine->setEnabled(false);
  this->Core->processQuery(query);
}

//-----------------------------------------------------------------------------
void vqApplication::cancelNewQuery()
{
  // Clear user region and release dialog
  this->Core->setRegion(vgGeocodedPoly());
  this->NewQueryDialog.reset();
}

//-----------------------------------------------------------------------------
void vqApplication::formulateQuery(
  vvProcessingRequest request, long long initialTime)

{
  // Is there an active query session?
  if (this->Core->isQuerySessionActive())
    {
    // Due to current limitations in one of our VVQS back-ends, we do not allow
    // concurrent query sessions. Therefore, in order to proceed, we will have
    // to shut down any currently active query session. Before doing so,
    // confirm with the user that this is okay...
    const QString msg =
      "In order to proceed with query formulation, the current query session "
      "must be ended. Further refinement of the current query results will "
      "not be possible.\n\nDo you want to continue?";

    int returnVal =
      QMessageBox::warning(this, qAppName(), msg,
                           QMessageBox::Yes | QMessageBox::No);
    CHECK_ARG(returnVal == QMessageBox::Yes);
    }

  this->showQueryNewDialog();
  this->NewQueryDialog->initiateDatabaseQuery(
    request.VideoUri, request.StartTime, request.EndTime, initialTime);
}

//-----------------------------------------------------------------------------
void vqApplication::showQueryOpenDialog()
{
  if (!this->Core->canIssueQuery())
    return;

  QString fileName = vgFileDialog::getOpenFileName(
    this, "Select saved query results to open...", QString(),
    "VisGUI saved queries (*.vqr *.vqp *.vqpx *.vqrx);;"
    "VisGUI Query Plans (*.vqp *.vqpx);;"
    "VisGUI Query Results (*.vqr *.vqrx);;"
    "All files (*)");
  if (!fileName.isEmpty())
    {
    QUrl uri = QUrl::fromLocalFile(fileName);
    this->UI.actionQueryRefine->setEnabled(false);
    this->UI.queryClip->reset();
    this->Core->loadQuery(uri, vqCore::SavePersistent);
    }
}

//-----------------------------------------------------------------------------
void vqApplication::showGroundTruthOpenDialog()
{
  if (!this->Core->canIssueQuery())
    return;

  QString fileName = vgFileDialog::getOpenFileName(
                       this, "Select ground truth results to open...",
                       QString(), "VisGUI Query Results (*.vqr);;"
                                  "All files (*)");
  if (!fileName.isEmpty())
    {
    QUrl uri = QUrl::fromLocalFile(fileName);
    this->Core->loadQuery(uri, vqCore::GroundTruth);
    }
}

//-----------------------------------------------------------------------------
void vqApplication::setGroundTruthEventType(int index)
{
  int type = this->UI.groundTruthEventType->itemData(index).toInt();
  this->Core->setGroundTruthEventType(type);
}

//-----------------------------------------------------------------------------
void vqApplication::setResultFilters()
{
  vqResultFilterDialog dlg;

  dlg.setFilter(this->Core->resultFilter());

  if (dlg.exec() == QDialog::Accepted)
    {
    vqResultFilter filter = dlg.filter();
    this->Core->setResultFilter(filter);
    this->UI.actionFilterResults->setChecked(!filter.isNoop());
    }
}

//-----------------------------------------------------------------------------
void vqApplication::refineResults()
{
  this->UI.actionQueryRefine->setEnabled(false);
  this->Core->refineQuery();
}

//-----------------------------------------------------------------------------
void vqApplication::showLayerAddFileDialog()
{
  QString fileName = vgFileDialog::getOpenFileName(
                       this, "Layer file to add...", QString(),
                       "Kitware Structured Text (*.kst);;"
                       "Georegistered TIFF (*.tif *.tiff);;"
                       "All files (*)");
  if (!fileName.isEmpty())
    {
    QUrl uri = QUrl::fromLocalFile(fileName);
    this->Core->addRasterLayer(uri);
    }
}

//-----------------------------------------------------------------------------
void vqApplication::showConfigureDialog()
{
  vqConfigureDialog dialog(this);

  connect(&dialog, SIGNAL(scoreGradientChanged()),
          this->Core, SLOT(reloadColorGradient()));
  connect(&dialog, SIGNAL(predefinedQueryUriChanged()),
          this, SLOT(reloadPredefinedQueryCache()));

  if (dialog.exec() == QDialog::Accepted)
    this->reloadConfiguration();
}

//-----------------------------------------------------------------------------
void vqApplication::setupTrackingClipViewer()
{
  if (this->TrackingClipViewer)
    {
    return;
    }

  this->TrackingClipViewer.reset(new vqTrackingClipViewer(this));

  connect(this->TrackingClipViewer.data(),
          SIGNAL(rejected()),
          SLOT(trackingClipViewerClosed()));

  connect(this->TrackingClipViewer.data(),
          SIGNAL(ClipSelected(int)),
          SLOT(selectedClip(int)));

  connect(this->TrackingClipViewer.data(),
          SIGNAL(ClipActivated(int)),
          SLOT(activatedClip(int)));

  connect(this->TrackingClipViewer.data(),
          SIGNAL(ClipRatingChanged(int, int)),
          SLOT(setClipRating(int, int)));

  connect(this->TrackingClipViewer.data(),
          SIGNAL(RequestNextClips(vtkIdType, int)),
          SLOT(showNextTrackingClips(vtkIdType, int)));
}

//-----------------------------------------------------------------------------
void vqApplication::showTrackingClips(QList<vtkVgVideoNode*> nodes)
{
  this->setupTrackingClipViewer();
  this->Core->showTrackingClips(nodes, this->TrackingClipViewer.data());
}

//-----------------------------------------------------------------------------
void vqApplication::showNextTrackingClips(vtkIdType previousId, int count)
{
  this->Core->showNextTrackingClips(previousId, count,
                                    this->TrackingClipViewer.data());
}

//-----------------------------------------------------------------------------
void vqApplication::setClipRating(int id, int rating)
{
  this->setUserScore(id, rating);
  this->UI.resultView->Refresh();
  this->UI.scoreView->Refresh();
}

//-----------------------------------------------------------------------------
void vqApplication::setUserScore(long long iid, int score)
{
  this->Core->setUserScore(iid, score);

  QList<const vvQueryResult*> results;
  results.append(this->Core->getResult(iid));

  this->UI.resultInfo->setResults(results);

  if (this->TrackingClipViewer)
    {
    this->TrackingClipViewer->RefreshClips();
    }
}

//-----------------------------------------------------------------------------
void vqApplication::setResultNote(long long iid, const QString& note)
{
  this->Core->setResultNote(iid, note);

  QList<const vvQueryResult*> results;
  results.append(this->Core->getResult(iid));

  this->UI.resultInfo->setResults(results);
}

//-----------------------------------------------------------------------------
void vqApplication::setResultStarred(long long iid, bool starred)
{
  // Need this trampoline signal to work around cognitive dissonance between
  // vqTreeView's 'long long' and vqCore's 'ResultId' data types for the
  // Instance ID
  this->Core->setResultStarred(iid, starred);
}

//-----------------------------------------------------------------------------
void vqApplication::showBestClips()
{
  this->setupTrackingClipViewer();
  this->Core->showBestClips(this->TrackingClipViewer.data());
}

//-----------------------------------------------------------------------------
void vqApplication::updateTrackingClipViewer(bool keepResults)
{
  if (!this->TrackingClipViewer)
    {
    return;
    }

  if (keepResults)
    {
    // Remove any looping clips that aren't in the new result set, and update
    // the rank labels if the result rankings have changed.
    this->TrackingClipViewer->UpdateClips(this->Core);
    }
  else
    {
    // Clear all tracking clips.
    this->TrackingClipViewer->close();
    }
}

//-----------------------------------------------------------------------------
void vqApplication::selectedClip(int id)
{
  this->Core->selectResult(id);
}

//-----------------------------------------------------------------------------
void vqApplication::activatedClip(int id)
{
  this->Core->activateResult(id);
}

//-----------------------------------------------------------------------------
void vqApplication::trackingClipViewerClosed()
{
  this->TrackingClipViewer.reset();
}

//-----------------------------------------------------------------------------
void vqApplication::addLayer(QUrl uri)
{
  this->Core->addRasterLayer(uri);
}

//-----------------------------------------------------------------------------
void vqApplication::initializeTesting(const qtCliArgs* args)
{
#ifdef ENABLE_QTTESTING
  this->TestUtility = new pqCoreTestUtility(this, this->UI.renderFrame);
  this->TestUtility->SetDataRootEnvironment("VISGUI_DATA_ROOT");
  this->TestUtility->AddTestingMenu(this->menuBar());

  this->TestUtility->ParseCommandLine(*args);
  this->TestUtility->ProcessCommandLine();
#endif

  Q_UNUSED(args);
}

//-----------------------------------------------------------------------------
void vqApplication::exportSelectedResults(const QString& exporterId)
{
  this->Core->exportResults(this->UI.resultView->GetSelectedNodes(),
                            exporterId);
}

//-----------------------------------------------------------------------------
void vqApplication::exportStarredResults(const QString& exporterId)
{
  this->Core->exportResults(this->UI.resultView->GetStarredNodes(),
                            exporterId);
}

//-----------------------------------------------------------------------------
void vqApplication::exportAllResults(const QString& exporterId)
{
  this->Core->exportResults(exporterId);
}

//-----------------------------------------------------------------------------
void vqApplication::showResultDetails(QList<vtkVgNodeBase*> nodes)
{
  QList<const vvQueryResult*> results;

  // Get list of results corresponding to these nodes
  foreach (vtkVgNodeBase* node, nodes)
    {
    if (vtkVgVideoNode* vnode = vtkVgVideoNode::SafeDownCast(node))
      {
      if (vnode->GetIsNormalResult() || vnode->GetIsRefinementResult())
        {
        results << this->Core->getResult(vnode->GetInstanceId());
        }
      else
        {
        // Not a normal or refinement result - must be ground truth
        results << this->Core->getGroundTruthResult(vnode->GetInstanceId());
        }
      }
    }

  // Update result info
  this->UI.resultInfo->setResults(results);
}

//-----------------------------------------------------------------------------
void vqApplication::updateVideoPlayerTitle(vtkVgVideoNode* videoNode)
{
  QString title("Video Player");

  if (!videoNode)
    {
    this->UI.videoPlayerDock->setWindowTitle(title);
    return;
    }

  title += QString(" - Result %1").arg(
             QString::number(videoNode->GetInstanceId()));
  this->UI.videoPlayerDock->setWindowTitle(title);
}

//-----------------------------------------------------------------------------
void vqApplication::findResult()
{
  bool ok = false;
  int id = QInputDialog::getInt(this, "Find Result", "Instance Id:",
                                0, 0, INT_MAX, 1, &ok);
  if (ok)
    {
    if (!this->Core->findResult(id))
      {
      QMessageBox::information(0, QString(), "No results found.");
      }
    }
}

//-----------------------------------------------------------------------------
void vqApplication::reloadPredefinedQueryCache()
{
  vqPredefinedQueryCache::reload();
}

//-----------------------------------------------------------------------------
void vqApplication::generateReport()
{
  vvGenerateReportDialog dlg(this);
  if (dlg.exec() == QDialog::Accepted)
    {
    this->Core->generateReport(dlg.outputPath(), dlg.generateVideo());
    }
}

//-----------------------------------------------------------------------------
void vqApplication::exportKml()
{
  QString path =
    vgFileDialog::getSaveFileName(
      this, "Location of KML export", QString(), "KML File (*.kml);;");

  if (!path.isEmpty())
    {
    this->Core->exportKml(path);
    }
}
