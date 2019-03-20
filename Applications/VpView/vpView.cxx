/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "ui_vpView.h"

#include "vpApplication.h"
#include "vpConfigureDialog.h"
#include "vpCreateEventDialog.h"
#include "vpExternalProcessDialog.h"
#include "vpFilterTypeDelegate.h"
#include "vpMergeTracksDialog.h"
#include "vpQtSceneUtils.h"
#include "vpQtViewer3dDialog.h"
#include "vpQtViewer3dWidget.h"
#include "vpSelectTimeIntervalDialog.h"
#include "vpSettings.h"
#include "vpTimelineDialog.h"
#include "vpTrackColorDialog.h"
#include "vpVersion.h"
#include "vpView.h"
#include "vpViewCore.h"
#include "vtkVpTrackModel.h"

#ifdef VISGUI_USE_SUPER3D
#include "vpSuperResWidget.h"
#endif

#include <vtkVgEvent.h>
#include <vtkVgEventFilter.h>
#include <vtkVgEventModel.h>
#include <vtkVgTemporalFilters.h>
#include <vtkVgTrackRepresentationBase.h>
#include <vtkVgTrackTypeRegistry.h>

#include <QVTKWidget.h>
#include <vtkNew.h>
#include <vtkRenderer.h>

#ifdef _WIN32
#include <vtkWin32ProcessOutputWindow.h>
#endif

#ifdef ENABLE_QTTESTING
#include "pqCoreTestUtility.h"
#endif

#include <vgAboutAction.h>
#include <vgFileDialog.h>
#include <vgUnixTime.h>

#include <qtCliArgs.h>
#include <qtScopedValueChange.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QSlider>
#include <QSplitter>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

//-----------------------------------------------------------------------------
class vpView::vtkInternal
{
public:
  ~vtkInternal()
    {
    }

  Ui::vpMainWindow UI;
  const qtCliArgs* CliArgs;

  QVTKWidget* QVTKMainRenderWidget;

  QStringList ProjectFileNames;
  QString ConfigFileName;

  QComboBox* eventTypeFilter;
  bool DisableFilterPresetUpdates;

  QComboBox* executeModeCombo;

  QSignalMapper* EmbeddedPipelineMapper;

  QLabel* Coordinates;
  QLabel* Gsd;
  QLabel* FrameFileName;
  QLabel* FrameDate;
  QLabel* FrameTime;
  QLabel* ActivityCount;
  QLabel* EventCount;
  QLabel* TrackCount;

  vpSettings::CoordDisplayMode CoordDisplayMode;
  vpSettings::ImageFilteringMode ImageFilteringMode;

  vtkVgTimeStamp MinFrameOffset;

  QWidget* contextLOD;

  bool TrackUpdateInProgress;
  QTimer* TrackUpdateTimer;

  bool DataLoaded;
  bool Seeking;
  bool Selecting;

  int RenderWindowMouseCoords[2];
};

// Helper to set up icons
//-----------------------------------------------------------------------------
void setupIcon(QAction* action, QString name)
{
  action->setIcon(qtUtil::standardActionIcon(name));
}

class SetScoped
{
public:
  SetScoped(bool& var) : Var(var)
    {
    this->Var = true;
    }

  ~SetScoped()
    {
    this->Var = false;
    }

private:
  bool& Var;
};


// Constructor
//-----------------------------------------------------------------------------
vpView::vpView()
{
  this->Internal = new vtkInternal;
  this->Internal->MinFrameOffset.SetFrameNumber(10);
  this->Internal->MinFrameOffset.SetTime(5.0e6);
  this->Internal->TrackUpdateInProgress = false;
  this->Internal->DataLoaded = false;
  this->Internal->Seeking = false;
  this->Internal->Selecting = false;
  this->Internal->CliArgs = 0;
  this->Internal->RenderWindowMouseCoords[0] = -1;
  this->Internal->RenderWindowMouseCoords[1] = -1;

  this->Internal->TrackUpdateTimer = new QTimer(this);
  connect(this->Internal->TrackUpdateTimer, SIGNAL(timeout()),
          this, SLOT(updateTracks()));

  this->Internal->UI.setupUi(this);

#ifdef VISGUI_USE_SUPER3D
    {
    QDockWidget* superResDock;
    superResDock = new QDockWidget(this);
    superResDock->setObjectName(QString::fromUtf8("superResDock"));
    QWidget* superDockWidgetContents = new QWidget();
    superDockWidgetContents->setObjectName(
      QString::fromUtf8("dockWidgetContents_4"));
    QVBoxLayout* superVerticalLayout = new QVBoxLayout(superDockWidgetContents);
    superVerticalLayout->setObjectName(QString::fromUtf8("verticalLayout_14"));
    this->SuperResWidget = new vpSuperResWidget(superDockWidgetContents);
    this->SuperResWidget->setObjectName(QString::fromUtf8("superResWidget"));
    superVerticalLayout->addWidget(this->SuperResWidget);
    QSpacerItem* superVerticalSpacer =
      new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    superVerticalLayout->addItem(superVerticalSpacer);
    superResDock->setWidget(superDockWidgetContents);
    this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), superResDock);
    superResDock->setWindowTitle(QApplication::translate("vpMainWindow",
                                 "Super Resolution", 0, QApplication::UnicodeUTF8));
    }
#endif

  // Add dock and toolbar view actions to the view menu
  QAction* insertBefore = this->Internal->UI.menuView->actions()[0];

  foreach (QDockWidget* dock, this->findChildren<QDockWidget*>())
    {
    if (dock->parentWidget() == this)
      {
      this->Internal->UI.menuView->insertAction(insertBefore,
                                                dock->toggleViewAction());
      }
    }
  this->Internal->UI.menuView->insertSeparator(insertBefore);

  foreach (QToolBar* toolbar, this->findChildren<QToolBar*>())
    {
    if (toolbar->parentWidget() == this)
      {
      this->Internal->UI.menuView->insertAction(insertBefore,
                                                toolbar->toggleViewAction());
      }
    }
  this->Internal->UI.menuView->insertSeparator(insertBefore);

  // disable render frame until we have data, because otherwise accidental
  // mouse movement / scrolling could cause crash
  this->Internal->UI.renderFrame->setEnabled(false);
  this->Internal->UI.currentFrame->setEnabled(false);

  this->Core = new vpViewCore();
#ifdef ENABLE_QTTESTING
  this->Core->setAntialiasing(false);
#endif // ENABLE_QTTESTING

  // Disable expensive 3d graph update when the window isn't visible
  this->Core->setGraphRenderingEnabled(false);

  this->loadSettings();

  this->Internal->eventTypeFilter = new QComboBox(this);
  this->Internal->eventTypeFilter->setInsertPolicy(QComboBox::NoInsert);
  this->Internal->eventTypeFilter->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  this->Internal->eventTypeFilter->setEnabled(false);
  this->Internal->DisableFilterPresetUpdates = false;

  this->Core->setupRenderWidget(this->Internal->UI.renderFrame);
  this->Core->setSessionView(this->Internal->UI.sessionView);

  QActionGroup* playbackActions = new QActionGroup(this);
  playbackActions->addAction(this->Internal->UI.actionFastBackward);
  playbackActions->addAction(this->Internal->UI.actionReverse);
  playbackActions->addAction(this->Internal->UI.actionPreviousFrame);
  playbackActions->addAction(this->Internal->UI.actionPause);
  playbackActions->addAction(this->Internal->UI.actionNextFrame);
  playbackActions->addAction(this->Internal->UI.actionPlay);
  playbackActions->addAction(this->Internal->UI.actionFastForward);

  // Set up action icons
  setupIcon(this->Internal->UI.actionFastBackward, "playback-fast-backward");
  setupIcon(this->Internal->UI.actionReverse, "playback-play-reverse");
  setupIcon(this->Internal->UI.actionPreviousFrame, "playback-frame-backward");
  setupIcon(this->Internal->UI.actionPause, "playback-pause");
  setupIcon(this->Internal->UI.actionNextFrame, "playback-frame-forward");
  setupIcon(this->Internal->UI.actionPlay, "playback-play");
  setupIcon(this->Internal->UI.actionFastForward, "playback-fast-forward");

  // Set window title (i.e. add version number) and icon
  QString title("%1 %3");
  title = title.arg(this->windowTitle()).arg(VPVIEW_VERSION_STR);
  this->setWindowTitle(title);
  qtUtil::setApplicationIcon("vpView", this);

  // Set up About dialog
  this->Internal->UI.menuHelp->addAction(new vgAboutAction(this));

  // Set up toolbar widgets
  QWidget* widget;
  QVBoxLayout* layout;
  QLabel* label;

  this->Internal->UI.toolBar->addSeparator();
  widget = new QWidget(this);
  layout = new QVBoxLayout(widget);
  label = new QLabel("Event Filter");
  label->setAlignment(Qt::AlignCenter);
  layout->addWidget(this->Internal->eventTypeFilter);
  layout->addWidget(label);
  this->Internal->UI.toolBar->addWidget(widget);
  this->Internal->UI.toolBar->addAction(this->Internal->UI.actionDisplayFullVolume);
  this->Internal->UI.toolBar->addAction(this->Internal->UI.actionLoop);

  this->Internal->UI.toolBar->addSeparator();
  this->Internal->UI.toolBar->addAction(this->Internal->UI.actionCreateTrack);
  this->Internal->UI.toolBar->addAction(this->Internal->UI.actionCreateEvent);
  this->Internal->UI.toolBar->addAction(this->Internal->UI.actionCreateSceneElement);

  QComboBox* editModeCombo = new QComboBox(this);
  editModeCombo->setObjectName("editModeCombo");
  editModeCombo->addItem("Auto");
  editModeCombo->addItem("Bounding Box");
  editModeCombo->addItem("Polygon");

  connect(editModeCombo, SIGNAL(currentIndexChanged(int)),
          this->Core, SLOT(setRegionEditMode(int)));

  widget = new QWidget(this);
  widget->setObjectName("editModeWidget");
  layout = new QVBoxLayout(widget);
  label = new QLabel("Edit Mode");
  label->setAlignment(Qt::AlignCenter);
  layout->addWidget(editModeCombo);
  layout->addWidget(label);

  this->Internal->UI.toolBar->addWidget(widget);

  this->Internal->UI.toolBar->addSeparator();

  this->Internal->executeModeCombo = new QComboBox(this);
  this->Internal->executeModeCombo->setObjectName("executeModeCombo");
  this->Internal->executeModeCombo->addItem("FSE Recognition", 0);
  this->Internal->executeModeCombo->addItem("Scene Learning", 1);
  this->Internal->executeModeCombo->addItem("Normalcy Anomaly", 2);
  this->Internal->executeModeCombo->addItem("Activity Learning", 3);
  this->Internal->executeModeCombo->setCurrentIndex(2);
  this->Core->setExternalExecuteMode(2);

  this->Internal->UI.toolBar->addWidget(this->Internal->executeModeCombo);
  this->Internal->UI.toolBar->addAction(this->Internal->UI.actionExecuteExternalProcess);

  connect(this->Internal->executeModeCombo, SIGNAL(currentIndexChanged(int)),
          this, SLOT(onExecuteModeChanged(int)));

  this->Internal->UI.toolBar->addSeparator();
  QSlider* contextLODSlider = new QSlider(Qt::Horizontal, this);
  contextLODSlider->setRange(-10, 10);
  contextLODSlider->setTickInterval(1);
  contextLODSlider->setSizePolicy(QSizePolicy::Preferred,
                                  QSizePolicy::Preferred);
  this->Internal->contextLOD =
    vpQtSceneUtils::createContextSlider(contextLODSlider, this);
  this->Internal->UI.toolBar->addWidget(this->Internal->contextLOD);

  this->Internal->UI.toolBar->addSeparator();
  this->Internal->UI.toolBar->addAction(this->Internal->UI.actionAutoUpdate);

  // Set up status bar
  this->Internal->Coordinates = new QLabel;
  this->Internal->Gsd = new QLabel;
  this->Internal->FrameFileName = new QLabel;
  this->Internal->FrameDate = new QLabel;
  this->Internal->FrameTime = new QLabel;
  this->Internal->ActivityCount = new QLabel;
  this->Internal->EventCount = new QLabel;
  this->Internal->TrackCount = new QLabel;
  this->statusBar()->addWidget(this->Internal->Coordinates);
  this->statusBar()->addPermanentWidget(this->Internal->Gsd);
  this->statusBar()->addPermanentWidget(this->Internal->FrameFileName);
  this->statusBar()->addPermanentWidget(this->Internal->FrameDate);
  this->statusBar()->addPermanentWidget(this->Internal->FrameTime);
  this->statusBar()->addPermanentWidget(this->Internal->ActivityCount);
  this->statusBar()->addPermanentWidget(this->Internal->EventCount);
  this->statusBar()->addPermanentWidget(this->Internal->TrackCount);
  this->Internal->Gsd->hide();
  this->Internal->FrameDate->hide();

  // Set the status bar to a good fixed height. If we don't do this, a second
  // layout event may occur after the first time the window is shown (Qt bug).
  this->updateFrameTime();
  this->updateObjectCounts();
  this->statusBar()->setFixedHeight(this->statusBar()->sizeHint().height());

  // Show the frame spin box now so that layout doesn't need to change later
  this->Internal->UI.currentFrame->setFrameNumberRange(0, 0);
  this->Internal->UI.minTime->hide();

  this->CreateEventDialog = 0;
  this->TimelineDialog = new vpTimelineDialog(this);

  // Initialize viewer3d dialog.
  this->Viewer3dDialog = new vpQtViewer3dDialog(this);

  // Graph model widget
  this->GraphModelWidget = this->Internal->UI.graphModelWidget;
  this->GraphModelWidget->setApplicationCore(this->Core);

  connect(this->GraphModelWidget, SIGNAL(eventSelected(int)),
          this, SLOT(selectEvent(int)));

  connect(this->GraphModelWidget, SIGNAL(distanceMeasurementRequested(bool)),
          this->Core, SLOT(setRulerEnabled(bool)));

  connect(this->Core, SIGNAL(geoDistanceMeasured(double)),
          this->GraphModelWidget, SLOT(setMeasuredDistance(double)));

  connect(this->GraphModelWidget, SIGNAL(timeIntervalMeasurementRequested()),
          this, SLOT(pickTimeInterval()));

  connect(this->Core, SIGNAL(graphModelExportRequested(QString)),
          this->GraphModelWidget, SLOT(exportJson(QString)));

  connect(this->Core, SIGNAL(projectProcessed()),
          this, SLOT(onProjectProcessed()));

#ifdef VISGUI_USE_SUPER3D
  // Super res widget
  this->SuperResWidget->initialize(this->Core);

  connect(this->Core, SIGNAL(frameRendered()),
          this->SuperResWidget, SLOT(frameChanged()));
#endif

  // Signals for setting up timeline selections
  connect(this->TimelineDialog, SIGNAL(SelectedObject(int, int)),
          this, SLOT(onTimelineSelectionChanged(int, int)));
  connect(this, SIGNAL(eventSelected(int)),
          this->TimelineDialog, SLOT(SelectEvent(int)));

#ifdef ENABLE_QTTESTING
  this->TestUtility = new pqCoreTestUtility(this, this->Internal->UI.renderFrame);
  this->TestUtility->SetDataRootEnvironment("VISGUI_DATA_ROOT");
  this->TestUtility->AddTestingMenu(this->Internal->UI.menuTools);

  connect(this->TestUtility, SIGNAL(StartedTesting()), SLOT(onTestingStarted()));
  connect(this->TestUtility, SIGNAL(StoppedTesting()), SLOT(onTestingStopped()));
