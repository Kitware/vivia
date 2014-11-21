/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackTreeWidget.h"

#include <QContextMenuEvent>
#include <QMenu>

#include "vsTrackTreeModel.h"

//-----------------------------------------------------------------------------
vsTrackTreeWidget::vsTrackTreeWidget(QWidget* parent) : QWidget(parent)
{
  this->UI.setupUi(this);
  this->AM.setupActions(this->UI, this);

  // Add all actions as actions of this widget, so shortcuts will work
  foreach (QObject* object, this->children())
    {
    QAction* action = qobject_cast<QAction*>(object);
    if (action)
      this->addAction(action);
    }

  connect(this->UI.actionHideAll, SIGNAL(triggered()),
          this->UI.tree, SLOT(hideAllItems()));
  connect(this->UI.actionShowAll, SIGNAL(triggered()),
          this->UI.tree, SLOT(showAllItems()));
  connect(this->UI.actionHideSelected, SIGNAL(triggered()),
          this->UI.tree, SLOT(hideSelectedItems()));
  connect(this->UI.actionShowSelected, SIGNAL(triggered()),
          this->UI.tree, SLOT(showSelectedItems()));

  // Connect signal and slot and sync GUI state with underlying model state
  connect(this->UI.actionShowHiddenItems, SIGNAL(toggled(bool)),
          this->UI.tree, SLOT(setHiddenItemsShown(bool)));

  this->UI.tree->setHiddenItemsShown(
    this->UI.actionShowHiddenItems->isChecked());

  connect(this->UI.actionJumpToStart, SIGNAL(triggered()),
          this->UI.tree, SLOT(jumpToSelectedStart()));
  connect(this->UI.actionJumpToEnd, SIGNAL(triggered()),
          this->UI.tree, SLOT(jumpToSelectedEnd()));
  connect(this->UI.actionFollowTrack, SIGNAL(triggered()),
          this->UI.tree, SLOT(followSelectedTrack()));

  connect(this->UI.tree, SIGNAL(selectionChanged(QList<vtkIdType>)),
          this, SLOT(setActionsEnabled()));

  connect(this->UI.tree, SIGNAL(selectionChanged(QList<vtkIdType>)),
          this, SIGNAL(selectionChanged(QList<vtkIdType>)));

  connect(this->UI.tree, SIGNAL(jumpToTrack(vtkIdType, bool)),
          this, SIGNAL(jumpToTrack(vtkIdType, bool)));

  connect(this->UI.tree, SIGNAL(trackFollowingRequested(vtkIdType)),
          this, SIGNAL(trackFollowingRequested(vtkIdType)));

  connect(this->UI.actionStopFollowingTrack, SIGNAL(triggered()),
          this, SIGNAL(trackFollowingCanceled()));

  this->contextMenu = new QMenu(this);
  this->contextMenu->addAction(this->UI.actionShowSelected);
  this->contextMenu->addAction(this->UI.actionHideSelected);
  this->contextMenu->addAction(this->UI.actionJumpToStart);
  this->contextMenu->addAction(this->UI.actionJumpToEnd);
  this->contextMenu->addAction(this->UI.actionFollowTrack);
  this->contextMenu->addAction(this->UI.actionStopFollowingTrack);

  this->setActionsEnabled();
}

//-----------------------------------------------------------------------------
vsTrackTreeWidget::~vsTrackTreeWidget()
{
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::contextMenuEvent(QContextMenuEvent* event)
{
  if (this->UI.tree->isAncestorOf(this->childAt(event->pos())))
    {
    this->contextMenu->exec(event->globalPos());
    return;
    }

  QWidget::contextMenuEvent(event);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::setModel(vsTrackTreeModel* model)
{
  this->UI.tree->setModel(model);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::selectTrack(vtkIdType trackId)
{
  this->UI.tree->selectTrack(trackId);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::setActionsEnabled()
{
  int count = this->UI.tree->selectedTrackCount();
  this->UI.actionHideSelected->setEnabled(count > 0);
  this->UI.actionShowSelected->setEnabled(count > 0);
  this->UI.actionJumpToStart->setEnabled(count == 1);
  this->UI.actionJumpToEnd->setEnabled(count == 1);
  this->UI.actionFollowTrack->setEnabled(count == 1);
  this->UI.actionStopFollowingTrack->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::setHiddenItemsShown(bool enable)
{
  this->UI.actionShowHiddenItems->setChecked(enable);
}
