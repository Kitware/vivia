/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackTreeView_h
#define __vsTrackTreeView_h

#include <QTreeView>

#include <vtkType.h>

#include <vtkVgTypeDefs.h>

class vsTrackTreeModel;

class QSortFilterProxyModel;

class vsTrackTreeView : public QTreeView
{
  Q_OBJECT

public:
  vsTrackTreeView(QWidget* parent = 0);
  virtual ~vsTrackTreeView();

  int selectedTrackCount();

  // Reimplemented from QWidget
  virtual void setModel(QAbstractItemModel* m);

signals:
  void jumpToTrack(vtkIdType trackId, bool jumpToEnd);
  void selectionChanged(QList<vtkIdType> selectedEventIds);
  void trackFollowingRequested(vtkIdType trackId);

public slots:
  void hideAllItems();
  void showAllItems();
  void setHiddenItemsShown(bool enable);

  void hideSelectedItems();
  void showSelectedItems();

  void selectTrack(vtkIdType trackId);

  void jumpToSelectedStart();
  void jumpToSelectedEnd();
  void followSelectedTrack();

protected slots:
  void itemActivated(const QModelIndex& index);

  void updateRows(const QModelIndex& parent, int start, int end);
  void updateRows(const QModelIndex& start, const QModelIndex& end);

protected:
  virtual void selectionChanged(const QItemSelection&, const QItemSelection&);

  vtkIdType trackIdFromIndex(const QModelIndex& index);

private:
  Q_DISABLE_COPY(vsTrackTreeView)

  vsTrackTreeModel* trackTreeModel;
  QScopedPointer<QSortFilterProxyModel> proxyModel;

  bool showHiddenItems;
};

#endif
