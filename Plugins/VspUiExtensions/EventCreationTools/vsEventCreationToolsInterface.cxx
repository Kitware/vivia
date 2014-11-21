/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsEventCreationToolsInterface.h"

#include <QComboBox>
#include <QMenu>
#include <QSignalMapper>
#include <QToolBar>
#include <QToolButton>

#include <vtkRenderWindow.h>

#include <QVTKWidget.h>

#include <qtPrioritizedMenuProxy.h>
#include <qtPrioritizedToolBarProxy.h>
#include <qtScopedValueChange.h>
#include <qtUtil.h>

#include <vtkVgRegionWidget.h>

#include <vsCore.h>
#include <vsMainWindow.h>
#include <vsScene.h>

QTE_IMPLEMENT_D_FUNC(vsEventCreationToolsInterface)

namespace // anonymous
{

//-----------------------------------------------------------------------------
QAction* createAction(QWidget* parent, const char* text, const char* icon = 0)
{
  QAction* action = new QAction(parent);
  action->setText(text);

  if (icon)
    {
    action->setIcon(qtUtil::standardActionIcon(icon));
    }

  return action;
}

} // namespace <anonymous>

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsEventCreationToolsInterfacePrivate

//-----------------------------------------------------------------------------
class vsEventCreationToolsInterfacePrivate
{
protected:
  QTE_DECLARE_PUBLIC_PTR(vsEventCreationToolsInterface)

private:
  QTE_DECLARE_PUBLIC(vsEventCreationToolsInterface)

public:
  explicit vsEventCreationToolsInterfacePrivate(
    vsEventCreationToolsInterface* q,
    vsMainWindow* window, vsScene* scene, vsCore* core);

  // Helper methods
  void cancelInteraction();

  void pauseVideoPlayback();
  void resumeVideoPlayback();

  void beginCreation(QAction* action, bool pause);
  void endCreation(QAction* action);

  vtkVgRegionWidget* createRegionWidget();

  QPolygonF viewToStab(QPolygonF) const;

  // Member variables
  vsCore* const Core;
  vsScene* const Scene;
  vsMainWindow* const Window;

  QToolButton* EventToolButton;
  QComboBox* EventTypeDropdown;
  QAction* ActionDraw;
  QAction* ActionBox;
  QAction* ActionQuick;
  QAction* ActionFullFrame;

  QMenu* CreateEventMenu;
  QActionGroup* TypeGroup;
  QSignalMapper* TypeMapper;
  QList<QAction*> EventTypeActions;

  QScopedPointer<vtkVgRegionWidget> RegionWidget;
  QSize LastBoxSize;

  bool SuppressCancelInteraction;
  bool IgnoreDrawingCanceled;
  bool VideoPausedByUs;
  bool IgnorePlaybackStatusChanged;
};

