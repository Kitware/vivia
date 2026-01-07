// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
