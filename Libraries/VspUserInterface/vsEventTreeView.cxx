// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsEventTreeView.h"

#include "vsEventTreeModel.h"
#include "vsEventTreeSelectionModel.h"

#include <qtGlobal.h>

#include <QDropEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QSortFilterProxyModel>

namespace // anonymous
{

//-----------------------------------------------------------------------------
class vsSortFilterProxyModel : public QSortFilterProxyModel
{
protected:
  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const
    {
    switch (left.column())
      {
      case vsEventTreeModel::RatingColumn:
        {
        QVariant l = left.model()->data(left, vsEventTreeModel::RatingRole);
        QVariant r = right.model()->data(right, vsEventTreeModel::RatingRole);
        return l.toInt() < r.toInt();
        }
      case vsEventTreeModel::NameColumn:
        {
        QVariant l = left.model()->data(left, vsEventTreeModel::NameSortRole);
        QVariant r = right.model()->data(right, vsEventTreeModel::NameSortRole);
        return l.toInt() < r.toInt();
        }
      case vsEventTreeModel::StarColumn:
        {
        QVariant l = left.model()->data(left, vsEventTreeModel::StarRole);
        QVariant r = right.model()->data(right, vsEventTreeModel::StarRole);
        return l.toBool() < r.toBool();
        }
      }

    return QSortFilterProxyModel::lessThan(left, right);
    }
};

//-----------------------------------------------------------------------------
class vsProxySelectionModel : public QItemSelectionModel
{
public:
  vsProxySelectionModel(QAbstractProxyModel* proxy,
                        QItemSelectionModel* model) :
    QItemSelectionModel(proxy),
    proxyItemModel(proxy),
    underlyingSelectionModel(model)
    {}

  void select(const QItemSelection& selection)
    {
    const QItemSelection mappedSelection =
      this->proxyItemModel->mapSelectionFromSource(selection);
    this->QItemSelectionModel::select(mappedSelection, Select);
    }

  void deselect(const QItemSelection& selection)
    {
    const QItemSelection mappedSelection =
      this->proxyItemModel->mapSelectionFromSource(selection);
    this->QItemSelectionModel::select(mappedSelection, Deselect);
    }

protected:
  virtual void select(
    const QModelIndex& index, SelectionFlags command) QTE_OVERRIDE
    {
    const QModelIndex mappedIndex = this->proxyItemModel->mapToSource(index);
    this->underlyingSelectionModel->select(mappedIndex, command);
    }

  virtual void select(
    const QItemSelection& selection, SelectionFlags command) QTE_OVERRIDE
    {
    const QItemSelection mappedSelection =
      this->proxyItemModel->mapSelectionToSource(selection);
    this->underlyingSelectionModel->select(mappedSelection, command);
    }

  QAbstractProxyModel* const proxyItemModel;
  QItemSelectionModel* const underlyingSelectionModel;
};

//-----------------------------------------------------------------------------
class DisableSort
{
  QSortFilterProxyModel* Model;

public:
  DisableSort(QSortFilterProxyModel* model) : Model(model)
    {
    this->Model->setDynamicSortFilter(false);
    }

