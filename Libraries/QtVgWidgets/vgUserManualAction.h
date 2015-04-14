/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgUserManualAction_h
#define __vgUserManualAction_h

#include <QAction>

#include <vgExport.h>

class QTVG_WIDGETS_EXPORT vgUserManualAction : public QAction
{
  Q_OBJECT

public:
  explicit vgUserManualAction(QObject* parent);
  virtual ~vgUserManualAction();

protected slots:
  void showUserManual();

private:
  Q_DISABLE_COPY(vgUserManualAction)
};

#endif
