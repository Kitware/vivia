// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsTrackTreeView.h"

#include <QHeaderView>
#include <QMouseEvent>
#include <QSortFilterProxyModel>

#include "vsTrackTreeModel.h"
#include "vsTrackTreeSelectionModel.h"

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
      const vsTrackId a = ad.value<vsTrackId>();
      const vsTrackId b = bd.value<vsTrackId>();
      return (a.Source == b.Source ? a.SerialNumber < b.SerialNumber
                                   : a.Source < b.Source);
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
vsTrackTreeView::vsTrackTreeView(QWidget* p) :
  QTreeView(p),
  trackTreeSelectionModel(0),
  showHiddenItems(true)
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
  this->setColumnWidth(vsTrackTreeModel::StarColumn,
                       this->header()->sectionSizeHint(
                         vsTrackTreeModel::StarColumn));
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
void vsTrackTreeView::setSelectionModel(QItemSelectionModel* m)
{
  vsTrackTreeSelectionModel* const tsm =
    qobject_cast<vsTrackTreeSelectionModel*>(m);

  if (!tsm)
    {
    return;
    }

  this->trackTreeSelectionModel = tsm;

  vsProxySelectionModel* const psm =
    new vsProxySelectionModel(this->proxyModel.data(), tsm);
  this->proxySelectionModel.reset(psm);
  this->QTreeView::setSelectionModel(psm);

  connect(tsm, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(updateSelection(QItemSelection, QItemSelection)));
  connect(tsm, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
          this, SLOT(setCurrentIndex(QModelIndex)));
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::itemActivated(const QModelIndex& index)
{
  bool toEnd = (index.column() == vsTrackTreeModel::EndTimeColumn);
  const auto& proxyIndex = this->proxyModel->mapToSource(index);
  emit this->jumpToTrack(this->trackIdFromIndex(proxyIndex), toEnd);
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::hideSelectedItems()
{
  DisableSort ds(this->proxyModel.data());

  const QModelIndexList& selected =
    this->trackTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  foreach (QModelIndex i, selected)
    {
    this->trackTreeModel->setData(
      i, false, vsTrackTreeModel::DisplayStateRole);
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::showSelectedItems()
{
  DisableSort ds(this->proxyModel.data());

  const QModelIndexList& selected =
    this->trackTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  foreach (QModelIndex i, selected)
    {
    this->trackTreeModel->setData(
      i, true, vsTrackTreeModel::DisplayStateRole);
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::jumpToSelectedStart()
{
  const QModelIndexList& selected =
    this->trackTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  Q_ASSERT(!selected.isEmpty());
  emit this->jumpToTrack(trackIdFromIndex(selected.front()), false);
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::jumpToSelectedEnd()
{
  const QModelIndexList& selected =
    this->trackTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  Q_ASSERT(!selected.isEmpty());
  emit this->jumpToTrack(trackIdFromIndex(selected.front()), true);
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::followSelectedTrack()
{
  const QModelIndexList& selected =
    this->trackTreeSelectionModel->selectedRows(this, this->proxyModel.data());
  Q_ASSERT(!selected.isEmpty());
  emit this->trackFollowingRequested(trackIdFromIndex(selected.front()));
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::setSelectedItemsStarred(bool starred)
{
  DisableSort ds(this->proxyModel.data());

  const QModelIndexList& selected =
    this->trackTreeSelectionModel->selectedRows(
      this, this->proxyModel.data(), vsTrackTreeModel::StarColumn);
  foreach (QModelIndex i, selected)
    {
    this->trackTreeModel->setData(i, starred, vsTrackTreeModel::StarRole);
    }
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
void vsTrackTreeView::updateSelection(
  const QItemSelection& selected, const QItemSelection& deselected)
{
  vsProxySelectionModel* const psm =
    static_cast<vsProxySelectionModel*>(this->proxySelectionModel.data());
  psm->select(selected);
  psm->deselect(deselected);
}

//-----------------------------------------------------------------------------
void vsTrackTreeView::setCurrentIndex(const QModelIndex& current)
{
  const QModelIndex mappedCurrent = this->proxyModel->mapFromSource(current);
  this->proxySelectionModel->setCurrentIndex(
    mappedCurrent, QItemSelectionModel::Current);
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
void vsTrackTreeView::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
    {
    QModelIndex i = indexAt(event->pos());
    if (i.isValid() && i.column() == vsTrackTreeModel::StarColumn)
      {
      const bool starred =
        this->model()->data(i, vsTrackTreeModel::StarRole).toBool();
      this->model()->setData(i, !starred, vsTrackTreeModel::StarRole);
      return;
      }
    }
  QTreeView::mousePressEvent(event);
}

//-----------------------------------------------------------------------------
vtkIdType vsTrackTreeView::trackIdFromIndex(const QModelIndex& index) const
{
  const QModelIndex& i =
    this->trackTreeModel->index(index.row(), vsTrackTreeModel::IdColumn);
  return this->trackTreeModel->data(i).value<vtkIdType>();
}
