// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsContextViewerPlugin.h"

#include "vsContextViewer.h"

#include <vsCore.h>
#include <vsMainWindow.h>
#include <vsScene.h>

#include <qtActionManager.h>
#include <qtCliArgs.h>
#include <qtPrioritizedMenuProxy.h>

#include <QDockWidget>
#include <QMenu>
#include <QtPlugin>

//-----------------------------------------------------------------------------
vsContextViewerPlugin::vsContextViewerPlugin()
{
}

//-----------------------------------------------------------------------------
vsContextViewerPlugin::~vsContextViewerPlugin()
{
}

//-----------------------------------------------------------------------------
void vsContextViewerPlugin::initialize(vsCore* core)
{
  vsUiExtensionInterface::initialize(core);
}

//-----------------------------------------------------------------------------
void vsContextViewerPlugin::createInterface(
  vsMainWindow* window, vsScene* scene)
{
  // Create dock widget
  QDockWidget* dock = new QDockWidget(window);
  dock->setObjectName("contextViewDock");
  dock->setWindowTitle("Context View");

  // Create change detection widget
  vsContextViewer* contents = new vsContextViewer(this->core(), scene);

  // Create action to toggle dock visibility
  QAction* const toggleDockAction =
    new QAction("Show Conte&xt View", window);
  toggleDockAction->setObjectName("actionWindowContextViewShow");
  toggleDockAction->setToolTip("Show or hide the context view");
  toggleDockAction->setShortcut(QKeySequence::fromString("Ctrl+D, X"));
  toggleDockAction->setCheckable(true);
  toggleDockAction->setChecked(true);

  QAction* const loadContextAction = new QAction("&Load Context", window);
  loadContextAction->setObjectName("actionContextLoad");
  loadContextAction->setShortcut(QKeySequence::fromString("Ctrl+O, X"));

  QSettings settings;
  qtAm->setupAction(settings, toggleDockAction,
                    "MainWindow/actionWindowContextViewShow");
  qtAm->setupAction(settings, loadContextAction,
                    "MainWindow/actionContextLoad");

  // Add widget to dock, dock to window, and toggle action to menu
  dock->setWidget(contents);
  window->addDockWidget(dock, Qt::RightDockWidgetArea, toggleDockAction);

  qtPrioritizedMenuProxy* menu = window->toolsMenu();
  menu->insertAction(loadContextAction, 150);
  menu->insertAction(toggleDockAction, 150);

  // Connect actions to their slots
  connect(loadContextAction, SIGNAL(triggered()),
          contents, SLOT(loadContext()));

  connect(this->core(), SIGNAL(trackAdded(vtkVgTrack*)),
          contents, SLOT(updateTrackMarker(vtkVgTrack*)));
  connect(this->core(), SIGNAL(trackChanged(vtkVgTrack*)),
          contents, SLOT(updateTrackMarker(vtkVgTrack*)));

  connect(this->core(), SIGNAL(eventAdded(vtkVgEvent*, vsEventId)),
          contents, SLOT(updateEventMarker(vtkVgEvent*)));
  connect(this->core(), SIGNAL(eventRegionsChanged(vtkVgEvent*)),
          contents, SLOT(updateEventMarker(vtkVgEvent*)));
  connect(this->core(), SIGNAL(eventChanged(vtkVgEvent*)),
          contents, SLOT(updateRegionlessEventMarker(vtkVgEvent*)));

  connect(this->core(), SIGNAL(trackAdded(vtkVgTrack*)),
          contents, SLOT(updateTrackInfo(vtkVgTrack*)));
  connect(this->core(), SIGNAL(trackChanged(vtkVgTrack*)),
          contents, SLOT(updateTrackInfo(vtkVgTrack*)));
  connect(this->core(), SIGNAL(trackNoteChanged(vtkVgTrack*, QString)),
          contents, SLOT(updateTrackInfo(vtkVgTrack*)));

  connect(this->core(), SIGNAL(eventAdded(vtkVgEvent*, vsEventId)),
          contents, SLOT(updateEventInfo(vtkVgEvent*)));
  connect(this->core(), SIGNAL(eventChanged(vtkVgEvent*)),
          contents, SLOT(updateEventInfo(vtkVgEvent*)));
  connect(this->core(), SIGNAL(eventNoteChanged(vtkVgEvent*, QString)),
          contents, SLOT(updateEventInfo(vtkVgEvent*)));

  connect(scene, SIGNAL(trackSceneUpdated()),
          contents, SLOT(updateTrackMarkers()));
  connect(scene, SIGNAL(eventSceneUpdated()),
          contents, SLOT(updateEventMarkers()));

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

  if (!this->ContextUri.isEmpty())
    {
    contents->addRasterLayer(QUrl::fromUserInput(this->ContextUri));
    }
}

//-----------------------------------------------------------------------------
void vsContextViewerPlugin::registerExtensionCliOptions(
  qtCliOptions& options)
{
  options.add("context <uri>", "Add context layer from 'uri' on startup");
}

//-----------------------------------------------------------------------------
void vsContextViewerPlugin::parseExtensionArguments(const qtCliArgs& args)
{
  // Cache CLI option
  this->ContextUri = args.value("context");
}