#endif

  // Set up action signals and slots
  connect(this->Internal->UI.actionNewProject, SIGNAL(triggered()),
          this->Core, SLOT(newProject()));
  connect(this->Internal->UI.actionOpenProject, SIGNAL(triggered()),
          this->Core, SLOT(openProject()));
  connect(this->Internal->UI.actionImportProject, SIGNAL(triggered()),
          this, SLOT(importProject()));

  connect(this->Internal->UI.actionExportTracks,
          SIGNAL(triggered()), SLOT(exportTracks()));
  connect(this->Internal->UI.actionExportEvents,
          SIGNAL(triggered()), SLOT(exportEvents()));
  connect(this->Internal->UI.actionExportSceneElements,
          SIGNAL(triggered()), SLOT(exportSceneElements()));
  connect(this->Internal->UI.actionExportFilters,
          SIGNAL(triggered()), SLOT(exportFilters()));

  connect(this->Internal->UI.actionImportFilters,
          SIGNAL(triggered()), this->Core, SLOT(loadFilters()));

  connect(this->Internal->UI.actionExportGraphModel, SIGNAL(triggered()),
          this->GraphModelWidget, SLOT(exportJson()));
  connect(this->Internal->UI.actionImportGraphModel, SIGNAL(triggered()),
          this->GraphModelWidget, SLOT(importJson()));

  connect(this->Internal->UI.actionSaveRenderedImages, SIGNAL(triggered(bool)),
          this, SLOT(onSaveRenderedImages(bool)));

  connect(this->Internal->UI.actionWebExport, SIGNAL(triggered()),
          this, SLOT(onWebExport()));

  connect(this->Internal->UI.actionExportImageTimeStamps, SIGNAL(triggered()),
          this->Core, SLOT(exportImageTimeStampsToFile()));

  connect(this->Internal->UI.actionExit, SIGNAL(triggered()),
          this, SLOT(exitApp()));

  connect(this->Core, SIGNAL(dataLoaded()), this, SLOT(onDataLoaded()));
  connect(this->Core, SIGNAL(dataSetChanged()), this, SLOT(onDataChanged()));
  connect(this->Core, SIGNAL(iconsLoaded()), this, SLOT(onIconsLoaded()));
  connect(this->Core, SIGNAL(overviewLoaded()), this, SLOT(onOverviewLoaded()));
  connect(this->Core, SIGNAL(frameChanged()), this, SLOT(onFrameChange()));
  connect(this->Core, SIGNAL(timeChanged(double)),
          this, SLOT(onTimeChange(double)));
  connect(this->Core, SIGNAL(reachedPlayBoundary()), this, SLOT(onReachedPlayBoundary()));

  connect(this->Core, SIGNAL(reinitialized()), this, SLOT(onReinitialized()));
  connect(this->Core, SIGNAL(frameRendered()), this, SLOT(onFrameRendered()));
  connect(this->Core, SIGNAL(eventFilterChanged()), this, SLOT(onEventFilterChanged()));

  connect(this->Core, SIGNAL(objectInfoUpdateNeeded(bool)),
          this, SLOT(updateInfoWidget(bool)));

  connect(this->Core, SIGNAL(projectVisibilityChanged(int, bool)),
          this->Internal->UI.sessionView, SLOT(SetSessionVisible(int, bool)));

  connect(this->Internal->UI.objectInfoWidget, SIGNAL(ObjectIdChanged(int, int)),
          this, SLOT(updateObject(int, int)));

  connect(this->Internal->UI.objectInfoWidget, SIGNAL(ObjectTypeChanged(int, int)),
          this, SLOT(updateObject(int, int)));

  connect(this->Internal->UI.objectInfoWidget, SIGNAL(TypeColorChanged(int, double*)),
          this, SLOT(updateColorofTracksOfType(int, double*)));

  connect(this->Core, SIGNAL(showStatusMessage(const QString&, int)),
          this->statusBar(), SLOT(showMessage(const QString&, int)));

  connect(this->Core, SIGNAL(timeBasedIndexingDisabled()),
          this, SLOT(disableTimeBasedIndexing()));

  connect(this->Core, SIGNAL(playbackStateChanged()),
          this, SLOT(updatePlaybackState()));

  connect(this->Core, SIGNAL(playbackRateChanged(qreal)),
          this, SLOT(updatePlaybackRate(qreal)));

  // Frame scrubber and time interval controls
  connect(this->Internal->UI.currentFrame,
          SIGNAL(frameNumbersChanged(int, int, bool)),
          SLOT(onFrameUpdate(int, int, bool)));

  connect(this->Internal->UI.currentFrame,
          SIGNAL(timesChanged(double, double, bool)),
          SLOT(onTimeUpdate(double, double, bool)));

  connect(this->Internal->UI.currentFrame,
          SIGNAL(minFrameNumberChanged(int)),
          this->Internal->UI.minFrame,
          SLOT(setValue(int)));

  connect(this->Internal->UI.currentFrame,
          SIGNAL(minTimeChanged(double)),
          SLOT(updateMinTime(double)));

  connect(this->Internal->UI.minFrame,
          SIGNAL(valueChanged(int)),
          this->Internal->UI.currentFrame,
          SLOT(setMinFrameNumber(int)));

  connect(this->Internal->UI.minFrameCheckBox,
          SIGNAL(stateChanged(int)),
          SLOT(onFrameIntervalEnabled(int)));

  // Filters
  connect(this->Internal->eventTypeFilter, SIGNAL(currentIndexChanged(int)),
          this, SLOT(onEventFilterPresetChanged(int)));
  connect(this->Internal->UI.normalcyFilter, SIGNAL(stateChanged(int, bool)),
          this, SLOT(onEventFilterVisibilityChanged()));

  connect(this->Core,
          SIGNAL(filterRegionComplete(vtkPoints*, vtkPoints*,
                                      const vtkVgTimeStamp*,
                                      vtkMatrix4x4*, QString, bool)),
          this,
          SLOT(onSpatialFilterComplete(vtkPoints*, vtkPoints*,
                                       const vtkVgTimeStamp*,
                                       vtkMatrix4x4*, QString, bool)));
  connect(this->Core,
          SIGNAL(temporalFilterReady(int, const QString&, int,
                                     double, double)),
          this,
          SLOT(onTemporalFilterReady(int, const QString&, int,
                                     double, double)));

  // View menu
  connect(this->Internal->UI.sessionView, SIGNAL(SessionChanged(int)),
          this, SLOT(onTreeSelectionChanged(int)));

  connect(this->Internal->UI.sessionView, SIGNAL(SelectionChanged(int)),
          this, SLOT(onTreeSelectionChanged(int)));

  connect(this->Internal->UI.sessionView, SIGNAL(HoverItemChanged(int)),
          this, SLOT(onTreeHoverItemChanged(int)));

  connect(this->Internal->UI.sessionView, SIGNAL(ItemsChanged()),
          this, SLOT(updateCore()));

  connect(this->Internal->UI.sessionView,
          SIGNAL(CreateEvent(int, vtkIdList*, int)),
          this, SLOT(onCreateEvent(int, vtkIdList*)));

  connect(this->Internal->UI.sessionView, SIGNAL(DeleteEvent(int, int)),
          this->Core, SLOT(deleteEvent(int, int)));

  connect(this->Internal->UI.sessionView,
          SIGNAL(SessionVisibilityChanged(int, bool)),
          this->Core, SLOT(setProjectVisible(int, bool)));

  connect(this->Internal->UI.sessionView, SIGNAL(EditTrack(int, int)),
          this->Core, SLOT(beginEditingTrack(int)));
  connect(this->Internal->UI.sessionView, SIGNAL(StopEditingTrack(int)),
          this->Core, SLOT(stopEditingTrack()));
  connect(this->Internal->UI.sessionView, SIGNAL(DeleteTrack(int, int)),
          this->Core, SLOT(deleteTrack(int, int)));
  connect(this->Internal->UI.sessionView, SIGNAL(SplitTrack(int, int)),
          this, SLOT(splitTrack(int, int)));
  connect(this->Internal->UI.sessionView, SIGNAL(ImproveTrack(int, int)),
          this, SLOT(improveTrack(int, int)));

  connect(this->Internal->UI.sessionView,
          SIGNAL(AddEventsToGraphModel(QList<int>, int)),
          this, SLOT(addEventsToGraphModel(QList<int>, int)));

  connect(this->Internal->UI.sessionView,
          SIGNAL(AddTrackEventsToGraphModel(int, int)),
          this, SLOT(addTrackEventsToGraphModel(int, int)));

  connect(this->Internal->UI.sessionView,
          SIGNAL(ShowStatusMessage(const QString&, int, int)),
          this->statusBar(), SLOT(showMessage(const QString&, int)));

  connect(this->Internal->UI.sessionView,
          SIGNAL(CloseProjectRequested(int)),
          this, SLOT(closeProject(int)));

  connect(this->Internal->UI.actionShowHideTimelineView, SIGNAL(triggered(bool)),
          this, SLOT(onShowHideTimelineView(bool)));
  connect(this->TimelineDialog, SIGNAL(rejected()),
          this, SLOT(onTimelineDialogClosed()));

  connect(this->Internal->UI.actionShowHideObjectTags, SIGNAL(triggered(bool)),
          this->Core, SLOT(showHideObjectTags(bool)));

  connect(this->Internal->UI.actionDisplayEventLegend, SIGNAL(triggered(bool)),
          this, SLOT(onDisplayEventLegend(bool)));
  connect(this->Internal->UI.actionViewEventIcons, SIGNAL(toggled(bool)),
          this, SLOT(onDisplayEventIcons(bool)));

  connect(this->Internal->UI.actionViewTracks, SIGNAL(toggled(bool)),
          this->Core, SLOT(onViewTracks(bool)));
  connect(this->Internal->UI.actionViewTrackHeads, SIGNAL(toggled(bool)),
          this->Core, SLOT(onViewTrackHeads(bool)));
  connect(this->Internal->UI.actionViewEvents, SIGNAL(toggled(bool)),
          this->Core, SLOT(onViewEvents(bool)));
  connect(this->Internal->UI.actionViewActivities, SIGNAL(toggled(bool)),
          this->Core, SLOT(onViewActivities(bool)));
  connect(this->Internal->UI.actionViewSceneElements, SIGNAL(toggled(bool)),
          this->Core, SLOT(onViewSceneElements(bool)));

  connect(this->Internal->UI.actionRandomEventColors, SIGNAL(toggled(bool)),
          this->Core, SLOT(onRandomEventColor(bool)));
  connect(this->Internal->UI.actionDisplayFullVolume, SIGNAL(toggled(bool)),
          this->Core, SLOT(onDisplayFullVolume(bool)));

  connect(this->Internal->UI.actionSetTrackTrailLength, SIGNAL(triggered()),
          this, SLOT(setTrackTrailLength()));
  connect(this->Internal->UI.actionChangeTrackColors, SIGNAL(triggered()),
          this, SLOT(changeTrackColors()));
  connect(this->Internal->UI.actionPreviousAttribute, SIGNAL(triggered()),
          this->Core, SLOT(previousTrackAttributeGroup()));
  connect(this->Internal->UI.actionNextAttribute, SIGNAL(triggered()),
          this->Core, SLOT(nextTrackAttributeGroup()));

  connect(this->Internal->UI.actionViewNormalcyCues, SIGNAL(triggered(bool)),
          this->Internal->UI.actionViewSwapNormalcyScaling, SLOT(setEnabled(bool)));
  connect(this->Internal->UI.actionViewNormalcyCues, SIGNAL(triggered(bool)),
          this->Core, SLOT(onUseNormalcyCues(bool)));
  connect(this->Internal->UI.actionViewSwapNormalcyScaling, SIGNAL(triggered()),
          this->Core, SLOT(onSwapNormalcyCues()));

  connect(this->Core, SIGNAL(updateFinished()), this, SLOT(finishTrackUpdate()));

  QActionGroup* iconSizeGroup = new QActionGroup(this);
  iconSizeGroup->addAction(this->Internal->UI.actionIconSizeSmall);
  iconSizeGroup->addAction(this->Internal->UI.actionIconSizeMedium);
  iconSizeGroup->addAction(this->Internal->UI.actionIconSizeLarge);
  this->onIconSizeChange(iconSizeGroup->checkedAction(), false);

  connect(iconSizeGroup, SIGNAL(triggered(QAction*)), SLOT(onIconSizeChange(QAction*)));

  connect(this->Internal->UI.actionIncreaseIconXOffset, SIGNAL(triggered()),
          this->Core, SLOT(onIncreaseIconXOffset()));
  connect(this->Internal->UI.actionDecreaseIconXOffset, SIGNAL(triggered()),
          this->Core, SLOT(onDecreaseIconXOffset()));
  connect(this->Internal->UI.actionIncreaseIconYOffset, SIGNAL(triggered()),
          this->Core, SLOT(onIncreaseIconYOffset()));
  connect(this->Internal->UI.actionDecreaseIconYOffset, SIGNAL(triggered()),
          this->Core, SLOT(onDecreaseIconYOffset()));

  connect(this->Internal->UI.actionIncreaseOverlayTransparency, SIGNAL(triggered()),
          this->Core, SLOT(onIncreaseOverlayTransparency()));
  connect(this->Internal->UI.actionDecreaseOverlayTransparency, SIGNAL(triggered()),
          this->Core, SLOT(onDecreaseOverlayTransparency()));
  connect(this->Internal->UI.actionIncreaseSceneElementTransparency, SIGNAL(triggered()),
          this->Core, SLOT(onIncreaseSceneElementTransparency()));
  connect(this->Internal->UI.actionDecreaseSceneElementTransparency, SIGNAL(triggered()),
          this->Core, SLOT(onDecreaseSceneElementTransparency()));
  connect(this->Internal->UI.actionDisplayOverview, SIGNAL(triggered(bool)),
          this, SLOT(onDisplayOverview(bool)));
  connect(this->Core, SIGNAL(enterAdjudicationMode()),
          this, SLOT(onEnterAdjudicationMode()));
  connect(this->Internal->UI.actionAdjudicationMode, SIGNAL(triggered(bool)),
          this, SLOT(toggleAdjudicationMode(bool)));
  connect(this->Core, SIGNAL(followTrackChange(int)),
          this, SLOT(onFollowTrackChange(int)));
  connect(this->Internal->UI.actionExitFollowMode, SIGNAL(triggered()),
          this, SLOT(onExitFollowTrack()));
  connect(this->Internal->UI.actionIncreasePolygonNodeSize, SIGNAL(triggered()),
          this->Core, SLOT(onIncreasePolygonNodeSize()));
  connect(this->Internal->UI.actionDecreasePolygonNodeSize, SIGNAL(triggered()),
          this->Core, SLOT(onDecreasePolygonNodeSize()));

  // Display options
  QActionGroup* eventExpireGroup = new QActionGroup(this);
  eventExpireGroup->addAction(this->Internal->UI.actionShowEventsUntilEventEnd);
  eventExpireGroup->addAction(this->Internal->UI.actionShowEventsUntilTrackEnd);
  this->onEventExpirationModeChange(eventExpireGroup->checkedAction(), false);

  connect(eventExpireGroup,
          SIGNAL(triggered(QAction*)),
          SLOT(onEventExpirationModeChange(QAction*)));

  connect(this->Internal->UI.actionSetFrameOffset, SIGNAL(triggered()),
          this, SLOT(setFrameOffset()));

  // Playback menu
  connect(this->Internal->UI.actionFastBackward, SIGNAL(triggered(bool)),
          this, SLOT(onFastBackward()));
  connect(this->Internal->UI.actionReverse, SIGNAL(triggered(bool)),
          this, SLOT(onReversePause()));
  connect(this->Internal->UI.actionPreviousFrame, SIGNAL(triggered()),
          this->Core, SLOT(prevFrame()));
  connect(this->Internal->UI.actionPause, SIGNAL(triggered(bool)),
          this, SLOT(onPause()));
  connect(this->Internal->UI.actionNextFrame, SIGNAL(triggered()),
          this->Core, SLOT(nextFrame()));
  connect(this->Internal->UI.actionPlay, SIGNAL(triggered(bool)),
          this, SLOT(onPlayPause()));
  connect(this->Internal->UI.actionFastForward, SIGNAL(triggered(bool)),
          this, SLOT(onFastForward()));

  QShortcut* prevShortcut = new QShortcut(QKeySequence("q"),
                                          this->Internal->UI.renderFrame);
  connect(prevShortcut, SIGNAL(activated()), this->Core, SLOT(prevFrame()));
  QShortcut* nextShortcut = new QShortcut(QKeySequence("w"),
                                          this->Internal->UI.renderFrame);
  connect(nextShortcut, SIGNAL(activated()), this->Core, SLOT(nextFrame()));


  connect(this->Internal->UI.actionLoop, SIGNAL(toggled(bool)),
          this, SLOT(onLoopToggle(bool)));

  // MainWindow event.
  // @NOTE: Using Qt::QueuedConnection to delay invoking the slot. This is important
  // as if we don't do this then the viewport size on the renderer and others
  // don't get updated resulting in wrong location of legend.
  connect(this, SIGNAL(windowSizeChanged(int, int)),
          this->Core, SLOT(onResize(int, int)),
          Qt::QueuedConnection);

  this->Internal->UI.renderFrame->installEventFilter(this);

  // AOI outline.
  connect(this->Internal->UI.actionDisplayAOIOutline, SIGNAL(triggered(bool)),
          this, SLOT(onDisplayAOIOutline(bool)));

  // Handle warning / errors.
  connect(this->Core,
          SIGNAL(criticalError(QString)), SLOT(onCriticalError(QString)));
  connect(this->Core,
          SIGNAL(warningError(QString)), SLOT(onWarningError(QString)));

  // Configure dialog.
  connect(this->Internal->UI.actionConfigure, SIGNAL(triggered()),
          this, SLOT(onShowConfigureDialog()));

  // External Process dialog.
  connect(this->Internal->UI.actionExternalProcess, SIGNAL(triggered()),
          this, SLOT(onShowExternalProcessDialog()));

  // Camera
  connect(this->Internal->UI.actionViewportZoomExtents, SIGNAL(triggered()),
          this->Core, SLOT(resetView()));
  connect(this->Internal->UI.actionViewportZoomAOI, SIGNAL(triggered()),
          this->Core, SLOT(resetToAOIView()));
  connect(this->Internal->UI.actionViewportBookmark, SIGNAL(triggered()),
          this, SLOT(bookmarkViewport()));
  connect(this->Internal->UI.actionViewportRestore, SIGNAL(triggered()),
          this->Core, SLOT(restoreCameraPosition()));
  connect(this->Internal->UI.actionViewportCopyViewExtentsToClipboard, SIGNAL(triggered()),
          this, SLOT(copyViewportExtentsToClipboard()));
  connect(this->Internal->UI.actionViewportCopyExtendedInfoToClipboard, SIGNAL(triggered()),
          this, SLOT(copyExtendedInfoToClipboard()));

  //  3D View.
  connect(this->Internal->UI.action3DView, SIGNAL(triggered(bool)), this,
          SLOT(onShow3dView(bool)));
  connect(this->Viewer3dDialog, SIGNAL(rejected()), this,
          SLOT(onHide3dView()));

  // Object panel display requests.
  connect(this->Internal->UI.sessionView, SIGNAL(DisplayActivities(bool)),
          this->Internal->UI.actionViewActivities, SLOT(setChecked(bool)));
  connect(this->Internal->UI.sessionView, SIGNAL(DisplayEvents(bool)),
          this->Internal->UI.actionViewEvents, SLOT(setChecked(bool)));
  connect(this->Internal->UI.sessionView, SIGNAL(DisplayTracks(bool)),
          this->Internal->UI.actionViewTracks, SLOT(setChecked(bool)));
  connect(this->Internal->UI.sessionView, SIGNAL(DisplayTrackHeads(bool)),
          this->Internal->UI.actionViewTrackHeads, SLOT(setChecked(bool)));
  connect(this->Internal->UI.sessionView, SIGNAL(DisplaySceneElements(bool)),
          this->Internal->UI.actionViewSceneElements, SLOT(setChecked(bool)));

  // Spatial filter dock
  this->Internal->UI.spatialFilterTree->sortItems(0, Qt::AscendingOrder);
  connect(this->Internal->UI.spatialFilterTree,
          SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          SLOT(spatialFilterChanged(QTreeWidgetItem*)));
  connect(this->Internal->UI.actionSpatialFilterAdd,
          SIGNAL(triggered(bool)), SLOT(onCreateNewSpatialFilter(bool)));
  connect(this->Internal->UI.actionSpatialFilterRemoveSelected,
          SIGNAL(triggered(bool)), SLOT(onRemoveSelectedSpatialFilters()));
  connect(this->Internal->UI.actionSpatialFilterRemoveAll,
          SIGNAL(triggered(bool)), SLOT(onRemoveAllSpatialFilters()));

  // Temporal filter dock
  this->Internal->UI.temporalFilterTree->sortItems(0, Qt::AscendingOrder);

  vpFilterTypeDelegate* delegate =
    new vpFilterTypeDelegate(this->Internal->UI.temporalFilterTree);
  this->Internal->UI.temporalFilterTree->setItemDelegateForColumn(1, delegate);

  connect(this->Internal->UI.temporalFilterTree,
          SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          SLOT(temporalFilterChanged(QTreeWidgetItem*, int)));
  connect(this->Internal->UI.actionTemporalFilterAdd,
          SIGNAL(triggered(bool)), SLOT(onAddTemporalFilter()));
  connect(this->Internal->UI.actionTemporalFilterSetStart,
          SIGNAL(triggered(bool)), SLOT(onTemporalFilterSetStart()));
  connect(this->Internal->UI.actionTemporalFilterSetEnd,
          SIGNAL(triggered(bool)), SLOT(onTemporalFilterSetEnd()));
  connect(this->Internal->UI.actionTemporalFilterRemoveSelected,
          SIGNAL(triggered(bool)), SLOT(onRemoveSelectedTemporalFilters()));
  connect(this->Internal->UI.actionTemporalFilterRemoveAll,
          SIGNAL(triggered(bool)), SLOT(onRemoveAllTemporalFilters()));
  connect(this->Core, SIGNAL(removeAllFilters()),
          this, SLOT(onRemoveAllFilters()));

  // Annotation tools
  connect(this->Internal->UI.actionCreateTrack, SIGNAL(toggled(bool)),
          this, SLOT(createTrack(bool)));
  connect(this->Internal->UI.actionCreateEvent, SIGNAL(toggled(bool)),
          this, SLOT(createEvent(bool)));
  connect(this->Internal->UI.actionCreateSceneElement, SIGNAL(toggled(bool)),
          this, SLOT(createSceneElement(bool)));
  connect(this->Internal->UI.actionMergeTracks, SIGNAL(toggled(bool)),
          this, SLOT(mergeTracks(bool)));

  connect(this->Core, SIGNAL(stoppedEditingTrack()),
          this, SLOT(stopCreatingTrack()));

  // Streaming mode
  connect(this->Internal->UI.actionAutoUpdate, SIGNAL(toggled(bool)),
          this, SLOT(setAutoUpdateEnabled(bool)));

  // Misc status bar stuff
  connect(this->Core,
          SIGNAL(mouseMoved(int, int)),
          SLOT(handleRenderWindowMouseMove(int, int)));
  connect(this->Internal->UI.actionExecuteExternalProcess, SIGNAL(triggered()),
          this, SLOT(executeExternalProcess()));
  connect(this->Core, SIGNAL(exportFilters(QString, bool)),
          this, SLOT(exportFilters(QString, bool)));
  connect(this->Core, SIGNAL(frameChanged(QString)),
          SLOT(updateFrameFileName(QString)));

  // Get notifications of config changes
  connect(vpApplication::instance(),
          SIGNAL(settingsChanged(vpSettings::SettingsKeys)),
          SLOT(onSettingsChanged()));

  // Set default dock visibility.
  this->setupDock();

  // Connect context level of detail controller.
  connect(contextLODSlider, SIGNAL(valueChanged(int)),
          this, SLOT(onContextLODChanged(int)));

  // Restore window state and geometry.
  QSettings settings(QSettings::NativeFormat, QSettings::UserScope,
                     qApp->organizationName(), qApp->applicationName());
  settings.beginGroup("Window");
  this->restoreGeometry(settings.value("geometry").toByteArray());
  this->restoreState(settings.value("state").toByteArray());
  settings.endGroup();

