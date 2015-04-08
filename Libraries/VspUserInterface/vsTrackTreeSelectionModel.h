/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackTreeSelectionModel_h
#define __vsTrackTreeSelectionModel_h

#include <QItemSelectionModel>

#include <vtkType.h>

class QSortFilterProxyModel;
class QTreeView;

class vsTrackTreeModel;

class vsTrackTreeSelectionModel : public QItemSelectionModel
{
  Q_OBJECT

public:
  vsTrackTreeSelectionModel(vsTrackTreeModel* model, QObject* parent = 0);
  virtual ~vsTrackTreeSelectionModel();

  QSet<vtkIdType> selectedTracks() const;
  int selectedTrackCount() const;

  // Reimplemented from QItemSelectionModel
  using QItemSelectionModel::selectedRows;
  QModelIndexList selectedRows(
    QTreeView* view, QSortFilterProxyModel* viewProxy, int column = 0) const;

  const vsTrackTreeModel* trackTreeModel() const
    {
    return this->TrackTreeModel;
    }

signals:
  void selectionChanged(QSet<vtkIdType> selectedTrackIds);

public slots:
  void selectTrack(vtkIdType trackId,
                   SelectionFlags mode = ClearAndSelect | Current);

  void setSelectedTracks(QSet<vtkIdType> selectedIds);
  void setCurrentTrack(vtkIdType currentId);

protected slots:
  void updateSelection();

protected:
  vtkIdType trackIdFromIndex(const QModelIndex& index) const;

private:
  Q_DISABLE_COPY(vsTrackTreeSelectionModel)

  vsTrackTreeModel* const TrackTreeModel;
};

#endif
