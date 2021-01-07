// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsNoteTreeWidget.h"
#include "ui_noteTree.h"
#include "am_noteTree.h"

#include "vsScene.h"

#include <vgfContextMenu.h>
#include <vgfNoteTreeModel.h>

#include <qtMap.h>

#include <QContextMenuEvent>

//-----------------------------------------------------------------------------
class vsNoteTreeWidgetPrivate
{
public:
  Ui::vsNoteTreeWidget UI;
  Am::vsNoteTreeWidget AM;

  vgfNoteTreeModel* ViewModel;
  vgfContextMenu ContextMenu;
};

QTE_IMPLEMENT_D_FUNC(vsNoteTreeWidget)

//-----------------------------------------------------------------------------
vsNoteTreeWidget::vsNoteTreeWidget(QWidget* parent) :
  QWidget(parent), d_ptr(new vsNoteTreeWidgetPrivate)
{
  QTE_D(vsNoteTreeWidget);

  d->UI.setupUi(this);
  d->AM.setupActions(d->UI, this);

  d->ViewModel = new vgfNoteTreeModel;
  d->UI.tree->setModel(d->ViewModel);

  d->UI.tree->setColumnWidth(vgfNoteTreeModel::EntityTypeColumn, 20);
  d->UI.tree->setColumnWidth(vgfNoteTreeModel::EntityIconColumn, 20);

  d->UI.tree->setSortingEnabled(true);
  d->UI.tree->sortByColumn(vgfNoteTreeModel::StartTimeColumn,
                           Qt::AscendingOrder);

  d->ViewModel->setInactiveItemsShown(d->UI.actionShowInactive->isChecked());

  connect(d->UI.actionShowInactive, SIGNAL(toggled(bool)),
          d->ViewModel, SLOT(setInactiveItemsShown(bool)));
  connect(d->UI.tree->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(updateSelection()));
  connect(d->UI.tree, SIGNAL(activated(QModelIndex)),
          this, SLOT(activate(QModelIndex)));

  d->ContextMenu.setModel(d->ViewModel);
  qtUtil::mapBound(d->ContextMenu.actions(), this, &QWidget::addAction);
}

//-----------------------------------------------------------------------------
vsNoteTreeWidget::~vsNoteTreeWidget()
{
}

//-----------------------------------------------------------------------------
void vsNoteTreeWidget::connectToScene(vsScene* scene)
{
  QTE_D(vsNoteTreeWidget);

  connect(scene, SIGNAL(currentTimeChanged(vgTimeStamp)),
          d->ViewModel, SLOT(setCurrentTime(vgTimeStamp)));
  connect(&d->ContextMenu,
          SIGNAL(jumpRequested(vgfItemReference, vgf::JumpFlags)),
          scene, SLOT(jumpToItem(vgfItemReference, vgf::JumpFlags)));
}

//-----------------------------------------------------------------------------
void vsNoteTreeWidget::setModel(QAbstractItemModel* model)
{
  QTE_D(vsNoteTreeWidget);
  d->ViewModel->setSourceModel(model);
}

//-----------------------------------------------------------------------------
void vsNoteTreeWidget::updateSelection()
{
  QTE_D(vsNoteTreeWidget);
  d->ContextMenu.setActiveItems(
    d->UI.tree->selectionModel()->selectedIndexes());
}

//-----------------------------------------------------------------------------
void vsNoteTreeWidget::activate(const QModelIndex& index)
{
  QTE_D(vsNoteTreeWidget);
  d->ContextMenu.activate(index, d->ViewModel->roleForColumn(index.column()));
}

//-----------------------------------------------------------------------------
void vsNoteTreeWidget::contextMenuEvent(QContextMenuEvent* e)
{
  QTE_D(vsNoteTreeWidget);

  if (d->UI.tree->isAncestorOf(this->childAt(e->pos())))
    {
    d->ContextMenu.exec(e->globalPos());
    return;
    }

  QWidget::contextMenuEvent(e);
}