#ifdef VISGUI_USE_KWIVER
  // Set up KWIVER embedded pipelines
  this->Internal->EmbeddedPipelineMapper = new QSignalMapper{this};
  connect(this->Internal->EmbeddedPipelineMapper, SIGNAL(mapped(QString)),
          this, SLOT(executeEmbeddedPipeline(QString)));

  QSettings config;
  for (auto i : qtIndexRange(config.beginReadArray("EmbeddedPipelines")))
    {
    config.setArrayIndex(i);
    const auto& name = config.value("Name").toString();
    const auto& path = config.value("Path").toString();
    if (!name.isEmpty() && !path.isEmpty())
      {
      this->Internal->UI.actionPipelinePlaceholder->setVisible(false);
      auto pipelineAction = new QAction{name, this};
      this->Internal->UI.menuPipeline->addAction(pipelineAction);
      this->Internal->EmbeddedPipelineMapper->setMapping(pipelineAction, path);
      connect(pipelineAction, SIGNAL(triggered()),
              this->Internal->EmbeddedPipelineMapper, SLOT(map()));
      }
    }
  config.endArray();
#endif
  this->Internal->UI.menuPipeline->setEnabled(false);
}

//-----------------------------------------------------------------------------
vpView::~vpView()
{
#ifdef ENABLE_QTTESTING
  delete this->TestUtility;
#endif
  delete this->Core;
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpView::closeEvent(QCloseEvent* e)
{
  if (this->Core->isTrackExportNeeded() ||
      this->Core->isSceneElementExportNeeded() ||
      this->Core->isEventExportNeeded())
    {
    QString questionText;
    if (this->Core->isTrackExportNeeded())
      {
      questionText =
        "Tracks have been modified but not exported. ";
      }
    else if (this->Core->isEventExportNeeded())
      {
      questionText =
        "Events have been modified but not exported. ";
      }
    if (this->Core->isSceneElementExportNeeded())
      {
      questionText =
        "Scene Elements have been modified but not exported. ";
      }
    questionText += "Are you sure you want to exit?";
    // check whether track states modified at all (or any user editing)
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, 0,
      questionText, QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
      QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes)
      {
      e->ignore();
      return;
      }
    }

  // Save window state and geometry.
  QSettings settings(QSettings::NativeFormat, QSettings::UserScope,
                     qApp->organizationName(), qApp->applicationName());
  settings.beginGroup("Window");
  settings.setValue("state", this->saveState());
  settings.setValue("geometry", this->saveGeometry());
  settings.endGroup();

  QWidget::closeEvent(e);
}

//-----------------------------------------------------------------------------
void vpView::onDataLoaded()
{
  this->resetFrameScrubberConfiguration();

  this->Internal->UI.menuCamera->setEnabled(true);
  this->Internal->UI.menuEventDisplay->setEnabled(true);
  this->Internal->UI.menuPipeline->setEnabled(true);
  this->Internal->UI.actionRandomEventColors->setEnabled(true);
  this->Internal->UI.actionChangeTrackColors->setEnabled(true);
  this->Internal->UI.actionPreviousAttribute->setEnabled(true);
  this->Internal->UI.actionNextAttribute->setEnabled(true);
  this->Internal->UI.actionDisplayFullVolume->setEnabled(true);
  this->Internal->UI.actionViewNormalcyCues->setEnabled(true);

  this->Internal->UI.actionDisplayAOIOutline->setEnabled(true);

  // By default it is on.
  // FIXME:
  this->Internal->UI.actionDisplayAOIOutline->setChecked(true);

  this->Internal->UI.actionShowHideTimelineView->setEnabled(true);
  this->Internal->UI.actionViewportBookmark->setEnabled(true);

  this->Internal->UI.actionCreateTrack->setEnabled(true);
  this->Internal->UI.actionCreateEvent->setEnabled(true);
  this->Internal->UI.actionCreateSceneElement->setEnabled(true);
  this->Internal->UI.actionMergeTracks->setEnabled(true);

  this->Internal->UI.actionExportTracks->setEnabled(true);
  this->Internal->UI.actionExportEvents->setEnabled(true);
  this->Internal->UI.actionExportSceneElements->setEnabled(true);
  this->Internal->UI.actionExportFilters->setEnabled(true);
  this->Internal->UI.actionImportFilters->setEnabled(true);

  this->Internal->UI.actionImportProject->setEnabled(true);
  this->Internal->UI.actionWebExport->setEnabled(true);
  this->Internal->UI.actionExportImageTimeStamps->setEnabled(true);

  this->Internal->UI.actionSetFrameOffset->setEnabled(
    !this->Core->getUsingTimeStampData());

  this->Internal->UI.actionViewTracks->setEnabled(true);
  this->Internal->UI.actionViewTrackHeads->setEnabled(true);
  this->Internal->UI.tocFilter->setEnabled(true);

  this->Internal->UI.actionViewEvents->setEnabled(true);
  this->Internal->UI.normalcyFilter->setEnabled(true);
  this->Internal->eventTypeFilter->setEnabled(true);

  this->Internal->UI.actionViewActivities->setEnabled(true);
  this->Internal->UI.actionViewSceneElements->setEnabled(true);

  this->Internal->UI.saliencyFilter->setEnabled(true);

  // display all activity types
  this->Internal->UI.saliencyFilter->setGroupState(0, true);

  this->Internal->UI.actionDisplayEventLegend->setEnabled(true);

  // enable viewing of various event types
  this->Internal->UI.normalcyFilter->setGroupState(0, true);
  // and finally, enable the renderFrame and slider / current frame controls
  this->Internal->UI.renderFrame->setEnabled(true);
  this->Internal->UI.currentFrame->setEnabled(true);
  this->Internal->UI.minFrameCheckBox->setEnabled(true);

  this->Internal->UI.actionSpatialFilterAdd->setEnabled(true);

  this->postDataLoaded();

  // Initialize the normalcy map widget.
  this->Core->setupNormalcyMapsFilters(this->Internal->UI.normalcyMapsFilter);

  this->Core->UpdateEventDisplayStates(true);

  // Now is a good time to setup 3d view widget.
  this->Core->setup3dWidget(this->Viewer3dDialog->getViewer3dWidget());

  // Update UI components.
  this->updateUI();

  this->updateObjectCounts();

  // Perform the initial streaming update
  if (this->Internal->CliArgs->isSet("streaming") &&
      this->Internal->UI.actionAutoUpdate->isChecked())
    {
    this->updateTracks();
    this->Internal->TrackUpdateTimer->start();
    }

  this->Core->updateScene();
}

