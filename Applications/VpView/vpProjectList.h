/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
