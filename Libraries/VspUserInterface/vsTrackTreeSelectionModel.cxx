/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackTreeSelectionModel.h"

#include "vsTrackTreeModel.h"
#include "vsTrackTreeView.h"

#include <QSortFilterProxyModel>

//-----------------------------------------------------------------------------
vsTrackTreeSelectionModel::vsTrackTreeSelectionModel(
  vsTrackTreeModel* model, QObject* parent) :
  QItemSelectionModel(model, parent),
  TrackTreeModel(model)
{
  connect(this, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(updateSelection()));
}

//-----------------------------------------------------------------------------
vsTrackTreeSelectionModel::~vsTrackTreeSelectionModel()
{
}

//-----------------------------------------------------------------------------
vtkIdType vsTrackTreeSelectionModel::trackIdFromIndex(
  const QModelIndex& index) const
{
  const QModelIndex ii =
    this->TrackTreeModel->index(index.row(), vsTrackTreeModel::IdColumn);
  return this->model()->data(ii).value<vtkIdType>();
}

//-----------------------------------------------------------------------------
QSet<vtkIdType> vsTrackTreeSelectionModel::selectedTracks() const
{
  QSet<vtkIdType> selectedIds;
  foreach (QModelIndex i, this->selectedRows())
    {
    selectedIds.insert(this->trackIdFromIndex(i));
    }
  return selectedIds;
}

//-----------------------------------------------------------------------------
int vsTrackTreeSelectionModel::selectedTrackCount() const
{
  return this->selectedRows().count();
}

//-----------------------------------------------------------------------------
void vsTrackTreeSelectionModel::updateSelection()
{
  emit this->selectionChanged(this->selectedTracks());
}

//-----------------------------------------------------------------------------
void vsTrackTreeSelectionModel::selectTrack(
  vtkIdType trackId, SelectionFlags mode)
{
  QModelIndex index = this->TrackTreeModel->indexOfTrack(trackId);

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
void vsTrackTreeSelectionModel::setSelectedTracks(QSet<vtkIdType> selectedIds)
{
  QItemSelection selection;
  foreach (vtkIdType id, selectedIds)
    {
    QModelIndex index = this->TrackTreeModel->indexOfTrack(id);
    if (index.isValid())
      {
      selection.select(index, index);
      }
    }
  this->select(selection, ClearAndSelect | Rows);
}

//-----------------------------------------------------------------------------
void vsTrackTreeSelectionModel::setCurrentTrack(vtkIdType currentId)
{
  if (currentId != -1)
    {
    QModelIndex index = this->TrackTreeModel->indexOfTrack(currentId);
    if (index.isValid())
      {
      this->setCurrentIndex(index, Current | Rows);
      }
    }
}

//-----------------------------------------------------------------------------
QModelIndexList vsTrackTreeSelectionModel::selectedRows(
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
