// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventTreeSelectionModel_h
#define __vsEventTreeSelectionModel_h

#include <QItemSelectionModel>

#include <vtkType.h>

class QSortFilterProxyModel;
class QTreeView;

class vsEventTreeModel;

class vsEventTreeSelectionModel : public QItemSelectionModel
{
  Q_OBJECT

public:
  vsEventTreeSelectionModel(vsEventTreeModel* model, QObject* parent = 0);
  virtual ~vsEventTreeSelectionModel();

  QSet<vtkIdType> selectedEvents() const;
  int selectedEventCount() const;

  // Reimplemented from QItemSelectionModel
  using QItemSelectionModel::selectedRows;
  QModelIndexList selectedRows(
    QTreeView* view, QSortFilterProxyModel* viewProxy, int column = 0) const;

  const vsEventTreeModel* eventTreeModel() const
    {
    return this->EventTreeModel;
    }

signals:
  void selectionChanged(QSet<vtkIdType> selectedEventIds);

public slots:
  void selectEvent(vtkIdType eventId,
                   SelectionFlags mode = ClearAndSelect | Current);

  void setSelectedEvents(QSet<vtkIdType> selectedIds);
  void setCurrentEvent(vtkIdType currentId);

protected slots:
  void updateSelection();

protected:
  vtkIdType eventIdFromIndex(const QModelIndex& index) const;

private:
  Q_DISABLE_COPY(vsEventTreeSelectionModel)

  vsEventTreeModel* const EventTreeModel;
};

#endif
