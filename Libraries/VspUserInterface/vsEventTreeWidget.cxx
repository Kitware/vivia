// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsEventTreeWidget.h"
#include "ui_eventTree.h"
#include "am_eventTree.h"

#include <QContextMenuEvent>
#include <QMenu>

#include <qtMap.h>
#include <qtScopedValueChange.h>

#include <vgTextEditDialog.h>

#include "vsEventRatingMenu.h"
#include "vsEventTreeModel.h"
#include "vsEventTreeSelectionModel.h"

#define connectEventTreeButtonToggled(_a_, _s_) do { \
  connect(_a_, SIGNAL(toggled(bool)), tree, SLOT(_s_(bool))); \
  tree->_s_(_a_->isChecked()); \
  } while (0)

//-----------------------------------------------------------------------------
class vsEventTreeWidgetPrivate
{
public:
  Ui::vsEventTreeWidget UI;
  Am::vsEventTreeWidget AM;

  QMenu* contextMenu;
  QMenu* ratingMenu;

  vsEventTreeSelectionModel* selectionModel;
};

QTE_IMPLEMENT_D_FUNC(vsEventTreeWidget)

//-----------------------------------------------------------------------------
vsEventTreeWidget::vsEventTreeWidget(QWidget* parent) :
  QWidget(parent), d_ptr(new vsEventTreeWidgetPrivate)
{
  QTE_D(vsEventTreeWidget);

  d->UI.setupUi(this);
  d->AM.setupActions(d->UI, this);

  vsEventTreeView* const tree = d->UI.tree;
  d->selectionModel = 0;

  // Add all actions as actions of this widget, so shortcuts will work
  foreach (QObject* object, this->children())
    {
    QAction* action = qobject_cast<QAction*>(object);
    if (action)
      this->addAction(action);
    }

  connect(d->UI.actionHideAll, SIGNAL(triggered()),
          tree, SLOT(hideAllItems()));
  connect(d->UI.actionShowAll, SIGNAL(triggered()),
          tree, SLOT(showAllItems()));
  connect(d->UI.actionHideSelected, SIGNAL(triggered()),
          tree, SLOT(hideSelectedItems()));
  connect(d->UI.actionShowSelected, SIGNAL(triggered()),
          tree, SLOT(showSelectedItems()));
  connect(d->UI.actionSetEventStart, SIGNAL(triggered(bool)),
          this, SLOT(setSelectedEventsStart()));
  connect(d->UI.actionSetEventEnd, SIGNAL(triggered(bool)),
          this, SLOT(setSelectedEventsEnd()));
  connect(d->UI.actionAddStar, SIGNAL(triggered()),
          this, SLOT(addStar()));
  connect(d->UI.actionRemoveStar, SIGNAL(triggered()),
          this, SLOT(removeStar()));
  connect(d->UI.actionEditNote, SIGNAL(triggered()),
          this, SLOT(editNote()));

  // Connect signal and slot and sync GUI state with underlying model state
  connectEventTreeButtonToggled(d->UI.actionShowHiddenItems,
                                setHiddenItemsShown);

  connect(d->UI.actionJumpToStart, SIGNAL(triggered()),
          tree, SLOT(jumpToSelectedStart()));
  connect(d->UI.actionJumpToEnd, SIGNAL(triggered()),
          tree, SLOT(jumpToSelectedEnd()));

  connect(tree, SIGNAL(jumpToEvent(vtkIdType, bool)),
          this, SIGNAL(jumpToEvent(vtkIdType, bool)));

  d->contextMenu = new QMenu(this);
  d->contextMenu->addAction(d->UI.actionShowSelected);
  d->contextMenu->addAction(d->UI.actionHideSelected);
  d->contextMenu->addAction(d->UI.actionJumpToStart);
  d->contextMenu->addAction(d->UI.actionJumpToEnd);
  d->contextMenu->addSeparator();
  d->contextMenu->addAction(d->UI.actionSetEventStart);
  d->contextMenu->addAction(d->UI.actionSetEventEnd);
  d->contextMenu->addSeparator();
  d->ratingMenu = d->contextMenu->addMenu("&Rate As");

  vsEventRatingMenu::buildMenu(d->ratingMenu);

  QAction* unrated =
    vsEventRatingMenu::getAction(d->ratingMenu, vsEventRatingMenu::Unrated);
  QAction* relevant =
    vsEventRatingMenu::getAction(d->ratingMenu, vsEventRatingMenu::Relevant);
  QAction* excluded =
    vsEventRatingMenu::getAction(d->ratingMenu, vsEventRatingMenu::Excluded);
  QAction* rejected =
    vsEventRatingMenu::getAction(d->ratingMenu, vsEventRatingMenu::Rejected);

  unrated->setShortcut(Qt::Key_Slash);
  relevant->setShortcut(Qt::Key_Plus);
  excluded->setShortcut(Qt::Key_Minus);
  rejected->setShortcut(Qt::Key_Delete);

  unrated->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  relevant->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  excluded->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  rejected->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  this->addActions(d->ratingMenu->actions());

  connect(unrated, SIGNAL(triggered()),
          this, SLOT(setSelectedEventsToUnratedVerified()));

  connect(relevant, SIGNAL(triggered()),
          this, SLOT(setSelectedEventsToRelevantVerified()));

  connect(excluded, SIGNAL(triggered()),
          this, SLOT(setSelectedEventsToNotRelevantVerified()));

  connect(rejected, SIGNAL(triggered()),
          this, SLOT(setSelectedEventsToNotRelevantRejected()));

  connect(this, SIGNAL(selectionStatusChanged(bool)),
          unrated, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(selectionStatusChanged(bool)),
          relevant, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(selectionStatusChanged(bool)),
          excluded, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(selectionStatusChanged(bool)),
          rejected, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(selectionStatusChanged(bool)),
          d->ratingMenu, SLOT(setEnabled(bool)));
  unrated->setEnabled(false);
  relevant->setEnabled(false);
  excluded->setEnabled(false);
  rejected->setEnabled(false);
  d->ratingMenu->setEnabled(false);

  d->contextMenu->addSeparator();
  d->contextMenu->addAction(d->UI.actionAddStar);
  d->contextMenu->addAction(d->UI.actionRemoveStar);
  d->contextMenu->addSeparator();
  d->contextMenu->addAction(d->UI.actionEditNote);

  qtScopedBlockSignals bs(this);
  this->updateSelectionStatus(QSet<vtkIdType>());
}

