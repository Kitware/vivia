/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpProjectList.h"

#include <QMenu>
#include <QContextMenuEvent>

//-----------------------------------------------------------------------------
vpProjectList::vpProjectList(QWidget* parent) :
  QListWidget(parent)
{
}

//-----------------------------------------------------------------------------
vpProjectList::~vpProjectList()
{
}

//-----------------------------------------------------------------------------
void vpProjectList::contextMenuEvent(QContextMenuEvent* event)
{
  if (this->selectedItems().size() != 1 ||
      this->itemAt(event->pos()) == 0)
    {
    return;
    }

  QMenu menu(this);
  QAction* closeProject = menu.addAction("Close Project");

  // Don't allow the last project to be closed.
  closeProject->setEnabled(this->count() > 1);

  if (menu.exec(event->globalPos()) == closeProject)
    {
    emit this->closeProjectRequested(
      this->indexFromItem(this->selectedItems()[0]).row());
    }
}
