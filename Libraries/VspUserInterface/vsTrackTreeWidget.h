// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
