/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackTreeView_h
#define __vsTrackTreeView_h

#include <QTreeView>

#include <qtGlobal.h>

#include <vtkType.h>

#include <vtkVgTypeDefs.h>

class vsTrackTreeModel;
class vsTrackTreeSelectionModel;

class QSortFilterProxyModel;

class vsTrackTreeView : public QTreeView
{
  Q_OBJECT

public:
  vsTrackTreeView(QWidget* parent = 0);
  virtual ~vsTrackTreeView();

  // Reimplemented from QWidget
  virtual void setModel(QAbstractItemModel* m);
  virtual void setSelectionModel(QItemSelectionModel* selectionModel);

  const vsTrackTreeSelectionModel* underlyingSelectionModel()
    {
    return this->trackTreeSelectionModel;
    }

signals:
  void jumpToTrack(vtkIdType trackId, bool jumpToEnd);
  void trackFollowingRequested(vtkIdType trackId);

public slots:
  void hideAllItems();
  void showAllItems();
  void setHiddenItemsShown(bool enable);

  void hideSelectedItems();
  void showSelectedItems();

  void setSelectedItemsStarred(bool starred);

  void selectTrack(vtkIdType trackId);

  void jumpToSelectedStart();
  void jumpToSelectedEnd();
  void followSelectedTrack();

protected slots:
  void itemActivated(const QModelIndex& index);

  void updateRows(const QModelIndex& parent, int start, int end);
  void updateRows(const QModelIndex& start, const QModelIndex& end);

  void updateSelection(const QItemSelection& selected,
                       const QItemSelection& deselected);
  void setCurrentIndex(const QModelIndex& current);

protected:
  virtual void mousePressEvent(QMouseEvent* event) QTE_OVERRIDE;

  vtkIdType trackIdFromIndex(const QModelIndex& index) const;

private:
  Q_DISABLE_COPY(vsTrackTreeView)

  vsTrackTreeModel* trackTreeModel;
  vsTrackTreeSelectionModel* trackTreeSelectionModel;
  QScopedPointer<QSortFilterProxyModel> proxyModel;
  QScopedPointer<QItemSelectionModel> proxySelectionModel;

  bool showHiddenItems;
};

#endif
