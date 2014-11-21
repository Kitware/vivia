/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackTreeView.h"

#include <QSortFilterProxyModel>

#include "vsTrackTreeModel.h"

namespace // anonymous
{

//-----------------------------------------------------------------------------
class vsSortFilterProxyModel : public QSortFilterProxyModel
{
protected:
  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const
    {
    if (left.column() == vsTrackTreeModel::NameColumn &&
        right.column() == vsTrackTreeModel::NameColumn)
      {
      const QVariant ad =
        left.model()->data(left, vsTrackTreeModel::LogicalIdRole);
      const QVariant bd =
        right.model()->data(right, vsTrackTreeModel::LogicalIdRole);
      const vvTrackId a = ad.value<vvTrackId>();
      const vvTrackId b = bd.value<vvTrackId>();
      return (a.Source == b.Source ? a.SerialNumber < b.SerialNumber
                                   : a.Source < b.Source);
      }

    return QSortFilterProxyModel::lessThan(left, right);
    }
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
vsTrackTreeView::vsTrackTreeView(QWidget* p)
  : QTreeView(p), showHiddenItems(true)
{
  connect(this, SIGNAL(activated(const QModelIndex&)),
          this, SLOT(itemActivated(const QModelIndex&)));
}

//-----------------------------------------------------------------------------
vsTrackTreeView::~vsTrackTreeView()
{
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::setModel(QAbstractItemModel* m)
{
  vsTrackTreeModel* tm = qobject_cast<vsTrackTreeModel*>(m);
  if (!tm)
    return;

  // NOTE: Caller is responsible for deleting any prior model
  this->trackTreeModel = tm;

  this->proxyModel.reset(new vsSortFilterProxyModel);
  this->proxyModel->setSourceModel(this->trackTreeModel);
  this->proxyModel->setDynamicSortFilter(false);

  QAbstractItemModel* old = this->model();
  this->QTreeView::setModel(this->proxyModel.data());
  delete old;

  this->setColumnHidden(vsTrackTreeModel::IdColumn, true);
  this->setColumnWidth(vsTrackTreeModel::NameColumn, 50);
  this->setColumnWidth(vsTrackTreeModel::StartTimeColumn, 80);
  this->setColumnWidth(vsTrackTreeModel::EndTimeColumn, 80);
  this->sortByColumn(vsTrackTreeModel::NameColumn, Qt::AscendingOrder);
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
int vsTrackTreeView::selectedTrackCount()
{
  if (this->selectionModel())
    return this->selectionModel()->selectedRows().count();
  return 0;
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::selectionChanged(const QItemSelection& selected,
                                       const QItemSelection& deselected)
{
  QList<vtkIdType> selectedIds;
  foreach (QModelIndex i, this->selectionModel()->selectedRows())
    {
    selectedIds << this->trackIdFromIndex(i);
    }

  emit this->selectionChanged(selectedIds);
  QTreeView::selectionChanged(selected, deselected);
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::itemActivated(const QModelIndex& index)
{
  bool toEnd = (index.column() == vsTrackTreeModel::EndTimeColumn);
  emit this->jumpToTrack(this->trackIdFromIndex(index), toEnd);
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::hideSelectedItems()
{
  DisableSort ds(this->proxyModel.data());

  QModelIndexList selected = this->selectionModel()->selectedRows();
  foreach (QModelIndex i, selected)
    {
    this->trackTreeModel->setData(
      this->proxyModel->mapToSource(i), false,
      vsTrackTreeModel::DisplayStateRole);
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::showSelectedItems()
{
  DisableSort ds(this->proxyModel.data());

  QModelIndexList selected = this->selectionModel()->selectedRows();
  foreach (QModelIndex i, selected)
    {
    this->trackTreeModel->setData(
      this->proxyModel->mapToSource(i), true,
      vsTrackTreeModel::DisplayStateRole);
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::jumpToSelectedStart()
{
  QModelIndex index = this->selectionModel()->selectedRows().front();
  emit this->jumpToTrack(trackIdFromIndex(index), false);
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::jumpToSelectedEnd()
{
  QModelIndex index = this->selectionModel()->selectedRows().front();
  emit this->jumpToTrack(trackIdFromIndex(index), true);
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::followSelectedTrack()
{
  QModelIndex index = this->selectionModel()->selectedRows().front();
  emit this->trackFollowingRequested(trackIdFromIndex(index));
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::hideAllItems()
{
  DisableSort ds(this->proxyModel.data());

  for (int i = 0, end = this->proxyModel->rowCount(); i < end; ++i)
    {
    QModelIndex idx =
      this->proxyModel->mapToSource(this->proxyModel->index(i, 0));

    this->trackTreeModel->setData(idx, false,
                                  vsTrackTreeModel::DisplayStateRole);
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::showAllItems()
{
  DisableSort ds(this->proxyModel.data());

  for (int i = 0, end = this->proxyModel->rowCount(); i < end; ++i)
    {
    QModelIndex idx =
      this->proxyModel->mapToSource(this->proxyModel->index(i, 0));

    this->trackTreeModel->setData(idx, true,
                                  vsTrackTreeModel::DisplayStateRole);
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::setHiddenItemsShown(bool enable)
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

    if (this->trackTreeModel->isIndexHidden(idx))
      this->setRowHidden(i, QModelIndex(), !enable);
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::updateRows(const QModelIndex& start,
                                 const QModelIndex& end)
{
  this->updateRows(QModelIndex(), start.row(), end.row());
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::updateRows(
  const QModelIndex& parent, int start, int end)
{
  for (int i = start; i <= end; ++i)
    {
    QModelIndex pidx = this->proxyModel->index(i, 0, parent);
    QModelIndex idx = this->proxyModel->mapToSource(pidx);

    bool hide;
    if (this->trackTreeModel->isIndexHidden(idx))
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
void vsTrackTreeView::selectTrack(vtkIdType trackId)
{
  QModelIndex index = this->trackTreeModel->indexOfTrack(trackId);
  index = this->proxyModel->mapFromSource(index);

  if (!index.isValid() || this->isRowHidden(index.row(), QModelIndex()))
    return;

  this->selectionModel()->setCurrentIndex(index,
                                          QItemSelectionModel::ClearAndSelect |
                                          QItemSelectionModel::Rows);
}

//-----------------------------------------------------------------------------
vtkIdType vsTrackTreeView::trackIdFromIndex(const QModelIndex& index)
{
  return this->model()->data(
           this->model()->index(
             index.row(), vsTrackTreeModel::IdColumn)).value<vtkIdType>();
}
