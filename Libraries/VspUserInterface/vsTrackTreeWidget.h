/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackTreeWidget_h
#define __vsTrackTreeWidget_h

#include <QSet>
#include <QWidget>

#include <qtGlobal.h>

#include <vtkType.h>

class QMenu;

class vsTrackTreeModel;
class vsTrackTreeSelectionModel;

class vsTrackTreeWidgetPrivate;

class vsTrackTreeWidget : public QWidget
{
  Q_OBJECT

public:
  vsTrackTreeWidget(QWidget* parent);
  ~vsTrackTreeWidget();

  void setModel(vsTrackTreeModel*);
  void setSelectionModel(vsTrackTreeSelectionModel*);
  void setHiddenItemsShown(bool enable);

  virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
  void jumpToTrack(vtkIdType trackId, bool jumpToEnd);
  void selectionChanged(QSet<vtkIdType> selectedTrackIds);
  void trackFollowingRequested(vtkIdType trackId);
  void trackFollowingCanceled();

public slots:
  void selectTrack(vtkIdType trackId);

protected slots:
  void updateSelectionStatus(QSet<vtkIdType> selection);

  void addStar();
  void removeStar();

  void editNote();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsTrackTreeWidget)

private:
  QTE_DECLARE_PRIVATE(vsTrackTreeWidget)
  QTE_DISABLE_COPY(vsTrackTreeWidget)
};

#endif
