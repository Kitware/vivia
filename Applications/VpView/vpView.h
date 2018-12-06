/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpView_h
#define __vpView_h

#include <QMainWindow>

class qtCliArgs;

class vtkIdList;
class vtkMatrix4x4;
class vtkPoints;

class vtkVgTimeStamp;

class pqCoreTestUtility;

class vpCreateEventDialog;
class vpGraphModelWidget;
class vpMergeTracksDialog;
class vpQtViewer3dDialog;
class vpTimelineDialog;
class vpTrackColorDialog;
class vpViewCore;

class QTreeWidgetItem;

class vpView : public QMainWindow
{
  Q_OBJECT

public:
  vpView();
  ~vpView();

  void initialize(const qtCliArgs* cliArgs);

signals:
  void windowSizeChanged(int, int);
  void quitApp();
  void eventSelected(int id);

public slots:
  void onInitialize();
  void onDataLoaded();
  void onDataChanged();
  void onIconsLoaded();
  void onOverviewLoaded();
  void onEnterAdjudicationMode();
  void onFollowTrackChange(int trackId);
  void onExitFollowTrack();
  void toggleAdjudicationMode(bool state);
  void exitApp();
  void onFrameChange();
  void onTimeChange(double now);
  void onReachedPlayBoundary();
  void onFastBackward();
  void onReversePause();
  void onPause();
  void onPlayPause();
  void onFastForward();
  void onLoopToggle(bool state);
  void onFrameUpdate(int minFrameNumber, int frameNumber, bool resize);
  void onTimeUpdate(double minTime, double time, bool resize);
  void updateMinTime(double time);
  void onFrameIntervalEnabled(int state);
  void onTreeSelectionChanged(int sessionId);
  void onTreeHoverItemChanged(int sessionId);
  void updateObject(int objectType, int id);
  void updateColorofTracksOfType(int typeIndex, double *rgb);
  void updateCore();
  void updateEverything();
  void onCreateEvent(int type, vtkIdList* ids);
  void onTimelineSelectionChanged(int type, int id);
  void onShowHideTimelineView(bool show);
  void onTimelineDialogClosed();
  void onDisplayEventLegend(bool state);
  void onDisplayEventIcons(bool state);
  void onIconSizeChange(QAction* sizeAction, bool render = true);
  void onDisplayOverview(bool state);
  void onDisplayAOIOutline(bool state);
  void onCriticalError(const QString&);
  void onWarningError(const QString&);
  void onShowConfigureDialog();
  void onShowExternalProcessDialog();
  void onSaveRenderedImages(bool state);
  void onExecuteModeChanged(int index);
  void onEventFilterPresetChanged(int index);
  void onEventFilterVisibilityChanged();
  void onEventFilterChanged();
  void onFrameRendered();
  void onReinitialized();
  void onShow3dView(bool state);
  void onHide3dView();
  void onEventExpirationModeChange(QAction* sizeAction, bool render = true);
  void onContextLODChanged(int value);
  void onWebExport();

  void updateFrameTime();
  void updateObjectCounts();
  void updateUI();

  void handleRenderWindowMouseMove(int x, int y);

  bool eventFilter(QObject* obj, QEvent* event);

  void onCreateNewSpatialFilter(bool state);
  void onSpatialFilterComplete(vtkPoints* contourPoints,
                               vtkPoints* filterPoints,
                               const vtkVgTimeStamp* timeStamp,
                               vtkMatrix4x4* worldToImageMatrix,
                               const QString& name,
                               bool enabled);
  void onTemporalFilterReady(int id, const QString& name,
                             int type, double start, double end);

  void onRemoveSelectedSpatialFilters();
  void onRemoveAllSpatialFilters();

  void spatialFilterChanged(QTreeWidgetItem* item);

  void onAddTemporalFilter();
  void onTemporalFilterSetStart();
  void onTemporalFilterSetEnd();
  void onRemoveSelectedTemporalFilters();
  void onRemoveAllTemporalFilters();
  void onRemoveAllFilters();

  void temporalFilterChanged(QTreeWidgetItem* item, int column);

  void bookmarkViewport();
  void copyViewportExtentsToClipboard();
  void copyExtendedInfoToClipboard();

  void updateInfoWidget();
  void rebuildObjectViews();

  void onSettingsChanged();

  void onTestingStarted();
  void onTestingStopped();

  void createTrack(bool start);
  void createEvent(bool start);
  void createSceneElement(bool start);

  void mergeTracks(bool start);

  void stopCreatingTrack();
  void stopCreatingEvent();
  void stopMergingTracks();

  void onEventCreated(int id);
  void onTracksMerged(int id);

  void splitTrack(int id, int sessionId);
  void improveTrack(int id, int sessionId);

  void addEventsToGraphModel(QList<int> eventIds, int sessionId);
  void addTrackEventsToGraphModel(int id, int sessionId);

  void selectEvent(int id);

  void exportTracks();
  void exportEvents();
  void exportSceneElements();
  void exportFilters();
  void exportFilters(QString path, bool startExternalProcess);

  void setTrackTrailLength();

  void changeTrackColors();
  void updateTrackColorsFromDialog(vpTrackColorDialog* dlg);

  void setFrameOffset();

  void importProject();
  void closeProject(int sessionId);

  void updateTracks();
  void finishTrackUpdate();

  void setAutoUpdateEnabled(bool enable);

  void disableTimeBasedIndexing();

  void updatePlaybackState();
  void updatePlaybackRate(qreal rate);

  void pickTimeInterval();

  void executeExternalProcess();
  void executeEmbeddedPipeline(const QString& pipelinePath);

  void updateTrackFilters();

protected:
  virtual void closeEvent(QCloseEvent*);  // reimplemented from QWidget

private:
  void parseCommandLine(const qtCliArgs& args);
  void processCommandLine();

  void setupDock();

  void postLoadConfig();

  void postDataLoaded();

  void updateCoordinateDisplay();
  void updateGsdDisplay();

  void showGsd(double width, double height, const QString& suffix = QString());

  void loadSettings();

  void updateFrameScrubberRange();
  void resetFrameScrubberConfiguration();

  QString setTemporalFilterTimeLabelValue(double time);

  class vtkInternal;
  vtkInternal*             Internal;

  // The core;
  vpViewCore*             Core;

  vpCreateEventDialog*    CreateEventDialog;
  vpMergeTracksDialog*    MergeTracksDialog;
  vpTimelineDialog*       TimelineDialog;
  vpQtViewer3dDialog*     Viewer3dDialog;

  pqCoreTestUtility*      TestUtility;

  vpGraphModelWidget*     GraphModelWidget;
};

#endif // __vpView_h