//-----------------------------------------------------------------------------
vsEventCreationToolsInterfacePrivate::vsEventCreationToolsInterfacePrivate(
  vsEventCreationToolsInterface* q, vsMainWindow* window,
  vsScene* scene, vsCore* core) :
  q_ptr(q),
  Core(core),
  Scene(scene),
  Window(window),
  LastBoxSize(30, 30),
  SuppressCancelInteraction(false),
  IgnoreDrawingCanceled(false),
  IgnorePlaybackStatusChanged(false)
{
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterfacePrivate::cancelInteraction()
{
  if (!this->SuppressCancelInteraction)
    {
    this->Scene->cancelInteraction();
    }
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterfacePrivate::pauseVideoPlayback()
{
  // Pause video, if not already paused
  this->VideoPausedByUs = false;
  const vgVideoPlayer::PlaybackMode ps = this->Scene->videoPlaybackStatus();
  if (ps == vgVideoPlayer::Playing ||
      ps == vgVideoPlayer::Buffering ||
      ps == vgVideoPlayer::Live)
    {
    this->IgnorePlaybackStatusChanged = true;
    this->Window->setVideoPlaybackPaused();
    this->VideoPausedByUs = true;
    }
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterfacePrivate::resumeVideoPlayback()
{
  if (this->VideoPausedByUs)
    {
    this->Window->setVideoPlaybackResumed();
    this->VideoPausedByUs = false;
    }
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterfacePrivate::beginCreation(
  QAction* action, bool pause)
{
  qtScopedValueChange<bool> ic(this->IgnoreDrawingCanceled, true);
  this->cancelInteraction();

  if (pause)
    {
    this->pauseVideoPlayback();
    }

  this->Window->pushViewCursor(Qt::CrossCursor, action);
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterfacePrivate::endCreation(QAction* action)
{
  this->cancelInteraction();
  this->Window->popViewCursor(action);
  this->resumeVideoPlayback(); // no-op if we didn't pause
  this->Scene->postUpdate();
}

//-----------------------------------------------------------------------------
vtkVgRegionWidget* vsEventCreationToolsInterfacePrivate::createRegionWidget()
{
  QTE_Q(vsEventCreationToolsInterface);

  vtkRenderWindowInteractor* const interactor =
    this->Window->view()->GetRenderWindow()->GetInteractor();
  vtkVgRegionWidget* rw = new vtkVgRegionWidget(interactor, q);

  q->connect(rw, SIGNAL(completed()), q, SLOT(createEventFromRegion()));
  q->connect(rw, SIGNAL(canceled()), q, SLOT(endEventCreation()));
  q->connect(rw, SIGNAL(beginningManipulation()), q, SLOT(popBoxCursor()));
  q->connect(rw, SIGNAL(statusMessageAvailable(QString)),
             this->Window, SLOT(setStatusText(QString)));

  rw->setStyle(Qt::red, 1.0);

  this->RegionWidget.reset(rw);
  return rw;
}

//-----------------------------------------------------------------------------
QPolygonF vsEventCreationToolsInterfacePrivate::viewToStab(QPolygonF r) const
{
  const QMatrix4x4 xf = this->Scene->currentTransform().inverted();
  for (int i = 0; i < r.count(); ++i)
    {
    r[i] = xf * r[i];
    }
  return r;
}

//END vsEventCreationToolsInterfacePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsEventCreationToolsInterface

//-----------------------------------------------------------------------------
vsEventCreationToolsInterface::vsEventCreationToolsInterface(
  vsMainWindow* window, vsScene* scene, vsCore* core) :
  QObject(window),
  d_ptr(new vsEventCreationToolsInterfacePrivate(this, window, scene, core))
{
  QTE_D(vsEventCreationToolsInterface);

  // Create tool actions
  d->ActionDraw = createAction(window, "&Draw Event", "event-draw");
  d->ActionDraw->setCheckable(true);
  connect(d->ActionDraw, SIGNAL(toggled(bool)),
          this, SLOT(toggleEventDrawing(bool)));
  d->ActionBox = createAction(window, "Create &Boxed Event",
                              "event-create-box");
  d->ActionBox->setCheckable(true);
  connect(d->ActionBox, SIGNAL(toggled(bool)),
          this, SLOT(toggleEventBoxing(bool)));
  d->ActionQuick = createAction(window, "&Create Event (&Quick)",
                                "event-create-quick");
  d->ActionQuick->setCheckable(true);
  connect(d->ActionQuick, SIGNAL(toggled(bool)),
          this, SLOT(toggleEventQuickCreation(bool)));
  d->ActionFullFrame = createAction(window, "Create &Full Frame Event");
  connect(d->ActionFullFrame, SIGNAL(triggered()),
          this, SLOT(createEventFullFrame()));

  // Add tool actions to menu
  d->CreateEventMenu = new QMenu("&Create Event", window);
  d->CreateEventMenu->addAction(d->ActionDraw);
  d->CreateEventMenu->addAction(d->ActionBox);
  d->CreateEventMenu->addAction(d->ActionQuick);
  d->CreateEventMenu->addAction(d->ActionFullFrame);
  d->CreateEventMenu->addSeparator();
  window->toolsMenu()->insertSeparator(300);
  window->toolsMenu()->insertMenu(d->CreateEventMenu, 300);
  window->toolsMenu()->insertSeparator(300);

  // Create helper objects for event type actions
  d->TypeGroup = new QActionGroup(d->CreateEventMenu);
  d->TypeMapper = new QSignalMapper(d->CreateEventMenu);

  // Create tools button
  d->EventToolButton = new QToolButton(window);
  QMenu* tools = new QMenu(d->EventToolButton);
  tools->addAction(d->ActionDraw);
  tools->addAction(d->ActionBox);
  tools->addAction(d->ActionQuick);
  tools->addAction(d->ActionFullFrame);
  d->EventToolButton->setMenu(tools);
  d->EventToolButton->setPopupMode(QToolButton::MenuButtonPopup);
  this->setActiveTool(d->ActionDraw);
  connect(tools, SIGNAL(triggered(QAction*)),
          this, SLOT(setActiveTool(QAction*)));

  // Add widgets to tool bar
  qtPrioritizedToolBarProxy* const toolBar = window->toolsToolBar();
  d->EventToolButton->setToolButtonStyle(toolBar->toolBar()->toolButtonStyle());
  toolBar->insertSeparator(300);
  d->EventTypeDropdown = new QComboBox(window);
  toolBar->insertWidget(d->EventTypeDropdown, 300);
  toolBar->insertWidget(d->EventToolButton, 300);
  toolBar->insertSeparator(300);

  connect(d->TypeMapper, SIGNAL(mapped(int)),
          d->EventTypeDropdown, SLOT(setCurrentIndex(int)));
  connect(d->EventTypeDropdown, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setCurrentEventType(int)));

  connect(scene, SIGNAL(interactionCanceled()),
          this, SLOT(endEventCreation()));
  connect(scene, SIGNAL(playbackStatusChanged(vgVideoPlayer::PlaybackMode,
                                              qreal)),
          this, SLOT(cancelVideoPlaybackModeRestoration()));

  connect(core, SIGNAL(manualEventTypesUpdated(QList<vsEventInfo>)),
          this, SLOT(repopulateEventTypes(QList<vsEventInfo>)));
  connect(core, SIGNAL(videoSourceStatusChanged(vsDataSource::Status)),
          this, SLOT(setVideoSourceStatus(vsDataSource::Status)));
  this->repopulateEventTypes(core->manualEventTypes());
  this->setVideoSourceStatus(core->videoSourceStatus());
}

//-----------------------------------------------------------------------------
vsEventCreationToolsInterface::~vsEventCreationToolsInterface()
{
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::setVideoSourceStatus(
  vsDataSource::Status status)
{
  QTE_D(vsEventCreationToolsInterface);

  bool eventCreationAllowed;
  switch (status)
    {
    case vsDataSource::StreamingActive:
    case vsDataSource::StreamingIdle:
    case vsDataSource::StreamingStopped:
    case vsDataSource::ArchivedActive:
    case vsDataSource::ArchivedSuspended:
    case vsDataSource::ArchivedIdle:
      eventCreationAllowed = true;
      break;
    default:
      eventCreationAllowed = false;
      break;
    }

  d->ActionDraw->setEnabled(eventCreationAllowed);
  d->ActionBox->setEnabled(eventCreationAllowed);
  d->ActionQuick->setEnabled(eventCreationAllowed);
  d->ActionFullFrame->setEnabled(eventCreationAllowed);
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::setActiveTool(QAction* tool)
{
  QTE_D(vsEventCreationToolsInterface);

  QAction* const oldTool = d->EventToolButton->defaultAction();
  if (oldTool)
    {
    // Remove shortcut from previous tool
    oldTool->setShortcut(QKeySequence());
    }

  // Set shortcut on new tool
  tool->setShortcut(QKeySequence("E"));
  tool->setShortcutContext(Qt::WindowShortcut);

  // Set new tool on button
  d->EventToolButton->setDefaultAction(tool);
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::setCurrentEventType(int index)
{
  QTE_D(vsEventCreationToolsInterface);
  d->EventTypeActions[index]->setChecked(true);
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::toggleEventDrawing(bool state)
{
  QTE_D(vsEventCreationToolsInterface);
  if (state)
    {
    // Begin event drawing
    qtScopedValueChange<bool> ic(d->IgnoreDrawingCanceled, true);
    d->beginCreation(d->ActionDraw, true);
    d->Scene->beginDrawing(vsContour::Annotation);
    connect(d->Scene, SIGNAL(drawingEnded()), this, SLOT(endEventCreation()));

    // When the contour is done, convert it to an event; use queued connection
    // so that vsCore has a chance to process the contour first
    connect(d->Scene, SIGNAL(contourCompleted(vsContour)),
            this, SLOT(convertContour(vsContour)), Qt::QueuedConnection);
    }
  else
    {
    d->endCreation(d->ActionDraw);
    disconnect(d->Scene, SIGNAL(contourCompleted(vsContour)), this, 0);
    disconnect(d->Scene, SIGNAL(drawingEnded()), this, 0);
    }
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::toggleEventBoxing(bool state)
{
  QTE_D(vsEventCreationToolsInterface);
  if (state)
    {
    d->beginCreation(d->ActionBox, true);
    d->createRegionWidget()->begin();
    }
  else
    {
    d->endCreation(d->ActionBox);
    d->RegionWidget.reset();
    }
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::toggleEventQuickCreation(bool state)
{
  QTE_D(vsEventCreationToolsInterface);
  if (state)
    {
    d->beginCreation(d->ActionBox, false);
    vtkVgRegionWidget* rw =  d->createRegionWidget();
    connect(rw, SIGNAL(dropDrawStarted()), this, SLOT(pauseVideoPlayback()));
    connect(rw, SIGNAL(dropDrawCanceled()), this, SLOT(resumeVideoPlayback()));
    rw->beginQuick(d->LastBoxSize);
    }
  else
    {
    d->endCreation(d->ActionBox);
    d->RegionWidget.reset();
    }
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::createEventFullFrame()
{
  QTE_D(vsEventCreationToolsInterface);

  const int i = d->EventTypeDropdown->currentIndex();
  if (i < 0)
    {
    // Should never happen...
    qWarning() << "Failed to convert region to event;"
               << " index of currently selected event type is not valid";
    return;
    }

  const int eventType = d->EventTypeDropdown->itemData(i).toInt();
  const vtkVgTimeStamp ts = d->Scene->currentVideoTime();
  QPoint topLeft(0,0);
  QPoint bottomRight(d->Scene->currentFrameMetaData().Width - 1.0,
                     d->Scene->currentFrameMetaData().Height - 1.0);
  const QPolygonF region = d->viewToStab(QRectF(topLeft, bottomRight));
  d->Core->createManualEvent(eventType, region, ts);
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::endEventCreation()
{
  QTE_D(vsEventCreationToolsInterface);

  if (!d->IgnoreDrawingCanceled)
    {
    qtScopedValueChange<bool> sci(d->SuppressCancelInteraction, true);

    d->ActionDraw->setChecked(false);
    d->ActionBox->setChecked(false);
    d->ActionQuick->setChecked(false);
    d->ActionFullFrame->setChecked(false);

    d->Window->setStatusText(QString());
    }
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::pauseVideoPlayback()
{
  QTE_D(vsEventCreationToolsInterface);
  d->pauseVideoPlayback();
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::resumeVideoPlayback()
{
  QTE_D(vsEventCreationToolsInterface);
  d->resumeVideoPlayback();
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::cancelVideoPlaybackModeRestoration()
{
  QTE_D(vsEventCreationToolsInterface);

  if (d->IgnorePlaybackStatusChanged)
    {
    d->IgnorePlaybackStatusChanged = false;
    return;
    }

  d->VideoPausedByUs = false;
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::popBoxCursor()
{
  QTE_D(vsEventCreationToolsInterface);
  d->Window->popViewCursor(d->ActionBox);
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::convertContour(vsContour c)
{
  QTE_D(vsEventCreationToolsInterface);

  const int i = d->EventTypeDropdown->currentIndex();
  if (i < 0)
    {
    // Should never happen...
    qWarning() << "Failed to convert region to event;"
               << " index of currently selected event type is not valid";
    return;
    }

  const int eventType = d->EventTypeDropdown->itemData(i).toInt();
  const vtkVgTimeStamp ts = d->Scene->currentVideoTime();
  d->Core->convertContourToEvent(c.id(), eventType, ts);
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::createEventFromRegion()
{
  QTE_D(vsEventCreationToolsInterface);

  const int i = d->EventTypeDropdown->currentIndex();
  if (i < 0)
    {
    // Should never happen...
    qWarning() << "Failed to convert region to event;"
               << " index of currently selected event type is not valid";
    return;
    }

  const int eventType = d->EventTypeDropdown->itemData(i).toInt();
  const vtkVgTimeStamp ts = d->Scene->currentVideoTime();
  QRect const box = d->RegionWidget->rect();
  d->LastBoxSize = d->RegionWidget->displaySize();
  QPolygonF const region = d->viewToStab(QRectF(box));
  d->Core->createManualEvent(eventType, region, ts);

  d->ActionBox->setChecked(false);
  d->ActionQuick->setChecked(false);
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsInterface::repopulateEventTypes(
  QList<vsEventInfo> eventTypes)
{
  QTE_D(vsEventCreationToolsInterface);

  int currentIndex = d->EventTypeDropdown->currentIndex();
  int currentType = 0;
  if (currentIndex >= 0)
    {
    // Get type value of currently selected event type
    currentType = d->EventTypeDropdown->itemData(currentIndex).toInt();
    }

  currentIndex = -1;
  qDeleteAll(d->EventTypeActions);
  d->EventTypeActions.clear();
  d->EventTypeDropdown->clear();

  foreach (const vsEventInfo& ei, eventTypes)
    {
    QAction* action = new QAction(ei.name, d->CreateEventMenu);
    action->setCheckable(true);
    d->CreateEventMenu->addAction(action);
    d->TypeGroup->addAction(action);
    d->TypeMapper->setMapping(action, d->EventTypeDropdown->count());
    connect(action, SIGNAL(triggered(bool)), d->TypeMapper, SLOT(map()));
    d->EventTypeActions.append(action);

    d->EventTypeDropdown->addItem(ei.name, ei.type);
    if (ei.type == currentType)
      {
      // If the type we just added is the previously selected type, remember
      // its index so we can reselect it later
      currentIndex = d->EventTypeDropdown->count() - 1;
      }
    }

  // Reselect the previously selected type, or the first available type if
  // there was no previous type or the previous type is no longer available
  d->EventTypeDropdown->setCurrentIndex(qMax(currentIndex, 0));
}

//END vsEventCreationToolsInterface