  ~DisableSort()
    {
    this->Model->setDynamicSortFilter(false);
    }
};

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vsEventTreeView::vsEventTreeView(QWidget* p) :
  QTreeView(p),
  eventTreeModel(0),
  eventTreeSelectionModel(0),
  showHiddenItems(true),
  viewMode(vs::UnverifiedEvent)
{
  connect(this, SIGNAL(activated(const QModelIndex&)),
          this, SLOT(itemActivated(const QModelIndex&)));
}

//-----------------------------------------------------------------------------
vsEventTreeView::~vsEventTreeView()
{
}

//-----------------------------------------------------------------------------
void vsEventTreeView::setModel(QAbstractItemModel* m)
{
  vsEventTreeModel* em = qobject_cast<vsEventTreeModel*>(m);
  if (!em)
    return;

  // NOTE: Caller is responsible for deleting any prior model
  this->eventTreeModel = em;

  this->proxyModel.reset(new vsSortFilterProxyModel);
  this->proxyModel->setSourceModel(this->eventTreeModel);
  this->proxyModel->setDynamicSortFilter(false);

  QAbstractItemModel* old = this->model();
  this->QTreeView::setModel(this->proxyModel.data());
  delete old;

  this->setColumnHidden(vsEventTreeModel::IdColumn, true);
  this->setColumnWidth(vsEventTreeModel::RatingColumn, 20);
  this->setColumnWidth(vsEventTreeModel::StarColumn,
                       this->header()->sectionSizeHint(
                         vsEventTreeModel::StarColumn));
  this->setColumnWidth(vsEventTreeModel::NameColumn, 50);
  this->setColumnWidth(vsEventTreeModel::StartTimeColumn, 80);
  this->setColumnWidth(vsEventTreeModel::EndTimeColumn, 80);
  this->sortByColumn(vsEventTreeModel::NameColumn, Qt::AscendingOrder);
  this->setSortingEnabled(true);

  connect(this->proxyModel.data(),
          SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this,
          SLOT(updateRows(const QModelIndex&, int, int)));

  connect(this->proxyModel.data(),
          SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
          this,
          SLOT(updateRows(const QModelIndex&, const QModelIndex&)));
}

//-----------------------------------------------------------------------------
void vsEventTreeView::setSelectionModel(QItemSelectionModel* m)
{
  vsEventTreeSelectionModel* const esm =
    qobject_cast<vsEventTreeSelectionModel*>(m);

  if (!esm)
    {
    return;
    }

  this->eventTreeSelectionModel = esm;

  vsProxySelectionModel* const psm =
    new vsProxySelectionModel(this->proxyModel.data(), esm);
  this->proxySelectionModel.reset(psm);
  this->QTreeView::setSelectionModel(psm);

  connect(esm, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(updateSelection(QItemSelection, QItemSelection)));
  connect(esm, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
          this, SLOT(setCurrentIndex(QModelIndex)));
}

//-----------------------------------------------------------------------------
void vsEventTreeView::itemActivated(const QModelIndex& index)
{
  bool toEnd = (index.column() == vsEventTreeModel::EndTimeColumn);
  const auto& proxyIndex = this->proxyModel->mapToSource(index);
  emit this->jumpToEvent(this->eventIdFromIndex(proxyIndex), toEnd);
}

//-----------------------------------------------------------------------------
void vsEventTreeView::hideSelectedItems()
{
  DisableSort ds(this->proxyModel.data());

  const QModelIndexList& selected =
    this->eventTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  foreach (QModelIndex i, selected)
    {
    this->eventTreeModel->setData(
      i, false, vsEventTreeModel::DisplayStateRole);
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::showSelectedItems()
{
  DisableSort ds(this->proxyModel.data());

  const QModelIndexList& selected =
    this->eventTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  foreach (QModelIndex i, selected)
    {
    this->eventTreeModel->setData(
      i, true, vsEventTreeModel::DisplayStateRole);
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::jumpToSelectedStart()
{
  const QModelIndexList& selection =
    this->eventTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  Q_ASSERT(!selection.isEmpty());
  emit this->jumpToEvent(this->eventIdFromIndex(selection.front()), false);
}

//-----------------------------------------------------------------------------
void vsEventTreeView::jumpToSelectedEnd()
{
  const QModelIndexList& selection =
    this->eventTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  Q_ASSERT(!selection.isEmpty());
  emit this->jumpToEvent(this->eventIdFromIndex(selection.front()), true);
}

//-----------------------------------------------------------------------------
void vsEventTreeView::setSelectedItemsRating(
  vgObjectStatus::enumObjectStatus rating)
{
  DisableSort ds(this->proxyModel.data());

  const QModelIndexList& selected =
    this->eventTreeSelectionModel->selectedRows(
      this, this->proxyModel.data(), vsEventTreeModel::RatingColumn);

  foreach (QModelIndex i, selected)
    {
    this->eventTreeModel->setData(
      i, static_cast<int>(rating), vsEventTreeModel::RatingRole);
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::setSelectedItemsStatus(vsEventStatus status)
{
  DisableSort ds(this->proxyModel.data());

  const QModelIndexList& selected =
    this->eventTreeSelectionModel->selectedRows(
      this, this->proxyModel.data(), vsEventTreeModel::RatingColumn);

  foreach (QModelIndex i, selected)
    {
    this->eventTreeModel->setData(
      i, static_cast<int>(status), vsEventTreeModel::StatusRole);
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::setSelectedItemsStarred(bool starred)
{
  DisableSort ds(this->proxyModel.data());

  const QModelIndexList& selected =
    this->eventTreeSelectionModel->selectedRows(
      this, this->proxyModel.data(), vsEventTreeModel::StarColumn);

  foreach (QModelIndex i, selected)
    {
    this->eventTreeModel->setData(i, starred, vsEventTreeModel::StarRole);
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::hideAllItems()
{
  DisableSort ds(this->proxyModel.data());

  for (int i = 0, end = this->proxyModel->rowCount(); i < end; ++i)
    {
    QModelIndex idx =
      this->proxyModel->mapToSource(this->proxyModel->index(i, 0));
    int itemStatus =
      this->eventTreeModel->data(idx, vsEventTreeModel::StatusRole).toInt();

    if (itemStatus == this->viewMode)
      {
      this->eventTreeModel->setData(idx, false,
                                    vsEventTreeModel::DisplayStateRole);
      }
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::showAllItems()
{
  DisableSort ds(this->proxyModel.data());

  for (int i = 0, end = this->proxyModel->rowCount(); i < end; ++i)
    {
    QModelIndex idx =
      this->proxyModel->mapToSource(this->proxyModel->index(i, 0));
    int itemStatus =
      this->eventTreeModel->data(idx, vsEventTreeModel::StatusRole).toInt();

    if (itemStatus == this->viewMode)
      {
      this->eventTreeModel->setData(idx, true,
                                    vsEventTreeModel::DisplayStateRole);
      }
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::setHiddenItemsShown(bool enable)
{
  if (enable == this->showHiddenItems)
    return;

  this->showHiddenItems = enable;

  if (!this->proxyModel)
    {
    return;
    }

  for (int i = 0, end = this->proxyModel->rowCount(); i < end; ++i)
    {
    QModelIndex idx =
      this->proxyModel->mapToSource(this->proxyModel->index(i, 0));
    int itemStatus =
      this->eventTreeModel->data(idx, vsEventTreeModel::StatusRole).toInt();

    if (itemStatus != this->viewMode)
      continue;

    if (this->eventTreeModel->isIndexHidden(idx))
      this->setRowHidden(i, QModelIndex(), !enable);
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::updateRows(const QModelIndex& start,
                                 const QModelIndex& end)
{
  this->updateRows(QModelIndex(), start.row(), end.row());
}

//-----------------------------------------------------------------------------
void vsEventTreeView::updateRows(
  const QModelIndex& parent, int start, int end)
{
  for (int i = start; i <= end; ++i)
    {
    QModelIndex pidx = this->proxyModel->index(i, 0, parent);
    QModelIndex idx = this->proxyModel->mapToSource(pidx);

    int itemStatus =
      this->eventTreeModel->data(idx, vsEventTreeModel::StatusRole).toInt();

    bool hide;
    if (itemStatus != this->viewMode)
      {
      hide = true;
      }
    else if (this->eventTreeModel->isIndexHidden(idx))
      {
      hide = !this->showHiddenItems ;
      }
    else
      {
      hide = false;
      }

    if (this->isRowHidden(i, parent) != hide)
      {
      this->setRowHidden(i, parent, hide);

      // Unselect hidden rows
      if (hide)
        {
        this->selectionModel()->select(pidx, QItemSelectionModel::Rows |
                                             QItemSelectionModel::Deselect);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::updateSelection(
  const QItemSelection& selected, const QItemSelection& deselected)
{
  vsProxySelectionModel* const psm =
    static_cast<vsProxySelectionModel*>(this->proxySelectionModel.data());
  psm->select(selected);
  psm->deselect(deselected);
}

//-----------------------------------------------------------------------------
void vsEventTreeView::setCurrentIndex(const QModelIndex& current)
{
  const QModelIndex mappedCurrent = this->proxyModel->mapFromSource(current);
  this->proxySelectionModel->setCurrentIndex(
    mappedCurrent, QItemSelectionModel::Current);
}

//-----------------------------------------------------------------------------
void vsEventTreeView::dropEvent(QDropEvent* event)
{
  DisableSort ds(this->proxyModel.data());

  const QMimeData* data = event->mimeData();
  QByteArray encodedData = data->data("application/vnd.kitware.event");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);

  while (!stream.atEnd())
    {
    vtkIdType id;
    stream >> id;

    QModelIndex idx = this->eventTreeModel->indexOfEvent(id);

    // Give the dropped row this view's status to 'move' it to our view.
    this->eventTreeModel->setData(idx,
                                  this->viewMode,
                                  vsEventTreeModel::StatusRole);

    // Automatically give events a negative rating if they were dropped in the
    // Rejected list.
    if (this->viewMode == vs::RejectedEvent)
      {
      this->eventTreeModel->setData(
        idx, vgObjectStatus::Excluded, vsEventTreeModel::RatingRole);
      }
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeView::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
    {
    QModelIndex i = indexAt(event->pos());
    if (i.isValid() && i.column() == vsEventTreeModel::StarColumn)
      {
      const bool starred =
        this->model()->data(i, vsEventTreeModel::StarRole).toBool();
      this->model()->setData(i, !starred, vsEventTreeModel::StarRole);
      return;
      }
    }
  QTreeView::mousePressEvent(event);
}

//-----------------------------------------------------------------------------
vtkIdType vsEventTreeView::eventIdFromIndex(const QModelIndex& index) const
{
  const QModelIndex& i =
    this->eventTreeModel->index(index.row(), vsEventTreeModel::IdColumn);
  return this->eventTreeModel->data(i).value<vtkIdType>();
}