//-----------------------------------------------------------------------------
vsEventTreeWidget::~vsEventTreeWidget()
{
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::contextMenuEvent(QContextMenuEvent* event)
{
  QTE_D(vsEventTreeWidget);

  if (d->UI.tree->isAncestorOf(this->childAt(event->pos())))
    {
    d->contextMenu->exec(event->globalPos());
    return;
    }

  QWidget::contextMenuEvent(event);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setStatusFilter(vsEventStatus s)
{
  QTE_D(vsEventTreeWidget);
  d->UI.tree->setStatusFilter(s);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setModel(vsEventTreeModel* model)
{
  QTE_D(vsEventTreeWidget);
  d->UI.tree->setModel(model);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setSelectionModel(vsEventTreeSelectionModel* model)
{
  QTE_D(vsEventTreeWidget);

  d->selectionModel = model;
  d->UI.tree->setSelectionModel(model);

  connect(model, SIGNAL(selectionChanged(QSet<vtkIdType>)),
          this, SLOT(updateSelectionStatus(QSet<vtkIdType>)));
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::selectEvent(vtkIdType eventId)
{
  QTE_D(vsEventTreeWidget);
  d->selectionModel->selectEvent(eventId);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::updateSelectionStatus(QSet<vtkIdType> selection)
{
  QTE_D(vsEventTreeWidget);

  const int count = selection.count();
  d->UI.actionJumpToStart->setEnabled(count == 1);
  d->UI.actionJumpToEnd->setEnabled(count == 1);
  d->UI.actionEditNote->setEnabled(count == 1);

  // Get count of modifiable events
  int modifiableCount = 0;

  const vsEventTreeSelectionModel* esm =
    d->UI.tree->underlyingSelectionModel();
  if (esm && esm->eventTreeModel())
    {
    const vsEventTreeModel* model = esm->eventTreeModel();
    foreach (const QModelIndex& i, esm->selectedRows())
      {
      const bool modifiable =
        model->data(i, vsEventTreeModel::ModifiableRole).toBool();
      modifiableCount += (modifiable ? 1 : 0);
      }
    }

  d->UI.actionSetEventStart->setEnabled(modifiableCount > 0 &&
                                        modifiableCount == count);
  d->UI.actionSetEventEnd->setEnabled(modifiableCount > 0 &&
                                      modifiableCount == count);

  emit this->selectionChanged(selection);
  emit this->selectionStatusChanged(count > 0);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setSelectedEventsToUnratedVerified()
{
  QTE_D(vsEventTreeWidget);
  d->UI.tree->setSelectedItemsRating(vgObjectStatus::None);
  d->UI.tree->setSelectedItemsStatus(vs::VerifiedEvent);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setSelectedEventsToRelevantVerified()
{
  QTE_D(vsEventTreeWidget);
  d->UI.tree->setSelectedItemsRating(vgObjectStatus::Adjudicated);
  d->UI.tree->setSelectedItemsStatus(vs::VerifiedEvent);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setSelectedEventsToNotRelevantVerified()
{
  QTE_D(vsEventTreeWidget);
  d->UI.tree->setSelectedItemsRating(vgObjectStatus::Excluded);
  d->UI.tree->setSelectedItemsStatus(vs::VerifiedEvent);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setSelectedEventsToNotRelevantRejected()
{
  QTE_D(vsEventTreeWidget);
  d->UI.tree->setSelectedItemsRating(vgObjectStatus::Excluded);
  d->UI.tree->setSelectedItemsStatus(vs::RejectedEvent);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setHiddenItemsShown(bool enable)
{
  QTE_D(vsEventTreeWidget);
  d->UI.actionShowHiddenItems->setChecked(enable);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::addStar()
{
  QTE_D(vsEventTreeWidget);
  d->UI.tree->setSelectedItemsStarred(true);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::removeStar()
{
  QTE_D(vsEventTreeWidget);
  d->UI.tree->setSelectedItemsStarred(false);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::editNote()
{
  QTE_D(vsEventTreeWidget);

  const QModelIndex i = d->UI.tree->selectionModel()->selectedRows().first();
  const QModelIndex n =
    d->UI.tree->model()->index(i.row(), vsEventTreeModel::NoteColumn);
  QString note = d->UI.tree->model()->data(n).toString();

  bool ok = false;
  note = vgTextEditDialog::getText(this, "Edit Event Note", note, &ok);
  if (ok)
    {
    d->UI.tree->model()->setData(n, note, Qt::DisplayRole);
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setSelectedEventsStart()
{
  QTE_D(vsEventTreeWidget);
  qtUtil::mapBound(d->selectionModel->selectedEvents(), this,
                   &vsEventTreeWidget::setEventStartRequested);
}

//-----------------------------------------------------------------------------
void vsEventTreeWidget::setSelectedEventsEnd()
{
  QTE_D(vsEventTreeWidget);
  qtUtil::mapBound(d->selectionModel->selectedEvents(), this,
                   &vsEventTreeWidget::setEventEndRequested);
}
