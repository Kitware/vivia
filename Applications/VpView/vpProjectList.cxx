// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
