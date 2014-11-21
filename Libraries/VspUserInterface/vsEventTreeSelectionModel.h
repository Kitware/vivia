/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsEventTreeSelectionModel_h
#define __vsEventTreeSelectionModel_h

#include <QItemSelectionModel>

#include <vtkType.h>

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
  QModelIndexList selectedRows(QTreeView* view, int column = 0) const;

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
