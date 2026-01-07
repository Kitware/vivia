// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsTimelineViewerPlugin.h"

#include "vsTimelineViewer.h"

#include <vsCore.h>
#include <vsMainWindow.h>
#include <vsScene.h>

#include <qtActionManager.h>
#include <qtPrioritizedMenuProxy.h>

#include <QDockWidget>
#include <QMenu>
#include <QtPlugin>

//-----------------------------------------------------------------------------
vsTimelineViewerPlugin::vsTimelineViewerPlugin()
{
}

//-----------------------------------------------------------------------------
vsTimelineViewerPlugin::~vsTimelineViewerPlugin()
{
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPlugin::initialize(vsCore* core)
{
  vsUiExtensionInterface::initialize(core);
}

//-----------------------------------------------------------------------------
void vsTimelineViewerPlugin::createInterface(
  vsMainWindow* window, vsScene* scene)
{
  // Create dock widget
  QDockWidget* dock = new QDockWidget(window);
  dock->setObjectName("timelineViewDock");
  dock->setWindowTitle("Timeline View");

  // Create change detection widget
  vsTimelineViewer* contents = new vsTimelineViewer(this->core(), scene);

  // Create action to toggle dock visibility
  QAction* const toggleDockAction =
    new QAction("Show &Timeline View", window);
  toggleDockAction->setObjectName("actionWindowTimelineViewShow");
  toggleDockAction->setToolTip("Show or hide the timeline view");
  toggleDockAction->setShortcut(QKeySequence::fromString("Ctrl+D, L"));
  toggleDockAction->setCheckable(true);
  toggleDockAction->setChecked(true);

  QSettings settings;
  qtAm->setupAction(settings, toggleDockAction,
                    "MainWindow/actionWindowTimelineViewShow");

  // Add widget to dock, dock to window, and toggle action to menu
  dock->setWidget(contents);
  window->addDockWidget(dock, Qt::RightDockWidgetArea, toggleDockAction);

  qtPrioritizedMenuProxy* menu = window->toolsMenu();
  menu->insertAction(toggleDockAction, 160);

  // Connect actions to their slots
  connect(this->core(), SIGNAL(trackAdded(vtkVgTrack*)),
          contents, SLOT(addTrack(vtkVgTrack*)));
  connect(this->core(), SIGNAL(trackChanged(vtkVgTrack*)),
          contents, SLOT(addTrack(vtkVgTrack*)));

  connect(this->core(), SIGNAL(eventAdded(vtkVgEvent*, vsEventId)),
          contents, SLOT(addEvent(vtkVgEvent*)));
  connect(this->core(), SIGNAL(eventChanged(vtkVgEvent*)),
          contents, SLOT(addEvent(vtkVgEvent*)));

  connect(scene, SIGNAL(videoMetadataUpdated(vtkVgVideoFrameMetaData, qint64)),
          contents, SLOT(updateTimeFromMetadata(vtkVgVideoFrameMetaData)));

  connect(scene, SIGNAL(trackSceneUpdated()),
          contents, SLOT(updateTracks()));
  connect(scene, SIGNAL(eventSceneUpdated()),
          contents, SLOT(updateEvents()));

  connect(scene, SIGNAL(selectedTracksChanged(QSet<vtkIdType>)),
          contents, SLOT(setSelectedTracks(QSet<vtkIdType>)));
  connect(contents, SIGNAL(trackSelectionChanged(QSet<vtkIdType>)),
          scene, SLOT(setTrackSelection(QSet<vtkIdType>)));

  connect(scene, SIGNAL(selectedEventsChanged(QSet<vtkIdType>)),
          contents, SLOT(setSelectedEvents(QSet<vtkIdType>)));
  connect(contents, SIGNAL(eventSelectionChanged(QSet<vtkIdType>)),
          scene, SLOT(setEventSelection(QSet<vtkIdType>)));

  connect(contents, SIGNAL(trackPicked(vtkIdType)),
          scene, SLOT(jumpToTrack(vtkIdType)));
  connect(contents, SIGNAL(eventPicked(vtkIdType)),
          scene, SLOT(jumpToEvent(vtkIdType)));
}
