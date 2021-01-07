// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpTabWidget.h"

//-----------------------------------------------------------------------------
vpTabWidget::vpTabWidget(QWidget* parent) : QTabWidget(parent)
{
}

//-----------------------------------------------------------------------------
vpTabWidget::~vpTabWidget()
{
}

//-----------------------------------------------------------------------------
void vpTabWidget::contextMenuEvent(QContextMenuEvent* event)
{
  int tab = this->tabBar()->tabAt(event->pos());
  if (tab >= 0)
  {
    this->setCurrentIndex(tab);
    emit tabBarContextMenu(event, tab);
  }
}
