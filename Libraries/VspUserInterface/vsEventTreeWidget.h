// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventTreeWidget_h
#define __vsEventTreeWidget_h

#include <QSet>
#include <QWidget>

#include <qtGlobal.h>

#include <vtkType.h>

#include "vsEventStatus.h"

class QAction;
class QMenu;

class vsEventTreeModel;
class vsEventTreeSelectionModel;

class vsEventTreeWidgetPrivate;

class vsEventTreeWidget : public QWidget
{
  Q_OBJECT

public:
  vsEventTreeWidget(QWidget* parent);
  ~vsEventTreeWidget();

  void setStatusFilter(vsEventStatus s);
  void setModel(vsEventTreeModel*);
  void setSelectionModel(vsEventTreeSelectionModel*);
  void setHiddenItemsShown(bool enable);

  virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
  void selectionChanged(QSet<vtkIdType> selectedEventIds);
  void selectionStatusChanged(bool selectionIsNonEmpty);

  void jumpToEvent(vtkIdType eventId, bool jumpToEnd);

  void setEventStartRequested(vtkIdType);
  void setEventEndRequested(vtkIdType);

public slots:
  void selectEvent(vtkIdType eventId);

protected slots:
  void updateSelectionStatus(QSet<vtkIdType> selection);

  void setSelectedEventsToUnratedVerified();
  void setSelectedEventsToRelevantVerified();
  void setSelectedEventsToNotRelevantVerified();
  void setSelectedEventsToNotRelevantRejected();

  void addStar();
  void removeStar();

  void editNote();

  void setSelectedEventsStart();
  void setSelectedEventsEnd();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsEventTreeWidget)

private:
  QTE_DECLARE_PRIVATE(vsEventTreeWidget)
  Q_DISABLE_COPY(vsEventTreeWidget)
};

#endif
