/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsFakeStreamControl_h
#define __vsFakeStreamControl_h

#include <qtGlobal.h>

#include "ui_fakeStreamControl.h"

class vsFakeStreamControl : public QDialog
{
  Q_OBJECT

public:
  vsFakeStreamControl(QWidget* parent = 0);
  ~vsFakeStreamControl();

signals:
  void rateChanged(double newRate);
  void jitterChanged(double newJitter);
  void maxBurstTimeChanged(double newSize);
  void flushRequested();

protected slots:
  void jitterChanged(int);

protected:
  Ui::vsFakeStreamControl UI;

private:
  QTE_DISABLE_COPY(vsFakeStreamControl)
};

#endif