//-----------------------------------------------------------------------------
void vpView::onDataChanged()
{
  this->updateFrameScrubberRange();
}

//-----------------------------------------------------------------------------
void vpView::onIconsLoaded()
{
  // Now we can enable the option to display event icons.
  this->Internal->UI.actionViewEventIcons->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vpView::onOverviewLoaded()
{
  this->Internal->UI.actionDisplayOverview->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vpView::onEnterAdjudicationMode()
{
  this->Internal->UI.actionAdjudicationMode->setEnabled(true);
  this->Internal->UI.actionAdjudicationMode->setChecked(true);
}

//-----------------------------------------------------------------------------
void vpView::onFollowTrackChange(int trackId)
{
  this->Internal->UI.actionExitFollowMode->setEnabled(trackId != -1);
}

//-----------------------------------------------------------------------------
void vpView::onExitFollowTrack()
{
  this->Core->setIdOfTrackToFollow(-1);
}

//-----------------------------------------------------------------------------
void vpView::toggleAdjudicationMode(bool vtkNotUsed(state))
{
  this->Internal->UI.actionAdjudicationMode->setEnabled(false);
  this->Core->exitAdjudicationMode();
}

//-----------------------------------------------------------------------------
void vpView::onTreeHoverItemChanged(int sessionId)
{
  vgItemInfo info;
  if (!this->Internal->UI.sessionView->GetHoveredItemInfo(info))
    {
    this->Core->hideAnnotation();
    return;
    }

  this->Core->showAnnotation(sessionId, info.Type, info.Id);
}

//-----------------------------------------------------------------------------
void vpView::updateObject(int objectType, int id)
{
  this->updateEverything();
  this->Internal->UI.sessionView->SelectItem(objectType, id);
}

//-----------------------------------------------------------------------------
void vpView::updateColorofTracksOfType(int typeIndex, double* rgb)
{
  this->Core->updateColorofTracksOfType(typeIndex, rgb);
  this->Core->updateScene();
}

//-----------------------------------------------------------------------------
void vpView::updateCore()
{
  this->Core->update();
}

//-----------------------------------------------------------------------------
void vpView::updateEverything()
{
  this->rebuildObjectViews();
  this->Core->updateScene();
}

//-----------------------------------------------------------------------------
void vpView::onTreeSelectionChanged(int sessionId)
{
  SetScoped ss(this->Internal->Selecting);

  this->Core->onShowObjectInfo(sessionId, this->Internal->UI.objectInfoWidget);
  this->updateInfoWidget();

  this->Core->onShowTrackAttributes(sessionId,
    this->Internal->UI.trackAttributesWidget);

  vgItemInfo info;
  if (!this->Internal->UI.sessionView->GetSelectedItemInfo(info) ||
      info.Type != vgObjectTypeDefinitions::Event)
    {
    // Clear selection
    this->GraphModelWidget->selectEvent(-1);
    }
  else
    {
    this->GraphModelWidget->selectEvent(info.Id);
    }

  this->Core->setTrackSelection(
    this->Internal->UI.sessionView->GetSelectedItems(
      vgObjectTypeDefinitions::Track), sessionId);

  this->Core->setEventSelection(
    this->Internal->UI.sessionView->GetSelectedItems(
      vgObjectTypeDefinitions::Event), sessionId);

  if (info.Type == vgObjectTypeDefinitions::SceneElement)
    {
    this->Core->setTrackSelection(
      this->Internal->UI.sessionView->GetSelectedItems(
        vgObjectTypeDefinitions::SceneElement), sessionId);
    }
}

//-----------------------------------------------------------------------------
void vpView::updateInfoWidget(bool trackAttributesOnly/*=false*/)
{
  vgItemInfo info;
  if (!this->Internal->UI.sessionView->GetSelectedItemInfo(info))
    {
    this->Internal->UI.trackAttributesWidget->
      ShowTrackAttributesControls(false);
    this->Internal->UI.objectInfoWidget->ShowEmptyPage();
    return;
    }
  if (trackAttributesOnly && info.Type != vgObjectTypeDefinitions::Track)
    {
    return;
    }

  switch (info.Type)
    {
    case vgObjectTypeDefinitions::Activity:
      this->Internal->UI.objectInfoWidget->ShowActivityInfo(info.Id);
      break;
    case vgObjectTypeDefinitions::Event:
      this->Internal->UI.objectInfoWidget->ShowEventInfo(
        info.Id,
        info.ParentId,
        info.Index);
      // select the event in the timeline
      if (this->TimelineDialog->isVisible())
        {
        emit this->eventSelected(info.Id);
        }
      break;
    case vgObjectTypeDefinitions::Track:
      this->Internal->UI.trackAttributesWidget->
        UpdateTrackAttributes(info.Id);
      if (!trackAttributesOnly)
        {
        this->Internal->UI.objectInfoWidget->ShowTrackInfo(
          info.Id,
          info.ParentId,
          info.Index);
        }
      break;
    case vgObjectTypeDefinitions::SceneElement:
      this->Internal->UI.objectInfoWidget->ShowSceneElementInfo(
        info.Id);
      break;
    }
}

//-----------------------------------------------------------------------------
void vpView::rebuildObjectViews()
{
  this->Internal->UI.sessionView->Update(true);
  this->updateInfoWidget();
}

//-----------------------------------------------------------------------------
void vpView::onCreateEvent(int type, vtkIdList* ids)
{
  int session = this->Internal->UI.sessionView->GetCurrentSession();
  int id = this->Core->createEvent(type, ids, session);

  // show the newly created event
  if (id >= 0)
    {
    this->Core->getEventModel(session)->GetEvent(id)->ClearFlags(
      vtkVgEvent::EF_Modifiable);

    this->Internal->UI.sessionView->AddAndSelectItem(
      vgObjectTypeDefinitions::Event, id);

    // focus it in the viewport
    this->Internal->UI.sessionView->FocusItem();
    }
}

//-----------------------------------------------------------------------------
void vpView::onTimelineSelectionChanged(int type, int id)
{
  this->Internal->UI.sessionView->SelectItem(type, id);
}

//-----------------------------------------------------------------------------
void vpView::onShowHideTimelineView(bool show)
{
  this->Core->showHideTimelineDialog(this->TimelineDialog, show);
}

//-----------------------------------------------------------------------------
void vpView::onTimelineDialogClosed()
{
  this->Internal->UI.actionShowHideTimelineView->setChecked(false);
}

//-----------------------------------------------------------------------------
void vpView::onDisplayEventLegend(bool state)
{
  this->Internal->UI.actionDisplayEventLegend->setChecked(state);
  this->Core->showHideEventLegend(state, true);
}

//-----------------------------------------------------------------------------
void vpView::onDisplayEventIcons(bool state)
{
  this->Internal->UI.actionViewEventIcons->setChecked(state);
  this->Core->onViewEventIcons(state);
}

//-----------------------------------------------------------------------------
void vpView::onFrameChange()
{
  // If the frame changed as a result of scrubbing there is nothing to do
  if (this->Internal->Seeking)
    {
    return;
    }

  if (this->Core->getUsingTimeBasedIndexing())
    {
    // Wait for the more accurate timeChanged() signal
    return;
    }

  // Frame numbers only
  int frameNumber = this->Core->getCurrentFrameNumber();

  // Make sure the max is high enough let us change the value.
  if (this->Internal->UI.minFrame->maximum() < frameNumber)
    {
    this->Internal->UI.minFrame->setMaximum(frameNumber);
    }

  // Maintain the existing time interval if possible.
  if (this->Internal->UI.minFrameCheckBox->checkState() == Qt::Checked)
    {
    int minFrame =
      std::max<int>(this->Core->getMinimumFrameNumber(),
                    frameNumber -
                    this->Internal->MinFrameOffset.GetFrameNumber());

      {
      qtScopedBlockSignals bs(this->Internal->UI.minFrame);
      this->Internal->UI.minFrame->setValue(minFrame);
      }
      {
      qtScopedBlockSignals bs(this->Internal->UI.currentFrame);
      this->Internal->UI.currentFrame->setFrameNumbers(minFrame, frameNumber);
      }
    }
  else
    {
      {
      qtScopedBlockSignals bs(this->Internal->UI.minFrame);
      this->Internal->UI.minFrame->setValue(frameNumber);
      }
      {
      qtScopedBlockSignals bs(this->Internal->UI.currentFrame);
      this->Internal->UI.currentFrame->setFrameNumber(frameNumber);
      }
    }

  // Make sure low end of the interval goes no higher than the current frame.
  this->Internal->UI.minFrame->setMaximum(frameNumber);
}

//-----------------------------------------------------------------------------
void vpView::onTimeChange(double now)
{
  if (!this->Core->getUsingTimeBasedIndexing())
    {
    return;
    }

  // Update the time controls to match the new core time, which is most likely
  // not the same as the current frame's timestamp.
  if (this->Internal->UI.minFrameCheckBox->checkState() == Qt::Checked)
    {
    double minTime =
      std::max(this->Core->getMinimumTime(),
               now - this->Internal->MinFrameOffset.GetTime());

    qtScopedBlockSignals bs(this->Internal->UI.currentFrame);
    this->Internal->UI.currentFrame->setTimes(minTime, now);
    this->updateMinTime(minTime);
    }
  else
    {
    qtScopedBlockSignals bs(this->Internal->UI.currentFrame);
    this->Internal->UI.currentFrame->setTime(now);
    this->updateMinTime(now);
    }
}

//-----------------------------------------------------------------------------
void vpView::onReachedPlayBoundary()
{
  if (!this->Internal->CliArgs->isSet("streaming"))
    {
    this->Internal->UI.actionPause->setChecked(true);
    this->Core->onPause();
    }
}

//-----------------------------------------------------------------------------
void vpView::onPlayPause()
{
  if (this->Core->isPlaying() && this->Core->getPlaybackRate() == 1.0)
    {
    this->Internal->UI.actionPause->setChecked(true);
    this->Core->onPause();
    }
  else
    {
    this->Core->setPlaybackRate(1.0);
    this->Core->onPlay();
    }
}

//-----------------------------------------------------------------------------
void vpView::onPause()
{
  if (this->Core->isPlaying())
    {
    this->Internal->UI.actionPause->setChecked(true);
    this->Core->onPause();
    }
  else
    {
    // Resume in the same direction we were previously playing
    if (this->Core->getPlaybackRate() < 0.0)
      {
      this->Internal->UI.actionReverse->setChecked(true);
      this->Core->setPlaybackRate(-1.0);
      }
    else
      {
      this->Internal->UI.actionPlay->setChecked(true);
      this->Core->setPlaybackRate(1.0);
      }
    this->Core->onPlay();
    }
}

//-----------------------------------------------------------------------------
void vpView::onReversePause()
{
  if (this->Core->isPlaying() && this->Core->getPlaybackRate() == -1.0)
    {
    this->Internal->UI.actionPause->setChecked(true);
    this->Core->onPause();
    }
  else
    {
    this->Core->setPlaybackRate(-1.0);
    this->Core->onPlay();
    }
}

//-----------------------------------------------------------------------------
void vpView::onFastForward()
{
  double rate = this->Core->getPlaybackRate();
  if (rate > 1.0)
    {
    rate = qMin(rate * 2.0, 32.0);
    this->Core->setPlaybackRate(rate);
    }
  else
    {
    this->Core->setPlaybackRate(2.0);
    }
  this->Core->onPlay();
}

//-----------------------------------------------------------------------------
void vpView::onFastBackward()
{
  double rate = this->Core->getPlaybackRate();
  if (rate < -1.0)
    {
    rate = qMax(rate * 2.0, -32.0);
    this->Core->setPlaybackRate(rate);
    }
  else
    {
    this->Core->setPlaybackRate(-2.0);
    }
  this->Core->onPlay();
}

//-----------------------------------------------------------------------------
void vpView::onLoopToggle(bool state)
{
  this->Internal->UI.actionLoop->setChecked(state);
  this->Core->setLoop(state);
}

//-----------------------------------------------------------------------------
void vpView::onFrameUpdate(int minFrameNumber, int frameNumber, bool resized)
{
  int frameNumberOffset = this->Core->getUseZeroBasedFrameNumbers() ? 0 : 1;
  if (frameNumber >= frameNumberOffset)
    {
    if (this->Core->getObjectExpirationTime().IsValid())
      {
      // Update the offset if this represents an intentional change to the
      // interval size (such as when dragging a single handle of the frame
      // slider).
      vtkVgTimeStamp offsetTimeStamp;
      int frameDelta = frameNumber - minFrameNumber;
      offsetTimeStamp.SetFrameNumber(frameDelta);
      offsetTimeStamp.SetTime(1e6 * (frameDelta / this->Core->getRequestFPS()));
      if (resized)
        {
        this->Internal->MinFrameOffset = offsetTimeStamp;
        }
      this->Core->setObjectExpirationTime(offsetTimeStamp);
      }

    vtkVgTimeStamp time;
    time.SetFrameNumber(frameNumber - frameNumberOffset);
    SetScoped ss(this->Internal->Seeking);
    this->Core->setCurrentTime(time);
    this->Core->updateScene();
    }
  else
    {
    qErrnoWarning("Frame number cannot be less that one. Possibly data import failed.");
    }
}

//-----------------------------------------------------------------------------
void vpView::onTimeUpdate(double minTime, double time, bool resized)
{
  if (this->Core->getObjectExpirationTime().IsValid())
    {
    vtkVgTimeStamp offsetTimeStamp;
    offsetTimeStamp.SetTime(time - minTime);
    offsetTimeStamp.SetFrameNumber(qRound((time - minTime) *
                                          this->Core->getRequestFPS() / 1e6));
    if (resized)
      {
      this->Internal->MinFrameOffset = offsetTimeStamp;
      }
    this->Core->setObjectExpirationTime(offsetTimeStamp);
    }

  SetScoped ss(this->Internal->Seeking);
  this->Core->setCurrentTime(vtkVgTimeStamp(time));
  this->Core->updateScene();
}

//-----------------------------------------------------------------------------
void vpView::updateMinTime(double time)
{
  this->Internal->UI.minTime->setText(vgUnixTime(time).timeString());
}

//-----------------------------------------------------------------------------
void vpView::onFrameIntervalEnabled(int state)
{
  if (this->Core->getUsingTimeBasedIndexing())
    {
    double currTime = this->Core->getCurrentFrameTime();
    if (state == Qt::Checked)
      {
      double val =
        std::max(this->Core->getMinimumTime(),
                 currTime - this->Internal->MinFrameOffset.GetTime());
      this->Internal->UI.minTime->setEnabled(true);
      this->updateMinTime(val);
      this->Internal->UI.currentFrame->setIntervalModeEnabled(true);
      this->Internal->UI.currentFrame->setMinTime(val);
      vtkVgTimeStamp offset;
      offset.SetTime(currTime - val);
      this->Core->setObjectExpirationTime(offset);
      }
    else
      {
      this->Internal->UI.minTime->setEnabled(false);
      this->updateMinTime(currTime);
      this->Internal->UI.currentFrame->setIntervalModeEnabled(false);
      this->Core->setObjectExpirationTime(vtkVgTimeStamp());
      }
    }
  else
    {
    int currFrame = this->Core->getCurrentFrameNumber();
    if (state == Qt::Checked)
      {
      int val =
        std::max<int>(this->Core->getMinimumFrameNumber(),
                      currFrame -
                      this->Internal->MinFrameOffset.GetFrameNumber());
      this->Internal->UI.minFrame->setEnabled(true);
      this->Internal->UI.minFrame->setValue(val);
      this->Internal->UI.currentFrame->setIntervalModeEnabled(true);
      this->Internal->UI.currentFrame->setMinFrameNumber(val);
      vtkVgTimeStamp offset;
      offset.SetFrameNumber(currFrame - val);
      offset.SetTime(1e6 * ((currFrame - val) / this->Core->getRequestFPS()));
      this->Core->setObjectExpirationTime(offset);
      }
    else
      {
      this->Internal->UI.minFrame->setEnabled(false);
      this->Internal->UI.minFrame->setValue(currFrame);
      this->Internal->UI.currentFrame->setIntervalModeEnabled(false);
      this->Core->setObjectExpirationTime(vtkVgTimeStamp());
      }
    }

  this->Core->updateScene();
}

//-----------------------------------------------------------------------------
void vpView::onIconSizeChange(QAction* sizeAction, bool render)
{
  int size;
  if (sizeAction == this->Internal->UI.actionIconSizeSmall)
    {
    size = 16;
    }
  else if (sizeAction == this->Internal->UI.actionIconSizeLarge)
    {
    size = 64;
    }
  else
    {
    size = 32;
    }

  this->Core->setIconSize(size, render);
}

//-----------------------------------------------------------------------------
void vpView::onDisplayOverview(bool state)
{
  this->Internal->UI.actionDisplayOverview->setChecked(state);
  this->Core->setOverviewDisplayState(state);
}

//-----------------------------------------------------------------------------
void vpView::onDisplayAOIOutline(bool state)
{
  this->Internal->UI.actionDisplayAOIOutline->setChecked(state);
  this->Core->setDisplayAOIOutlineState(state);
}

//-----------------------------------------------------------------------------
void vpView::onCriticalError(const QString& errorMsg)
{
  switch (QMessageBox::critical(0, "vpView", errorMsg))
    {
    case QMessageBox::Ok:
      {
      break;
      }
    default:
      {
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vpView::onWarningError(const QString& warningMsg)
{
  switch (QMessageBox::warning(0, "vpView", warningMsg))
    {
    case QMessageBox::Ok:
      {
      break;
      }
    default:
      {
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vpView::onShowConfigureDialog()
{
  vpConfigureDialog dialog(this->Core, this);
  dialog.setTrackAttributes(this->Core->getTrackAttributes());
  dialog.setColorWindow(this->Core->colorWindow());
  dialog.setColorLevel(this->Core->colorLevel());

  if (dialog.exec() == QDialog::Accepted)
    {
    // Do nothing.
    }
}

//-----------------------------------------------------------------------------
void vpView::onShowExternalProcessDialog()
{
  if (this->Core->isExternalProcessRunning())
    {
    QMessageBox::warning(this, "Warning!",
                         "Can only execute one process at a time!");
    return;
    }

  vpExternalProcessDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted)
    {
    this->Core->startExternalProcess(dialog.getProgram(),
                                     dialog.getArguments(),
                                     dialog.getIOPath());
    }
}


//-----------------------------------------------------------------------------
void vpView::executeExternalProcess()
{
  if (this->Core->isExternalProcessRunning())
    {
    QMessageBox::warning(this, "Warning!",
                         "Can only execute one process at a time!");
    return;
    }

  vpSettings settings;
  this->Core->startExternalProcess(settings.externalProcessProgram(),
                                   settings.externalProcessArgs(),
                                   settings.externalProcessIOPath());
}

//-----------------------------------------------------------------------------
void vpView::executeEmbeddedPipeline(const QString& pipelinePath)
{
  const auto session = this->Internal->UI.sessionView->GetCurrentSession();
  this->Core->executeEmbeddedPipeline(session, pipelinePath);
}

//-----------------------------------------------------------------------------
bool vpView::eventFilter(QObject* obj, QEvent* event)
{
  switch (event->type())
    {
    default:
      break;
    case QEvent::Resize:
      {
      emit this->windowSizeChanged(this->Internal->UI.renderFrame->width(),
                                   this->Internal->UI.renderFrame->height());
      break;
      }
    case QEvent::Leave:
      {
      this->updateGsdDisplay();
      this->updateCoordinateDisplay();
      this->Core->mouseLeftViewport();
      break;
      }
    case QEvent::Enter:
      {
      this->Core->mouseEnteredViewport();
      break;
      }
    }

  return QObject::eventFilter(obj, event);
}

//-----------------------------------------------------------------------------
void vpView::initialize(const qtCliArgs* cliArgs)
{
  this->Internal->CliArgs = cliArgs;
  // Defer command line parsing until the event loop has been entered.
  QMetaObject::invokeMethod(this, "onInitialize", Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
void vpView::onInitialize()
{
  if (this->Internal->CliArgs)
    {
#ifdef ENABLE_QTTESTING
    this->TestUtility->ParseCommandLine(*this->Internal->CliArgs);
#endif
    this->parseCommandLine(*this->Internal->CliArgs);
    }

  this->onLoopToggle(this->Internal->UI.actionLoop->isChecked());

  bool streaming = this->Internal->CliArgs->isSet("streaming");
  this->Core->setStreamingEnabled(streaming);

  this->Internal->UI.actionAutoUpdate->setEnabled(streaming);

  this->processCommandLine();

  this->Core->setupEventConfig(this->Internal->UI.eventConfig);
  this->Core->setupActivityConfig(this->Internal->UI.activityConfig);

  // Update graph model
  this->GraphModelWidget->initializeUi();
  this->GraphModelWidget->loadEventTypes(this->Core->getEventTypeRegistry());
  this->GraphModelWidget->loadPrimitiveTypes();
  this->GraphModelWidget->loadAttributeTypes();
  this->GraphModelWidget->setSpatialOverlayRenderer(
    this->Core->getSceneRenderer(), this->Core->getInteractorStyle());

#ifdef ENABLE_QTTESTING
  this->TestUtility->ProcessCommandLine();
#endif
}

//-----------------------------------------------------------------------------
void vpView::parseCommandLine(const qtCliArgs& args)
{
  this->Internal->ProjectFileNames = args.values("project");
  this->Internal->ConfigFileName = args.value("config");
}

//-----------------------------------------------------------------------------
void vpView::processCommandLine()
{
  // Reset all settings?
  if (this->Internal->CliArgs->isSet("reset-config"))
    {
    QSettings().clear();  // clear ini settings
    vpSettings().clear(); // clear any other settings
    this->loadSettings();
    }

#ifdef _WIN32
  vtkNew<vtkWin32ProcessOutputWindow> processOutputWindow;
  vtkOutputWindow::SetInstance(processOutputWindow.GetPointer());
#endif

  // Load the config file. Order is important.
  if (!this->Internal->ConfigFileName.isEmpty())
    {
    this->Core->loadConfig(qPrintable(this->Internal->ConfigFileName));
    this->GraphModelWidget->loadConfig(this->Internal->ConfigFileName);
    }

  this->postLoadConfig();

  // Now load the projects.
  bool loadedProject = false;
  foreach (const QString& p, this->Internal->ProjectFileNames)
    {
    if (this->Core->loadProject(qPrintable(p)))
      {
      loadedProject = true;
      }
    }

  if (loadedProject)
    {
    // Update observers.
    this->Core->dataChanged();
    }
}

//-----------------------------------------------------------------------------
void vpView::setupDock()
{
  // set default visibility
  //
  // NOTE: wait until actions are set up, as the action check state is not
  //       correct until the parent window has been shown; if we did this
  //       first, it wouldn't work, and all docks would be hidden by the dock
  //       controller's initial state synchronization
  this->Internal->UI.sessionViewDock->show();
  this->Internal->UI.objectInfoDock->show();
  this->Internal->UI.trackAttributesDock->hide();
  this->Internal->UI.eventOptionDock->hide();
  this->Internal->UI.displayOptionsDock->hide();
  this->Internal->UI.normalcyMapsDock->hide();
  this->Internal->UI.spatialFiltersDock->hide();
  this->Internal->UI.graphModelDock->show();
}

//-----------------------------------------------------------------------------
void vpView::postLoadConfig()
{
  this->Core->setupTrackAttributeColors();
  this->Core->setupTrackFilters(this->Internal->UI.tocFilter);

  connect(this->Core, SIGNAL(trackTypesModified()),
          this, SLOT(updateTrackFilters()));
}

//-----------------------------------------------------------------------------
void vpView::postDataLoaded()
{
  // \note: If state is not changed by setChecked qt does not invoke
  // toggled(...). Hence we have to first set it to unchecked state and
  // then back to the state we want it to be.
  bool state = this->Internal->UI.actionViewTracks->isEnabled() &&
               this->Internal->UI.actionViewTracks->isChecked();
  this->Internal->UI.actionViewTracks->setChecked(false);
  this->Internal->UI.actionViewTracks->setChecked(state);

  state = this->Internal->UI.actionViewTrackHeads->isEnabled() &&
          this->Internal->UI.actionViewTrackHeads->isChecked();
  this->Internal->UI.actionViewTrackHeads->setChecked(false);
  this->Internal->UI.actionViewTrackHeads->setChecked(state);

  state = this->Internal->UI.actionViewEvents->isEnabled() &&
          this->Internal->UI.actionViewEvents->isChecked();
  this->Internal->UI.actionViewEvents->setChecked(false);
  this->Internal->UI.actionViewEvents->setChecked(state);

  state = this->Internal->UI.actionViewEventIcons->isEnabled() &&
          this->Internal->UI.actionViewEventIcons->isChecked();
  this->Internal->UI.actionViewEventIcons->setChecked(false);
  this->Internal->UI.actionViewEventIcons->setChecked(state);

  state = this->Internal->UI.actionDisplayEventLegend->isChecked();
  this->Core->showHideEventLegend(state, false);

  state = this->Internal->UI.actionViewEventIcons->isEnabled() &&
          this->Internal->UI.actionViewEventIcons->isChecked();
  this->Internal->UI.actionViewEventIcons->setChecked(false);
  this->Internal->UI.actionViewEventIcons->setChecked(state);

  state = this->Internal->UI.actionViewActivities->isEnabled() &&
          this->Internal->UI.actionViewActivities->isChecked();
  this->Internal->UI.actionViewActivities->setChecked(false);
  this->Internal->UI.actionViewActivities->setChecked(state);

  state = this->Internal->UI.actionDisplayFullVolume->isChecked();
  this->Core->onDisplayFullVolume(state, false);

  // Initialize the event and activity widgets now that we now the types used.
  this->Core->setupEventFilters(this->Internal->UI.normalcyFilter,
                                this->Internal->eventTypeFilter);
  this->Core->setupActivityFilters(this->Internal->UI.saliencyFilter);

  this->Internal->UI.eventConfig->Update();
  this->Internal->UI.activityConfig->Update();

  this->Internal->DataLoaded = true;
}

//-----------------------------------------------------------------------------
void vpView::exitApp()
{
  this->close();
}

//-----------------------------------------------------------------------------
void vpView::updateTrackFilters()
{
  auto oldTypes = this->Internal->UI.tocFilter->keys().toSet();
  auto* const ttr = this->Core->getTrackTypeRegistry();

  const int k = ttr->GetNumberOfTypes();
  for (int i = 0; i < k; ++i)
    {
    const auto& tt = ttr->GetEntityType(i);
    const auto& typeName = QString::fromLocal8Bit(tt.GetName());

    if (typeName.isEmpty())
      {
      // Handle "deleted" types
      if (oldTypes.contains(i))
        {
        this->Internal->UI.tocFilter->removeItem(i);
        }
      }
    else
      {
      if (oldTypes.contains(i))
        {
        this->Internal->UI.tocFilter->setText(i, typeName);
        }
      else
        {
        this->Core->addTrackFilter(this->Internal->UI.tocFilter, i, typeName);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vpView::onSaveRenderedImages(bool state)
{
  // if turning "on", ask for an output directory (required)
  if (state == true)
    {
    QString path = vgFileDialog::getExistingDirectory(
                     0, "Image Output Directory");

    if (!path.isEmpty())
      {
      this->Core->setSaveRenderedImages(state, &path);
      }
    else
      {
      this->Internal->UI.actionSaveRenderedImages->setChecked(false);
      }
    }
  else
    {
    this->Core->setSaveRenderedImages(false);
    }
}

//-----------------------------------------------------------------------------
void vpView::updateFrameTime()
{
  vgUnixTime time(this->Core->getCurrentFrameTime());
  QString ts = time.timeString();
  if (this->Core->getUsingTimeBasedIndexing())
    {
    ts += QString(" [%1]").arg(this->Core->getCurrentFrameIndex());
    }
  if (this->Core->getUsingTimeStampData())
    {
    this->Internal->FrameDate->setText(time.dateString());
    this->Internal->FrameDate->show();
    }
  this->Internal->FrameTime->setText(ts);
  this->Internal->FrameTime->show();
}

//-----------------------------------------------------------------------------
void vpView::updateObjectCounts()
{
  QString as = QString("%1 activities").arg(this->Core->getNumberOfActivities());
  QString es = QString("%1 events").arg(this->Core->getNumberOfEvents());
  QString ts = QString("%1 tracks").arg(this->Core->getNumberOfTracks());
  this->Internal->ActivityCount->setText(as);
  this->Internal->EventCount->setText(es);
  this->Internal->TrackCount->setText(ts);
}

//-----------------------------------------------------------------------------
void vpView::handleRenderWindowMouseMove(int x, int y)
{
  this->Internal->RenderWindowMouseCoords[0] = x;
  this->Internal->RenderWindowMouseCoords[1] = y;

  this->updateCoordinateDisplay();
  this->updateGsdDisplay();
}

//-----------------------------------------------------------------------------
void vpView::updateCoordinateDisplay()
{
  if (!this->Internal->UI.renderFrame->underMouse())
    {
    this->Internal->Coordinates->setText(QString());
    return;
    }

  int out[2];
  bool hasImgCoords = false;
  switch (this->Internal->CoordDisplayMode)
    {
      // Display coordinates only within the AOI region.
    case vpSettings::AOIRelativeCoords:
      {
      hasImgCoords =
        this->Core->displayToAOI(this->Internal->RenderWindowMouseCoords, out);
      break;
      }

    // Display coordinates within the bounds of the image.
    case vpSettings::ImageRelativeCoords:
      {
      hasImgCoords =
        this->Core->displayToImage(this->Internal->RenderWindowMouseCoords, out);
      break;
      }
    }

  QString str;
  if (hasImgCoords)
    {
    str = QString("%1, %2 ").arg(out[0]).arg(out[1]);
    }

  double northing, easting;
  if (this->Core->displayToGeo(this->Internal->RenderWindowMouseCoords,
                               northing, easting))
    {
    str += QString().sprintf("(%+.6f, %+.6f)", northing, easting);
    }

  this->Internal->Coordinates->setText(str);
}

//-----------------------------------------------------------------------------
void vpView::updateGsdDisplay()
{
  double latDist, lonDist;
  double width, height;

  if (this->Internal->UI.renderFrame->underMouse())
    {
    // The mouse is positioned over the render window, so get the width and
    // height of the hovered image pixel.
    if (this->Core->getGsd(this->Internal->RenderWindowMouseCoords,
                           latDist, lonDist, width, height))
      {
      this->showGsd(width, height);
      }
    }
  else
    {
    // The mouse is outside the render window, so just show the average GSD
    // across the whole image.
    if (this->Core->getGsd(latDist, lonDist, width, height))
      {
      this->showGsd(width, height, "/px");
      }
    }
}

//-----------------------------------------------------------------------------
void vpView::showGsd(double width, double height, const QString& suffix)
{
  this->Internal->Gsd->setText(
    ((width < 0.5 && height < 0.5) ?
     QString().sprintf("%.3f x %.3f cm", width * 100.0, height * 100.0) :
     QString().sprintf("%.3f x %.3f m", width, height)) + suffix);

  this->Internal->Gsd->show();
}

//-----------------------------------------------------------------------------
void vpView::updateFrameFileName(const QString& fileName)
{
  QFileInfo fileInfo(fileName);
  this->Internal->FrameFileName->setText(fileInfo.fileName());
}

//-----------------------------------------------------------------------------
void vpView::updateUI()
{
  if (this->Core->hasMultiLevelOfDetailSource())
    {
    this->Internal->contextLOD->setEnabled(true);
    }
  else
    {
    this->Internal->contextLOD->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void vpView::onExecuteModeChanged(int index)
{
  QComboBox* combo = this->Internal->executeModeCombo;
  this->Core->setExternalExecuteMode(combo->itemData(index).toInt());
}

//-----------------------------------------------------------------------------
void vpView::onEventFilterPresetChanged(int index)
{
  QComboBox* combo = this->Internal->eventTypeFilter;
  int ftype = combo->itemData(index).toInt();
  if (ftype == -2)
    {
    // selected 'custom', don't change filters
    return;
    }

  // remove 'custom' if it exists
  int indexCustom = combo->findData(QVariant(-2));
  if (indexCustom >= 0)
    {
    combo->removeItem(indexCustom);
    }

  // don't change preset while we update filters... can't use blockSignals
  // because we want filter changes to otherwise be seen.
  this->Internal->DisableFilterPresetUpdates = true;

  vgMixerWidget* filter = this->Internal->UI.normalcyFilter;
  foreach (int t, filter->keys())
    {
    // turn on filter if it matches preset or preset is 'all', else turn off
    filter->setState(t, ftype == -1 || ftype == t);
    }

  this->onEventFilterChanged();

  this->Internal->DisableFilterPresetUpdates = false;
}

//-----------------------------------------------------------------------------
void vpView::onEventFilterVisibilityChanged()
{
  if (this->Internal->DisableFilterPresetUpdates)
    {
    return;
    }

  vgMixerWidget* filter = this->Internal->UI.normalcyFilter;
  QList<int> keys = filter->keys();
  int countEnabled = 0;
  int maybeType = -2;

  // count events turned on and record last type
  foreach (int t, keys)
    {
    if (filter->state(t))
      {
      ++countEnabled;
      maybeType = t;
      }
    }

  // how many events turned on?
  if (countEnabled == keys.count())
    {
    maybeType = -1;
    }
  else if (countEnabled != 1)
    {
    maybeType = -2;
    }

  QComboBox* combo = this->Internal->eventTypeFilter;
  combo->blockSignals(true);
  int indexCustom = combo->findData(QVariant(-2));
  if (maybeType == -2)
    {
    // ensure 'custom' exists
    if (indexCustom < 0)
      {
      combo->addItem("(Custom)", QVariant(-2));
      }
    }
  else if (indexCustom >= 0)
    {
    // remove 'custom'
    combo->removeItem(indexCustom);
    }

  // set to appropriate index
  combo->setCurrentIndex(combo->findData(maybeType));
  combo->blockSignals(false);
}

//-----------------------------------------------------------------------------
void vpView::onFrameRendered()
{
  if (this->Viewer3dDialog->isVisible())
    {
    this->Viewer3dDialog->update(this->Core->getImageryTimeStamp());
    }
  this->updateGsdDisplay();
  this->updateFrameTime();
}

//-----------------------------------------------------------------------------
void vpView::onEventFilterChanged()
{
  this->Core->update();
}

//-----------------------------------------------------------------------------
void vpView::onReinitialized()
{
  this->Internal->UI.actionViewTracks->setEnabled(false);
  this->Internal->UI.actionViewTrackHeads->setEnabled(false);
  this->Internal->UI.actionViewEvents->setEnabled(false);
  this->Internal->UI.actionViewEventIcons->setEnabled(false);
  this->Internal->UI.actionViewActivities->setEnabled(false);
}

//-----------------------------------------------------------------------------
void vpView::onShow3dView(bool state)
{
  if (state)
    {
    this->Core->setGraphRenderingEnabled(true);
    this->Viewer3dDialog->update(this->Core->getImageryTimeStamp());
    this->Viewer3dDialog->show();
    this->Viewer3dDialog->reset();
    }
  else
    {
    this->Viewer3dDialog->close();
    }
}

//-----------------------------------------------------------------------------
void vpView::onHide3dView()
{
  this->Core->setGraphRenderingEnabled(false);
  this->Internal->UI.action3DView->setChecked(false);
}

//-----------------------------------------------------------------------------
void vpView::onContextLODChanged(int value)
{
  if (value > 0)
    {
    this->Core->setImageSourceLevelOfDetailFactor(1.0 / value);
    }
  else if (value < 0)
    {
    this->Core->setImageSourceLevelOfDetailFactor(1.0 * abs(value));
    }
  else
    {
    this->Core->setImageSourceLevelOfDetailFactor(1.0);
    }

  this->Core->render(false);
}

//-----------------------------------------------------------------------------
void vpView::onWebExport()
{
  QFileDialog fileDialog(0, tr("Image Output Directory"));
  fileDialog.setObjectName("GenerateEventClipsDialog");
  fileDialog.setFileMode(QFileDialog::Directory);
  fileDialog.setOption(QFileDialog::ShowDirsOnly);

  if (fileDialog.exec() == QDialog::Accepted)
    {
    this->Core->exportForWeb(qPrintable(fileDialog.selectedFiles().front()), 4);
    }
}

//-----------------------------------------------------------------------------
void vpView::onCreateNewSpatialFilter(bool state)
{
  if (state)
    {
    this->Internal->UI.renderFrame->setCursor(Qt::CrossCursor);
    this->Core->drawFilterRegion();
    }
  else
    {
    this->Core->completeFilterRegion();
    }
}

//-----------------------------------------------------------------------------
void vpView::onSpatialFilterComplete(vtkPoints* contourPoints,
                                     vtkPoints* filterPoints,
                                     const vtkVgTimeStamp* timeStamp,
                                     vtkMatrix4x4* worldToImageMatrix,
                                     const QString& name,
                                     bool enabled)
{
  this->Internal->UI.actionSpatialFilterAdd->setChecked(false);
  this->Internal->UI.renderFrame->setCursor(Qt::ArrowCursor);

  if (!contourPoints || !filterPoints)
    {
    return;
    }

  static int filterId = 1;

  vtkVgTimeStamp invalidTimeStamp;
  QTreeWidgetItem* wi = new QTreeWidgetItem;
  wi->setData(0, Qt::UserRole, QVariant::fromValue<void*>(contourPoints));
  wi->setData(0, Qt::UserRole + 1, QVariant::fromValue<void*>(filterPoints));
  if (timeStamp)
    {
    wi->setData(0, Qt::UserRole + 3, timeStamp->HasFrameNumber()
                                     ? timeStamp->GetFrameNumber()
                                     : invalidTimeStamp.GetFrameNumber());
    wi->setData(0, Qt::UserRole + 4, timeStamp->HasTime()
                                     ? timeStamp->GetTimeInSecs()
                                     : invalidTimeStamp.GetTime());
    }
  else
    {
    wi->setData(0, Qt::UserRole + 3, invalidTimeStamp.GetFrameNumber());
    wi->setData(0, Qt::UserRole + 4, invalidTimeStamp.GetTime());
    }

  if (name.isEmpty())
    {
    wi->setText(0, QString("filter %1").arg(filterId));
    }
  else
    {
    wi->setText(0, name);
    }
  wi->setData(0, Qt::UserRole + 5,
              QVariant::fromValue<void*>(worldToImageMatrix));

  Qt::CheckState initialState = enabled ? Qt::Checked : Qt::Unchecked;
  wi->setCheckState(0, initialState);
  wi->setData(1, Qt::UserRole + 2, initialState);
  wi->setFlags(wi->flags() |  Qt::ItemIsEditable | Qt::ItemIsUserCheckable |
               Qt::ItemIsTristate);

  this->Internal->UI.spatialFilterTree->addTopLevelItem(wi);
  this->Internal->UI.spatialFilterTree->setCurrentItem(wi);

  this->Internal->UI.spatialFiltersDock->show();
  ++filterId;
}

//-----------------------------------------------------------------------------
void vpView::onRemoveSelectedSpatialFilters()
{
  QTreeWidgetItem* root =
    this->Internal->UI.spatialFilterTree->invisibleRootItem();
  foreach (QTreeWidgetItem* wi,
           this->Internal->UI.spatialFilterTree->selectedItems())
    {
    this->Core->removeFilterRegion(
      static_cast<vtkPoints*>(wi->data(0, Qt::UserRole).value<void*>()),
      static_cast<vtkPoints*>(wi->data(0, Qt::UserRole + 1).value<void*>()));
    root->removeChild(wi);
    delete wi;
    }
}

//-----------------------------------------------------------------------------
void vpView::onRemoveAllSpatialFilters()
{
  this->Core->removeAllFilterRegions();
  this->Internal->UI.spatialFilterTree->clear();
}

//-----------------------------------------------------------------------------
void vpView::spatialFilterChanged(QTreeWidgetItem* item)
{
  static bool ignoreChanges = false;
  if (ignoreChanges)
    {
    return;
    }

  // Normally, Qt doesn't let the user directly change an item's state to
  // partially checked. But since we want to be able to do that, we have to
  // set the state ourselves.
  int prevCheckState = item->data(0, Qt::UserRole + 2).toInt();
  if (prevCheckState == Qt::Unchecked && item->checkState(0) == Qt::Checked)
    {
    item->setCheckState(0, Qt::PartiallyChecked);
    return;
    }

  // Record the current checkstate so we know how to transition next time
  ignoreChanges = true;
  item->setData(0, Qt::UserRole + 2, item->checkState(0));
  ignoreChanges = false;

  vtkPoints* contourPoints =
    static_cast<vtkPoints*>(item->data(0, Qt::UserRole).value<void*>());
  vtkPoints* filterPoints =
    static_cast<vtkPoints*>(item->data(0, Qt::UserRole + 1).value<void*>());

  // If the item is partially checked, show the contour but disable the filter
  this->Core->setContourVisible(contourPoints,
                                item->checkState(0) != Qt::Unchecked);
  this->Core->setFilterRegionEnabled(filterPoints,
                                     item->checkState(0) == Qt::Checked);
}

//-----------------------------------------------------------------------------
void vpView::onAddTemporalFilter()
{
  vtkVgTimeStamp start, end;
  start.SetToMinTime();
  end.SetToMaxTime();

  int id =
    this->Core->addTemporalFilter(vtkVgTemporalFilters::FT_Select, start, end);

  this->onTemporalFilterReady(id, QString("filter %1").arg(id),
                              vtkVgTemporalFilters::FT_Select, -1, -1);
}

//-----------------------------------------------------------------------------
void vpView::onTemporalFilterReady(int id, const QString& name,
                                   int type, double start, double end)
{
  QTreeWidgetItem* wi = new QTreeWidgetItem;
  wi->setData(0, Qt::UserRole, id);
  wi->setText(0, name);
  wi->setText(1, vtkVgTemporalFilters::StringForType(
                   vtkVgTemporalFilters::FilterType(type)));
  wi->setData(1, Qt::UserRole, type);

  if (start != -1)
    {
    wi->setText(2, this->setTemporalFilterTimeLabelValue(start));
    }
  wi->setData(2, Qt::UserRole, start);
  if (end != -1)
    {
    wi->setText(3, this->setTemporalFilterTimeLabelValue(end));
    }
  wi->setData(3, Qt::UserRole, end);

  wi->setCheckState(0, Qt::Checked);
  wi->setFlags(wi->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);

  this->Internal->UI.temporalFilterTree->addTopLevelItem(wi);
  this->Internal->UI.temporalFilterTree->setCurrentItem(wi);
}

//-----------------------------------------------------------------------------
void vpView::onTemporalFilterSetStart()
{
  QList<QTreeWidgetItem*> selectedItems =
    this->Internal->UI.temporalFilterTree->selectedItems();
  if (selectedItems.count() != 1)
    {
    return;
    }

  QTreeWidgetItem* wi = selectedItems.first();

  double start =
    this->Core->getUsingTimeStampData() ? this->Core->getCurrentFrameTime()
                                        : this->Core->getCurrentFrameNumber();

  // limit based on end (frame / time)
  double end = wi->data(3, Qt::UserRole).toDouble();

  if (end > 0 && start > end)
    {
    start = end;
    }

  this->Internal->UI.temporalFilterTree->blockSignals(true);
  wi->setData(2, Qt::UserRole, start);
  wi->setText(2, this->setTemporalFilterTimeLabelValue(start));
  this->Internal->UI.temporalFilterTree->blockSignals(false);

  vtkVgTimeStamp startTime;
  if (this->Core->getUsingTimeStampData())
    {
    startTime.SetTime(start);
    }
  else
    {
    start -= (this->Core->getUseZeroBasedFrameNumbers() ? 0 : 1);
    startTime.SetFrameNumber(start);
    }

  this->Core->updateTemporalFilterStart(wi->data(0, Qt::UserRole).toInt(),
                                        startTime);
}

//-----------------------------------------------------------------------------
void vpView::onTemporalFilterSetEnd()
{
  QList<QTreeWidgetItem*> selectedItems =
    this->Internal->UI.temporalFilterTree->selectedItems();
  if (selectedItems.count() != 1)
    {
    return;
    }

  QTreeWidgetItem* wi = selectedItems.first();

  double end =
    this->Core->getUsingTimeStampData() ? this->Core->getCurrentFrameTime()
                                        : this->Core->getCurrentFrameNumber();

  // limit based on end (frame / time)
  double start = wi->data(2, Qt::UserRole).toDouble();

  if (end < start)
    {
    end = start;
    }

  this->Internal->UI.temporalFilterTree->blockSignals(true);
  wi->setData(3, Qt::UserRole, end);
  wi->setText(3, this->setTemporalFilterTimeLabelValue(end));
  this->Internal->UI.temporalFilterTree->blockSignals(false);

  vtkVgTimeStamp endTime;
  if (this->Core->getUsingTimeStampData())
    {
    endTime.SetTime(end);
    }
  else
    {
    end -= (this->Core->getUseZeroBasedFrameNumbers() ? 0 : 1);
    endTime.SetFrameNumber(end);
    }

  this->Core->updateTemporalFilterEnd(wi->data(0, Qt::UserRole).toInt(),
                                      endTime);
}

//-----------------------------------------------------------------------------
void vpView::onRemoveSelectedTemporalFilters()
{
  QTreeWidgetItem* root =
    this->Internal->UI.temporalFilterTree->invisibleRootItem();
  foreach (QTreeWidgetItem* wi,
           this->Internal->UI.temporalFilterTree->selectedItems())
    {
    this->Core->removeTemporalFilter(wi->data(0, Qt::UserRole).toInt());
    root->removeChild(wi);
    delete wi;
    }
}

//-----------------------------------------------------------------------------
void vpView::onRemoveAllTemporalFilters()
{
  this->Core->removeAllTemporalFilters();
  this->Internal->UI.temporalFilterTree->clear();
}

//-----------------------------------------------------------------------------
void vpView::onRemoveAllFilters()
{
  this->Core->removeAllTemporalFilters();
  this->Internal->UI.temporalFilterTree->clear();
  this->Core->removeAllFilterRegions();
  this->Internal->UI.spatialFilterTree->clear();
}

//-----------------------------------------------------------------------------
void vpView::temporalFilterChanged(QTreeWidgetItem* item, int column)
{
  // Fix up invalid columns.
  // TODO: Write a custom delegate for these columns in order to clean this up.
  if (column != 1)
    {
    if (column) // column 2 or 3 (not column 0)
      {
      const double value = item->data(column, Qt::UserRole).toDouble();
      item->setText(column, this->setTemporalFilterTimeLabelValue(value));
      }
    else // did checked state change?
      {
      bool enabled = item->checkState(0) == Qt::Checked;
      this->Core->enableTemporalFilter(item->data(0, Qt::UserRole).toInt(),
                                       enabled);
      }
    return;
    }

  this->Core->updateTemporalFilterType(item->data(0, Qt::UserRole).toInt(),
                                       item->data(1, Qt::UserRole).toInt());
}

//-----------------------------------------------------------------------------
void vpView::bookmarkViewport()
{
  this->Core->saveCameraPosition();
  this->Internal->UI.actionViewportRestore->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vpView::copyViewportExtentsToClipboard()
{
  int extents[4];
  this->Core->getImageSpaceViewportExtents(extents);

  // output in format "WxH[+/-]X[+/-]Y"
  QString str("%1x%2");
  str = str.arg(extents[1] - extents[0] + 1).arg(extents[3] - extents[2] + 1);

  if (extents[0] >= 0)
    {
    str += '+';
    }
  str += QString::number(extents[0]);

  if (extents[2] >= 0)
    {
    str += '+';
    }
  str += QString::number(extents[2]);

  QApplication::clipboard()->setText(str);
}

//-----------------------------------------------------------------------------
void vpView::copyExtendedInfoToClipboard()
{
  QString info;
  info += "frame time: ";
  if (this->Core->getUsingTimeStampData())
    {
    vgUnixTime time(this->Core->getCurrentFrameTime());
    info += time.dateString();
    info += 'T';
    info += time.timeString();
    info += "Z";
    }

  info += "\ncursor position: ";
  info += this->Internal->Coordinates->text();

  double latDist, lonDist;
  double width, height;
  if (this->Internal->UI.renderFrame->underMouse())
    {
    // The mouse is positioned over the render window, so get the width and
    // height of the hovered image pixel.
    info += "\npixel size (at cursor): ";
    if (this->Core->getGsd(this->Internal->RenderWindowMouseCoords,
                           latDist, lonDist, width, height))
      {
      info += QString().sprintf("%.3f x %.3f m (%.15f, %.15f)",
                                width, height, latDist, lonDist);
      }
    }
  else
    {
    // The mouse is outside the render window, so just show the average GSD
    // across the whole image.
    info += "\npixel size: ";
    if (this->Core->getGsd(latDist, lonDist, width, height))
      {
      info += QString().sprintf("%.3f x %.3f m/px (%.15f, %.15f)",
                                width, height, latDist, lonDist);
      }
    }

  QApplication::clipboard()->setText(info);
}

//-----------------------------------------------------------------------------
void vpView::onSettingsChanged()
{
  bool prevUsedTimeIndexing = this->Core->getUsingTimeBasedIndexing();
  this->loadSettings();
  if (this->Core->getUsingTimeBasedIndexing() != prevUsedTimeIndexing)
    {
    this->resetFrameScrubberConfiguration();
    }
  else if (this->Internal->DataLoaded)
    {
    this->updateFrameScrubberRange();
    }
  this->onFrameChange();
  this->updateFrameTime();
  this->updateInfoWidget();
}

//-----------------------------------------------------------------------------
void vpView::loadSettings()
{
  vpSettings settings;
  this->Internal->CoordDisplayMode = settings.coordinateDisplayMode();
  this->Internal->ImageFilteringMode = settings.imageFilteringMode();

  bool enable = this->Internal->ImageFilteringMode != vpSettings::NoFiltering;
  this->Core->setImageFiltering(enable);

  vtkVgTimeStamp trackTrailLength;
  if (settings.trackTrailLengthFrames() != 0)
    {
    trackTrailLength.SetFrameNumber(settings.trackTrailLengthFrames());
    }
  if (settings.trackTrailLengthSeconds() != 0.0)
    {
    trackTrailLength.SetTime(settings.trackTrailLengthSeconds() * 1e6);
    }
  this->Core->setTrackTrailLength(trackTrailLength);

  this->Core->setUseTimeStampDataIfAvailable(settings.useTimeStampData());
  this->Core->setUseTimeBasedIndexing(!settings.forceFrameBasedVideoControls());

  this->Core->setEnableWorldDisplayIfAvailable(settings.worldDisplayEnabled());
  this->Core->setEnableTranslateImage(settings.translateImageEnabled());

  this->Core->setUseZeroBasedFrameNumbers(settings.useZeroBasedFrameNumbers());
  this->Core->setRightClickToEditEnabled(settings.rightClickToEdit());
  this->Core->setAutoAdvanceDuringCreation(settings.autoAdvanceDuringCreation());
  this->Core->setInterpolateToGround(settings.interpolateToGround());
  this->Core->setSceneElementLineWidth(settings.sceneElementLineWidth());

  this->Core->setTrackUpdateChunkSize(settings.streamingTrackUpdateChunkSize());

  this->Core->setRealTimeVideoPlayback(settings.videoPlaybackMode() ==
                                       vpSettings::RealTimePlayback);

  this->Core->setRequestFPS(settings.videoSuggestedFps());

  this->Internal->TrackUpdateTimer->setInterval(
    std::max(1, settings.streamingUpdateInterval()) * 1000);
}

//-----------------------------------------------------------------------------
void vpView::onTestingStarted()
{
  this->Core->setImageFiltering(false);
}

//-----------------------------------------------------------------------------
void vpView::onTestingStopped()
{
  bool enable = this->Internal->ImageFilteringMode != vpSettings::NoFiltering;
  this->Core->setImageFiltering(enable);
}

//-----------------------------------------------------------------------------
void vpView::createTrack(bool start)
{
  if (!start)
    {
    this->Core->stopEditingTrack();
    return;
    }

  // stop event creation
  this->Internal->UI.actionCreateEvent->setChecked(false);
  this->Internal->UI.actionCreateSceneElement->setChecked(false);

  int session = this->Internal->UI.sessionView->GetCurrentSession();

  int id;
  bool ok;
  for (;;)
    {
    id = QInputDialog::getInt(this,
                              tr("Add Track"),
                              tr("Track Id:"),
                              this->Core->getCreateTrackId(session),
                              0, 2147483647, 1, &ok);
    if (!ok)
      {
      this->stopCreatingTrack();
      return;
      }

    if (this->Core->getTrackModel(session)->GetTrack(id))
      {
      QMessageBox::warning(0,
                           QString(),
                           "A track with this ID already exists.");
      }
    else
      {
      break;
      }
    }

  // show the newly created track and begin editing
  if (this->Core->createTrack(id, session))
    {
    this->Core->setCreateTrackId(id + 1, session);
    this->Internal->UI.sessionView->AddAndSelectItem(
      vgObjectTypeDefinitions::Track, id);
    this->Core->beginEditingTrack(id);

    this->Internal->UI.actionViewTrackHeads->setChecked(true);
    if (this->Core->getAutoAdvanceDuringCreation())
      {
      this->Core->setIdOfTrackToFollow(id);
      }
    }
  else
    {
    this->stopCreatingTrack();
    }
}

//-----------------------------------------------------------------------------
void vpView::createEvent(bool start)
{
  if (!start)
    {
    if (this->CreateEventDialog)
      {
      this->CreateEventDialog->reject();
      }
    return;
    }

  // stop track creation
  this->Internal->UI.actionCreateTrack->setChecked(false);
  this->Internal->UI.actionCreateSceneElement->setChecked(false);

  // make only the current project visible
  int session = this->Internal->UI.sessionView->GetCurrentSession();
  for (int i = 0; i < this->Internal->UI.sessionView->GetSessionCount(); ++i)
    {
    this->Core->setProjectVisible(i, i == session);
    }

  // Show events and tracks
  this->Internal->UI.actionViewEvents->setChecked(true);
  this->Internal->UI.actionViewTracks->setChecked(true);
  this->Internal->UI.actionViewTrackHeads->setChecked(true);

  this->Core->update();

  this->CreateEventDialog = new vpCreateEventDialog(this->Core, session, this);

  connect(this->CreateEventDialog,
          SIGNAL(eventCreated(int)), SLOT(onEventCreated(int)));

  connect(this->CreateEventDialog,
          SIGNAL(rejected()), SLOT(stopCreatingEvent()));

  this->CreateEventDialog->initialize();
  this->CreateEventDialog->show();
}

//-----------------------------------------------------------------------------
void vpView::createSceneElement(bool start)
{
  // This is almost the same as createTrack since we will be using the normal
  // track creation machinery.

  if (!start)
    {
    this->Core->stopEditingTrack();
    return;
    }

  // stop event creation
  this->Internal->UI.actionCreateTrack->setChecked(false);
  this->Internal->UI.actionCreateEvent->setChecked(false);

  int session = this->Internal->UI.sessionView->GetCurrentSession();

  int id;
  bool ok;
  for (;;)
    {
    id = QInputDialog::getInt(this,
                              tr("Add Scene Element"),
                              tr("Scene Element Id:"),
                              this->Core->getCreateTrackId(session),
                              0, 2147483647, 1, &ok);
    if (!ok)
      {
      this->stopCreatingTrack();
      return;
      }

    vtkVgTrackModel* model = this->Core->getTrackModel(session);
    id = model->GetTrackIdForSceneElement(id);
    if (model->GetTrack(id))
      {
      QMessageBox::warning(0,
                           QString(),
                           "A object with this ID already exists.");
      }
    else
      {
      break;
      }
    }

  // show the newly created track and begin editing
  if (this->Core->createTrack(id, session, true))
    {
    this->Core->setCreateTrackId(id + 1, session);
    this->Internal->UI.sessionView->AddAndSelectItem(
      vgObjectTypeDefinitions::SceneElement, id);
    this->Core->beginEditingTrack(id);

    this->Internal->UI.actionViewSceneElements->setChecked(true);
    if (this->Core->getAutoAdvanceDuringCreation())
      {
      this->Core->setIdOfTrackToFollow(id);
      }
    }
  else
    {
    this->stopCreatingTrack();
    }
}

//-----------------------------------------------------------------------------
void vpView::mergeTracks(bool start)
{
  if (!start)
    {
    if (this->MergeTracksDialog)
      {
      this->MergeTracksDialog->reject();
      }
    return;
    }

  // stop track / event creation
  this->Internal->UI.actionCreateTrack->setChecked(false);
  this->Internal->UI.actionCreateEvent->setChecked(false);

  // make only the current project visible
  int session = this->Internal->UI.sessionView->GetCurrentSession();
  for (int i = 0; i < this->Internal->UI.sessionView->GetSessionCount(); ++i)
    {
    this->Core->setProjectVisible(i, i == session);
    }

  // show tracks only
  this->Internal->UI.actionViewActivities->setChecked(false);
  this->Internal->UI.actionViewEvents->setChecked(false);
  this->Internal->UI.actionViewTracks->setChecked(true);
  this->Internal->UI.actionViewTrackHeads->setChecked(true);

  this->Core->update();

  this->MergeTracksDialog = new vpMergeTracksDialog(this->Core, session, this);

  connect(this->MergeTracksDialog,
          SIGNAL(tracksMerged(int)), SLOT(onTracksMerged(int)));

  connect(this->MergeTracksDialog,
          SIGNAL(rejected()), SLOT(stopMergingTracks()));

  this->MergeTracksDialog->initialize();
  this->MergeTracksDialog->show();
}

//-----------------------------------------------------------------------------
void vpView::stopCreatingTrack()
{
  this->Internal->UI.actionCreateTrack->setChecked(false);
  this->Internal->UI.actionCreateSceneElement->setChecked(false);
}

//-----------------------------------------------------------------------------
void vpView::stopCreatingEvent()
{
  this->Internal->UI.actionCreateEvent->setChecked(false);
  delete this->CreateEventDialog;
  this->CreateEventDialog = 0;
}

//-----------------------------------------------------------------------------
void vpView::stopMergingTracks()
{
  this->Internal->UI.actionMergeTracks->setChecked(false);
  delete this->MergeTracksDialog;
  this->MergeTracksDialog = 0;
}

//-----------------------------------------------------------------------------
void vpView::onEventCreated(int id)
{
  this->Internal->UI.sessionView->SetCurrentTab(vgObjectTypeDefinitions::Event);
  this->rebuildObjectViews();
  this->Internal->UI.sessionView->SelectItem(vgObjectTypeDefinitions::Event, id);
  this->Core->updateScene();
}

//-----------------------------------------------------------------------------
void vpView::onTracksMerged(int id)
{
  this->Internal->UI.sessionView->SetCurrentTab(vgObjectTypeDefinitions::Track);
  this->rebuildObjectViews();
  this->Internal->UI.sessionView->SelectItem(vgObjectTypeDefinitions::Track, id);
  this->Core->updateScene();
}

//-----------------------------------------------------------------------------
void vpView::splitTrack(int id, int sessionId)
{
  std::vector<vtkVgEvent*> events;
  this->Core->getEventModel(sessionId)->GetEvents(id, events);

  if (!events.empty())
    {
    QMessageBox::StandardButton btn =
      QMessageBox::question(0, "Split Events?",
                            QString("The track being split is referenced by %1 "
                                    "events. Splitting the track will cause "
                                    "event track references to be updated "
                                    "and events to be split apart, as "
                                    "necessary.")
                            .arg(events.size()),
                            QMessageBox::Ok | QMessageBox::Cancel);

    if (btn != QMessageBox::Ok)
      {
      return;
      }
    }

  int newId;
  bool ok;
  for (;;)
    {
    newId = QInputDialog::getInt(this,
                                 tr("Split Track"),
                                 tr("Second Track Id:"),
                                 this->Core->getCreateTrackId(sessionId),
                                 0, 2147483647, 1, &ok);
    if (!ok)
      {
      return;
      }

    if (this->Core->getTrackModel(sessionId)->GetTrack(newId))
      {
      QMessageBox::warning(0,
                           QString(),
                           "A track with this ID already exists.");
      }
    else
      {
      break;
      }
    }

  // try to split the track
  if (this->Core->splitTrack(id, newId, sessionId))
    {
    this->Core->setCreateTrackId(newId + 1, sessionId);
    this->Internal->UI.sessionView->AddAndSelectItem(
      vgObjectTypeDefinitions::Track, newId);
    }
}

//-----------------------------------------------------------------------------
void vpView::improveTrack(int id, int sessionId)
{
  this->Core->improveTrack(id, sessionId);
}

//-----------------------------------------------------------------------------
void vpView::addEventsToGraphModel(QList<int> eventIds, int sessionId)
{
  vtkVgEventModel* model = this->Core->getEventModel(sessionId);
  std::vector<vtkVgEvent*> events;

  foreach (int id, eventIds)
    {
    if (vtkVgEvent* event = model->GetEvent(id))
      {
      events.push_back(event);
      }
    }

  this->GraphModelWidget->addEventNodes(events);
}

//-----------------------------------------------------------------------------
void vpView::addTrackEventsToGraphModel(int id, int sessionId)
{
  vtkVgEventModel* model = this->Core->getEventModel(sessionId);
  vtkVgEventFilter* filter = this->Core->getEventFilter();

  std::vector<vtkVgEvent*> events;
  model->GetEvents(id, events);

  // Remove all the events are not turned on or don't pass filters
  for (size_t i = 0; i < events.size();)
    {
    vtkVgEvent* event = events[i];
    vtkVgEventInfo info = model->GetEventInfo(event->GetId());
    if (!info.GetDisplayEvent() || !info.GetPassesFilters() ||
        filter->GetBestClassifier(event) < 0)
      {
      // This is slow, but not expecting it to matter
      events.erase(events.begin() + i);
      }
    else
      {
      ++i;
      }
    }

  this->GraphModelWidget->addEventNodes(events);
}

//-----------------------------------------------------------------------------
void vpView::selectEvent(int id)
{
  if (this->Internal->Selecting)
    {
    return;
    }

  this->Internal->UI.sessionView->SelectItem(vgObjectTypeDefinitions::Event,
                                             id);
}

//-----------------------------------------------------------------------------
void vpView::exportTracks()
{
  if (this->Internal->UI.actionCreateTrack->isChecked())
    {
    this->onWarningError(
      "Cannot export tracks while a track is being created.");
    }
  else
    {
    this->Core->exportTracksToFile();
    }
}

//-----------------------------------------------------------------------------
void vpView::exportEvents()
{
  if (this->Internal->UI.actionCreateEvent->isChecked())
    {
    this->onWarningError(
      "Cannot export events while an event is being created.");
    }
  else
    {
    this->Core->exportEventsToFile();
    }
}

//-----------------------------------------------------------------------------
void vpView::exportSceneElements()
{
  if (this->Internal->UI.actionCreateSceneElement->isChecked())
    {
    this->onWarningError(
      "Cannot export scene elements while one is being created.");
    }
  else
    {
    this->Core->exportSceneElementsToFile();
    }
}

//-----------------------------------------------------------------------------
void vpView::exportFilters()
{
  QString path = vgFileDialog::getSaveFileName(
                   this, "Save Filters", QString(),
                   "vpView filters (*.txt);;");

  if (path.isEmpty())
    {
    return;
    }
  this->exportFilters(path, false);
}

//-----------------------------------------------------------------------------
void vpView::exportFilters(QString path, bool startExternalProcess)
{
  std::ofstream file(qPrintable(path), ios::out | ios::trunc);
  if (!file)
    {
    this->onWarningError("Unable to write filter file.");
    return;
    }

  // Write out the spatial filters, one per line
  for (int i = 0,
       count = this->Internal->UI.spatialFilterTree->topLevelItemCount();
       i < count; ++i)
    {
    QTreeWidgetItem* item =
      this->Internal->UI.spatialFilterTree->topLevelItem(i);

    vtkPoints* contourPoints =
      static_cast<vtkPoints*>(item->data(0, Qt::UserRole).value<void*>());
    vtkPoints* filterPoints =
      static_cast<vtkPoints*>(item->data(0, Qt::UserRole + 1).value<void*>());
    unsigned int frameNumber = item->data(0, Qt::UserRole + 3).toUInt();
    double time = item->data(0, Qt::UserRole + 4).toDouble();
    vtkMatrix4x4* worldtoImageMatrix =
      static_cast<vtkMatrix4x4*>(
        item->data(0, Qt::UserRole + 5).value<void*>());

    this->Core->writeSpatialFilter(contourPoints, filterPoints,
                                   frameNumber, time, worldtoImageMatrix,
                                   stdString(item->text(0)), file);
    }

  // Write out the temporal filters, one per line
  for (int i = 0,
       count = this->Internal->UI.temporalFilterTree->topLevelItemCount();
       i < count; ++i)
    {
    QTreeWidgetItem* item =
      this->Internal->UI.temporalFilterTree->topLevelItem(i);

    int type = item->data(1, Qt::UserRole).toInt();
    double start = item->data(2, Qt::UserRole).toDouble();
    double end = item->data(3, Qt::UserRole).toDouble();
    this->Core->writeTemporalFilter(type, start, end,
                                    stdString(item->text(0)), file);
    }
  file.close();

  if (startExternalProcess)
    {
    this->Core->startExternalProcess();
    }
}

//-----------------------------------------------------------------------------
void vpView::setTrackTrailLength()
{
  vpSettings settings;
  vtkVgTimeStamp trailLength = this->Core->getTrackTrailLength();
  if ((this->Internal->DataLoaded && this->Core->getUsingTimeStampData()) ||
      (!this->Internal->DataLoaded && settings.useTimeStampData()))
    {
    double prevLength = trailLength.HasTime() ? trailLength.GetTime() * 1e-6
                                              : 0.0;
    bool ok = false;
    double length = QInputDialog::getDouble(
                      this, "Set Track Trail Length",
                      "Trail time (seconds, 0 = unlimited)",
                      prevLength, 0.0, static_cast<double>(INT_MAX), 1, &ok);
    if (ok)
      {
      settings.setTrackTrailLengthSeconds(length);
      settings.commit();

      vtkVgTimeStamp ts;
      if (length != 0.0)
        {
        ts.SetTime(length * 1e6);
        }
      this->Core->setTrackTrailLength(ts);
      this->Core->updateScene();
      }
    }
  else
    {
    // Frame numbers only
    int prevLength =
      trailLength.HasFrameNumber() ? trailLength.GetFrameNumber() : 0;
    bool ok = false;
    int length = QInputDialog::getInt(
                   this, "Set Track Trail Length", "Trail frames (0 = unlimited)",
                   prevLength, 0, INT_MAX, 1, &ok);
    if (ok)
      {
      settings.setTrackTrailLengthFrames(length);
      settings.commit();

      vtkVgTimeStamp ts;
      if (length != 0)
        {
        ts.SetFrameNumber(length);
        }
      this->Core->setTrackTrailLength(ts);
      this->Core->updateScene();
      }
    }
}

//-----------------------------------------------------------------------------
void vpView::changeTrackColors()
{
  vpTrackColorDialog dlg(this->Core->getTrackAttributes());

  int trackColorMode = this->Core->getTrackColorMode();
  switch (trackColorMode)
    {
    case vtkVgTrackRepresentationBase::TCM_StateAttrs:
      {
      dlg.setMode(vpTrackColorDialog::ColorByStateAttribute);
      break;
      }
    case vtkVgTrackRepresentationBase::TCM_PVO:
      {
      dlg.setMode(vpTrackColorDialog::ColorByStatePVO);
      break;
      }
    case vtkVgTrackRepresentationBase::TCM_Random:
      {
      dlg.setMode(vpTrackColorDialog::RandomColor);
      break;
      }
    default:
      {
      dlg.setMode(vpTrackColorDialog::SingleColor);
      }
    }
  dlg.setAttributeGroup(this->Core->getTrackAttributeGroup());

  connect(&dlg, SIGNAL(updateRequested(vpTrackColorDialog*)),
          this, SLOT(updateTrackColorsFromDialog(vpTrackColorDialog*)));

  if (dlg.exec() == QDialog::Accepted)
    {
    this->updateTrackColorsFromDialog(&dlg);
    }
}

//-----------------------------------------------------------------------------
void vpView::updateTrackColorsFromDialog(vpTrackColorDialog* dlg)
{
  switch (dlg->mode())
    {
    case vpTrackColorDialog::ColorByStateAttribute:
      {
      this->Core->setTrackColorMode(
        vtkVgTrackRepresentationBase::TCM_StateAttrs, dlg->attributeGroup());
      break;
      }
    case vpTrackColorDialog::ColorByStatePVO:
      {
      this->Core->setTrackColorMode(
        vtkVgTrackRepresentationBase::TCM_PVO, dlg->attributeGroup());
      break;
      }
    case vpTrackColorDialog::RandomColor:
      {
      this->Core->setTrackColorMode(
        vtkVgTrackRepresentationBase::TCM_Random, dlg->attributeGroup());
      break;
      }
    case vpTrackColorDialog::SingleColor:
    default:
      {
      this->Core->setTrackColorMode(
        vtkVgTrackRepresentationBase::TCM_Model, dlg->attributeGroup());
      }
    }
}

//-----------------------------------------------------------------------------
void vpView::onEventExpirationModeChange(QAction* action, bool render)
{
  vpViewCore::enumEventExpirationMode mode;
  if (action == this->Internal->UI.actionShowEventsUntilEventEnd)
    {
    mode = vpViewCore::ShowUntilEventEnd;
    }
  else
    {
    mode = vpViewCore::ShowUntilTrackEnd;
    }
  this->Core->setEventExpirationMode(mode, render);
}

//-----------------------------------------------------------------------------
void vpView::setFrameOffset()
{
  int prevOffset = this->Core->getFrameNumberOffset();

  bool ok = false;
  int offset = QInputDialog::getInt(
                 this, "Set Image Data Frame Offset", "Offset",
                 prevOffset, 0, INT_MAX, 1, &ok);
  if (ok)
    {
    this->Core->setFrameNumberOffset(offset);

    this->Internal->UI.currentFrame->setFrameNumberRange(
      this->Core->getMinimumFrameNumber(), this->Core->getMaximumFrameNumber());

    vtkVgTimeStamp time;
    time.SetFrameNumber(this->Core->getCurrentFrameIndex() + offset);
    this->Core->setCurrentTime(time);
    this->Core->update();
    }
}

//-----------------------------------------------------------------------------
void vpView::importProject()
{
  this->Core->importProject();
  this->updateObjectCounts();
}

//-----------------------------------------------------------------------------
void vpView::closeProject(int sessionId)
{
  this->Core->closeProject(sessionId);
  this->Internal->UI.sessionView->RemoveSession(sessionId);
  this->updateEverything();
}

//-----------------------------------------------------------------------------
void vpView::updateTracks()
{
  if (this->Internal->TrackUpdateInProgress)
    {
    return;
    }

  this->Internal->TrackUpdateInProgress = true;
  if (this->Core->updateTracks() == VTK_ERROR)
    {
    this->Internal->TrackUpdateInProgress = false;
    }
}

//-----------------------------------------------------------------------------
void vpView::finishTrackUpdate()
{
  this->Internal->TrackUpdateInProgress = false;
  this->updateEverything();
  this->updateObjectCounts();
}

//-----------------------------------------------------------------------------
void vpView::setAutoUpdateEnabled(bool enable)
{
  if (!this->Internal->DataLoaded)
    {
    return;
    }

  if (!enable)
    {
    this->Internal->TrackUpdateTimer->stop();
    return;
    }

  this->updateTracks();
  this->Internal->TrackUpdateTimer->start();
}

//-----------------------------------------------------------------------------
void vpView::disableTimeBasedIndexing()
{
  vpSettings settings;
  settings.setForceFrameBasedVideoControls(true);
  settings.commit();
}

//-----------------------------------------------------------------------------
void vpView::updatePlaybackState()
{
  if (this->Core->isPlaying())
    {
    this->updatePlaybackRate(this->Core->getPlaybackRate());
    }
  else
    {
    this->Internal->UI.playbackSpeed->setText("P");
    }
}

//-----------------------------------------------------------------------------
void vpView::updatePlaybackRate(qreal rate)
{
  this->Internal->UI.playbackSpeed->setText(
    QString("%1x").arg(static_cast<int>(rate)));
}

//-----------------------------------------------------------------------------
void vpView::pickTimeInterval()
{
  QEventLoop eventLoop;
  vpSelectTimeIntervalDialog selectTime(this->Core, this->GraphModelWidget);

  connect(&selectTime, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));

  selectTime.show();
  eventLoop.exec();

  this->GraphModelWidget->setMeasuredTimeInterval(
    selectTime.getDurationInSeconds());
}

//-----------------------------------------------------------------------------
void vpView::updateFrameScrubberRange()
{
  if (this->Core->getUsingTimeBasedIndexing())
    {
    this->Internal->UI.currentFrame->setTimeRange(
      this->Core->getMinimumTime(), this->Core->getMaximumTime());
    }
  else
    {
    this->Internal->UI.currentFrame->setFrameNumberRange(
      this->Core->getMinimumFrameNumber(), this->Core->getMaximumFrameNumber());
    }
}

//-----------------------------------------------------------------------------
void vpView::resetFrameScrubberConfiguration()
{
  if (this->Core->getUsingTimeBasedIndexing())
    {
    this->Internal->UI.currentFrame->blockSignals(true);
    this->Internal->UI.currentFrame->setTimeRange(
      this->Core->getMinimumTime(), this->Core->getMaximumTime());
    this->Internal->UI.currentFrame->blockSignals(false);
    this->Internal->UI.minFrame->setVisible(false);
    this->Internal->UI.minTime->setVisible(true);
    this->Internal->UI.minTime->setEnabled(
      this->Internal->UI.minFrameCheckBox->checkState() == Qt::Checked);
    this->Internal->UI.minFrameCheckBox->setText("Min Time");
    }
  else
    {
    this->Internal->UI.currentFrame->blockSignals(true);
    this->Internal->UI.currentFrame->setFrameNumberRange(
      this->Core->getMinimumFrameNumber(), this->Core->getMaximumFrameNumber());
    this->Internal->UI.currentFrame->blockSignals(false);
    this->Internal->UI.minFrame->setVisible(true);
    this->Internal->UI.minFrame->setEnabled(
      this->Internal->UI.minFrameCheckBox->checkState() == Qt::Checked);
    this->Internal->UI.minTime->setVisible(false);
    this->Internal->UI.minFrameCheckBox->setText("Min Frame");
    }
  this->onFrameChange();
}

//-----------------------------------------------------------------------------
QString vpView::setTemporalFilterTimeLabelValue(double time)
{
  QString ts;
  if (this->Core->getUsingTimeStampData())
    {
    vgUnixTime unixTime(time);
    ts = unixTime.timeString();
    }
  else
    {
    ts = QString("%1").arg(time);
    }
  return ts;
}

//-----------------------------------------------------------------------------
void vpView::onProjectProcessed()
{
#ifdef VISGUI_USE_SUPER3D
  this->SuperResWidget->disableGreyscaleOption(
    this->Core->getImagesAreGreyscale());

  if (!this->Core->cameraDirectory().isEmpty())
    {
    this->SuperResWidget->loadCameras(
      stdString(this->Core->cameraDirectory()));
    }

  if (!this->Core->bundleAdjustmentConfigFile().isEmpty())
    {
    this->SuperResWidget->setBundleAjustmentConfigFile(
      stdString(this->Core->bundleAdjustmentConfigFile()));
    }

  if (!this->Core->depthConfigFile().isEmpty())
    {
    this->SuperResWidget->loadDepthConfigFile(
      stdString(this->Core->depthConfigFile()));
    }

  this->SuperResWidget->enableOptions();
#endif
}
