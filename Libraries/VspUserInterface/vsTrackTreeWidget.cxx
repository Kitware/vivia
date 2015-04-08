/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackTreeWidget.h"
#include "ui_trackTree.h"
#include "am_trackTree.h"

#include <QContextMenuEvent>
#include <QMenu>

#include <qtScopedValueChange.h>

#include <vgTextEditDialog.h>

#include "vsTrackTreeModel.h"
#include "vsTrackTreeSelectionModel.h"

//-----------------------------------------------------------------------------
class vsTrackTreeWidgetPrivate
{
public:
  Ui::vsTrackTreeWidget UI;
  Am::vsTrackTreeWidget AM;

  QMenu* ContextMenu;
};

QTE_IMPLEMENT_D_FUNC(vsTrackTreeWidget)

//-----------------------------------------------------------------------------
vsTrackTreeWidget::vsTrackTreeWidget(QWidget* parent) :
  QWidget(parent), d_ptr(new vsTrackTreeWidgetPrivate)
{
  QTE_D(vsTrackTreeWidget);

  d->UI.setupUi(this);
  d->AM.setupActions(d->UI, this);

  // Add all actions as actions of this widget, so shortcuts will work
  foreach (QObject* object, this->children())
    {
    QAction* action = qobject_cast<QAction*>(object);
    if (action)
      this->addAction(action);
    }

  connect(d->UI.actionHideAll, SIGNAL(triggered()),
          d->UI.tree, SLOT(hideAllItems()));
  connect(d->UI.actionShowAll, SIGNAL(triggered()),
          d->UI.tree, SLOT(showAllItems()));
  connect(d->UI.actionHideSelected, SIGNAL(triggered()),
          d->UI.tree, SLOT(hideSelectedItems()));
  connect(d->UI.actionShowSelected, SIGNAL(triggered()),
          d->UI.tree, SLOT(showSelectedItems()));
  connect(d->UI.actionAddStar, SIGNAL(triggered()),
          this, SLOT(addStar()));
  connect(d->UI.actionRemoveStar, SIGNAL(triggered()),
          this, SLOT(removeStar()));
  connect(d->UI.actionEditNote, SIGNAL(triggered()),
          this, SLOT(editNote()));

  // Connect signal and slot and sync GUI state with underlying model state
  connect(d->UI.actionShowHiddenItems, SIGNAL(toggled(bool)),
          d->UI.tree, SLOT(setHiddenItemsShown(bool)));

  d->UI.tree->setHiddenItemsShown(
    d->UI.actionShowHiddenItems->isChecked());

  connect(d->UI.actionJumpToStart, SIGNAL(triggered()),
          d->UI.tree, SLOT(jumpToSelectedStart()));
  connect(d->UI.actionJumpToEnd, SIGNAL(triggered()),
          d->UI.tree, SLOT(jumpToSelectedEnd()));
  connect(d->UI.actionFollowTrack, SIGNAL(triggered()),
          d->UI.tree, SLOT(followSelectedTrack()));

  connect(d->UI.tree, SIGNAL(jumpToTrack(vtkIdType, bool)),
          this, SIGNAL(jumpToTrack(vtkIdType, bool)));

  connect(d->UI.tree, SIGNAL(trackFollowingRequested(vtkIdType)),
          this, SIGNAL(trackFollowingRequested(vtkIdType)));

  connect(d->UI.actionCancelFollowing, SIGNAL(triggered()),
          this, SIGNAL(trackFollowingCanceled()));

  d->ContextMenu = new QMenu(this);
  d->ContextMenu->addAction(d->UI.actionShowSelected);
  d->ContextMenu->addAction(d->UI.actionHideSelected);
  d->ContextMenu->addAction(d->UI.actionJumpToStart);
  d->ContextMenu->addAction(d->UI.actionJumpToEnd);
  d->ContextMenu->addAction(d->UI.actionFollowTrack);
  d->ContextMenu->addAction(d->UI.actionCancelFollowing);
  d->ContextMenu->addSeparator();
  d->ContextMenu->addAction(d->UI.actionAddStar);
  d->ContextMenu->addAction(d->UI.actionRemoveStar);
  d->ContextMenu->addSeparator();
  d->ContextMenu->addAction(d->UI.actionEditNote);

  qtScopedBlockSignals bs(this);
  this->updateSelectionStatus(QSet<vtkIdType>());
}

//-----------------------------------------------------------------------------
vsTrackTreeWidget::~vsTrackTreeWidget()
{
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::contextMenuEvent(QContextMenuEvent* event)
{
  QTE_D(vsTrackTreeWidget);

  if (d->UI.tree->isAncestorOf(this->childAt(event->pos())))
    {
    d->ContextMenu->exec(event->globalPos());
    return;
    }

  QWidget::contextMenuEvent(event);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::setModel(vsTrackTreeModel* model)
{
  QTE_D(vsTrackTreeWidget);
  d->UI.tree->setModel(model);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::setSelectionModel(vsTrackTreeSelectionModel* model)
{
  QTE_D(vsTrackTreeWidget);

  d->UI.tree->setSelectionModel(model);

  connect(model, SIGNAL(selectionChanged(QSet<vtkIdType>)),
          this, SLOT(updateSelectionStatus(QSet<vtkIdType>)));
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::selectTrack(vtkIdType trackId)
{
  QTE_D(vsTrackTreeWidget);
  d->UI.tree->selectTrack(trackId);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::updateSelectionStatus(QSet<vtkIdType> selection)
{
  QTE_D(vsTrackTreeWidget);

  const int count = selection.count();

  d->UI.actionHideSelected->setEnabled(count > 0);
  d->UI.actionShowSelected->setEnabled(count > 0);
  d->UI.actionAddStar->setEnabled(count > 0);
  d->UI.actionRemoveStar->setEnabled(count > 0);
  d->UI.actionJumpToStart->setEnabled(count == 1);
  d->UI.actionJumpToEnd->setEnabled(count == 1);
  d->UI.actionFollowTrack->setEnabled(count == 1);
  d->UI.actionEditNote->setEnabled(count == 1);
  d->UI.actionCancelFollowing->setEnabled(true);

  emit this->selectionChanged(selection);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::setHiddenItemsShown(bool enable)
{
  QTE_D(vsTrackTreeWidget);
  d->UI.actionShowHiddenItems->setChecked(enable);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::addStar()
{
  QTE_D(vsTrackTreeWidget);
  d->UI.tree->setSelectedItemsStarred(true);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::removeStar()
{
  QTE_D(vsTrackTreeWidget);
  d->UI.tree->setSelectedItemsStarred(false);
}

//-----------------------------------------------------------------------------
void vsTrackTreeWidget::editNote()
{
  QTE_D(vsTrackTreeWidget);

  const QModelIndex i = d->UI.tree->selectionModel()->selectedRows().first();
  const QModelIndex n =
    d->UI.tree->model()->index(i.row(), vsTrackTreeModel::NoteColumn);
  QString note = d->UI.tree->model()->data(n).toString();

  bool ok = false;
  note = vgTextEditDialog::getText(this, "Edit Track Note", note, &ok);
  if (ok)
    {
    d->UI.tree->model()->setData(n, note, Qt::DisplayRole);
    }
}
