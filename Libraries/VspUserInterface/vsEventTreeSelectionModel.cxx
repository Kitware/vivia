// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsEventTreeSelectionModel.h"

#include "vsEventTreeModel.h"
#include "vsEventTreeView.h"

#include <QSortFilterProxyModel>

//-----------------------------------------------------------------------------
vsEventTreeSelectionModel::vsEventTreeSelectionModel(
  vsEventTreeModel* model, QObject* parent) :
  QItemSelectionModel(model, parent),
  EventTreeModel(model)
{
  connect(this, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(updateSelection()));
}

//-----------------------------------------------------------------------------
vsEventTreeSelectionModel::~vsEventTreeSelectionModel()
{
}

//-----------------------------------------------------------------------------
vtkIdType vsEventTreeSelectionModel::eventIdFromIndex(
  const QModelIndex& index) const
{
  const QModelIndex ii =
    this->EventTreeModel->index(index.row(), vsEventTreeModel::IdColumn);
  return this->model()->data(ii).value<vtkIdType>();
}

//-----------------------------------------------------------------------------
QSet<vtkIdType> vsEventTreeSelectionModel::selectedEvents() const
{
  QSet<vtkIdType> selectedIds;
  foreach (QModelIndex i, this->selectedRows())
    {
    selectedIds.insert(this->eventIdFromIndex(i));
    }
  return selectedIds;
}

//-----------------------------------------------------------------------------
int vsEventTreeSelectionModel::selectedEventCount() const
{
  return this->selectedRows().count();
}

//-----------------------------------------------------------------------------
void vsEventTreeSelectionModel::updateSelection()
{
  emit this->selectionChanged(this->selectedEvents());
}

//-----------------------------------------------------------------------------
void vsEventTreeSelectionModel::selectEvent(
  vtkIdType eventId, SelectionFlags mode)
{
  QModelIndex index = this->EventTreeModel->indexOfEvent(eventId);

  if (mode & Current)
    {
    // Just having the 'current' flag does not seem sufficient to actually set
    // the current index... so call setCurrentIndex directly
    this->setCurrentIndex(index, mode | Rows);
    }
  else
    {
    this->select(index, mode | Rows);
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeSelectionModel::setSelectedEvents(QSet<vtkIdType> selectedIds)
{
  QItemSelection selection;
  foreach (vtkIdType id, selectedIds)
    {
    QModelIndex index = this->EventTreeModel->indexOfEvent(id);
    if (index.isValid())
      {
      selection.select(index, index);
      }
    }
  this->select(selection, ClearAndSelect | Rows);
}

//-----------------------------------------------------------------------------
void vsEventTreeSelectionModel::setCurrentEvent(vtkIdType currentId)
{
  if (currentId != -1)
    {
    QModelIndex index = this->EventTreeModel->indexOfEvent(currentId);
    if (index.isValid())
      {
      this->setCurrentIndex(index, Current | Rows);
      }
    }
}

//-----------------------------------------------------------------------------
QModelIndexList vsEventTreeSelectionModel::selectedRows(
  QTreeView* view, QSortFilterProxyModel* viewProxy, int column) const
{
  QModelIndexList globalSelection = this->selectedRows(column);
  QModelIndexList viewSelection;
  foreach(const QModelIndex& i, globalSelection)
    {
    const QModelIndex& si = viewProxy->mapFromSource(i);
    if (!view->isRowHidden(si.row(), si.parent()))
      {
      viewSelection.append(i);
      }
    }

  return viewSelection;
}
