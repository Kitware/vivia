/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpTabWidget_h
#define __vpTabWidget_h

#include <QContextMenuEvent>
#include <QTabWidget>
#include <QTabBar>

class vpTabWidget : public QTabWidget
{
  Q_OBJECT

public:
  vpTabWidget(QWidget* parent = 0);
  virtual ~vpTabWidget();

signals:
  void tabBarContextMenu(QContextMenuEvent* event, int index);

protected:
  virtual void contextMenuEvent(QContextMenuEvent* event);
};

#endif
