/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
