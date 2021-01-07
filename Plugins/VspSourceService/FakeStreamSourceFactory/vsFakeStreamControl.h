// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
