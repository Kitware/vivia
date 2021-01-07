// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventTreeView_h
#define __vsEventTreeView_h

#include <QTreeView>

#include <qtGlobal.h>

#include <vtkType.h>

#include <vtkVgTypeDefs.h>

#include "vsEventStatus.h"

class vsEventTreeModel;
class vsEventTreeSelectionModel;

class QSortFilterProxyModel;

class vsEventTreeView : public QTreeView
{
  Q_OBJECT

public:
  vsEventTreeView(QWidget* parent = 0);
  virtual ~vsEventTreeView();

  void setStatusFilter(vsEventStatus s) { this->viewMode = s; }

  // Reimplemented from QWidget
  virtual void setModel(QAbstractItemModel* m);
  virtual void setSelectionModel(QItemSelectionModel* selectionModel);

  const vsEventTreeSelectionModel* underlyingSelectionModel()
    {
    return this->eventTreeSelectionModel;
    }

signals:
  void jumpToEvent(vtkIdType eventId, bool jumpToEnd);

public slots:
  void hideAllItems();
  void showAllItems();
  void setHiddenItemsShown(bool enable);

  void hideSelectedItems();
  void showSelectedItems();

  void setSelectedItemsRating(vgObjectStatus::enumObjectStatus rating);
  void setSelectedItemsStatus(vsEventStatus status);
  void setSelectedItemsStarred(bool starred);

  void jumpToSelectedStart();
  void jumpToSelectedEnd();

protected slots:
  void itemActivated(const QModelIndex& index);

  void updateRows(const QModelIndex& parent, int start, int end);
  void updateRows(const QModelIndex& start, const QModelIndex& end);

  void updateSelection(const QItemSelection& selected,
                       const QItemSelection& deselected);
  void setCurrentIndex(const QModelIndex& current);

protected:
  virtual void dropEvent(QDropEvent* event) QTE_OVERRIDE;
  virtual void mousePressEvent(QMouseEvent* event) QTE_OVERRIDE;

  vtkIdType eventIdFromIndex(const QModelIndex& index) const;

private:
  Q_DISABLE_COPY(vsEventTreeView)

  vsEventTreeModel* eventTreeModel;
  vsEventTreeSelectionModel* eventTreeSelectionModel;
  QScopedPointer<QSortFilterProxyModel> proxyModel;
  QScopedPointer<QItemSelectionModel> proxySelectionModel;

  bool showHiddenItems;
  vsEventStatus viewMode;
};

#endif
