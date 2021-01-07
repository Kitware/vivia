// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpProjectList_h
#define __vpProjectList_h

#include <QListWidget>

class vpProjectList : public QListWidget
{
  Q_OBJECT

public:
  vpProjectList(QWidget* parent = 0);
  ~vpProjectList();

signals:
  void closeProjectRequested(int index);

protected:
  virtual void contextMenuEvent(QContextMenuEvent* event);
};

#endif
