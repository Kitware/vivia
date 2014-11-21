/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackTreeWidget_h
#define __vsTrackTreeWidget_h

#include <QWidget>

#include "ui_trackTree.h"
#include "am_trackTree.h"

class QMenu;

class vsTrackTreeWidget : public QWidget
{
  Q_OBJECT

public:
  vsTrackTreeWidget(QWidget* parent);
  ~vsTrackTreeWidget();

  void setModel(vsTrackTreeModel*);
  void setHiddenItemsShown(bool enable);

  virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
  void jumpToTrack(vtkIdType trackId, bool jumpToEnd);
  void selectionChanged(QList<vtkIdType> selectedTrackIds);
  void trackFollowingRequested(vtkIdType trackId);
  void trackFollowingCanceled();

public slots:
  void selectTrack(vtkIdType trackId);

protected slots:
  void setActionsEnabled();

protected:
  Ui::vsTrackTreeWidget UI;
  Am::vsTrackTreeWidget AM;

  QMenu* contextMenu;

private:
  Q_DISABLE_COPY(vsTrackTreeWidget)
};

#endif
