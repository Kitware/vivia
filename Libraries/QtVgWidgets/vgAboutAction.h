/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgAboutAction_h
#define __vgAboutAction_h

#include <QAction>

#include <vgExport.h>

class QTVG_WIDGETS_EXPORT vgAboutAction : public QAction
{
  Q_OBJECT

public:
  explicit vgAboutAction(QObject* parent);
  virtual ~vgAboutAction();

protected slots:
  void showDialog();

private:
  Q_DISABLE_COPY(vgAboutAction)
};

#endif
