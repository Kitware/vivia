// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsFakeStreamControl.h"

//-----------------------------------------------------------------------------
vsFakeStreamControl::vsFakeStreamControl(QWidget* parent) : QDialog(parent)
{
  this->UI.setupUi(this);

  connect(this->UI.rate, SIGNAL(valueChanged(double)),
          this, SIGNAL(rateChanged(double)));
  connect(this->UI.jitter, SIGNAL(valueChanged(int)),
          this, SLOT(jitterChanged(int)));
  connect(this->UI.maxBurstTime, SIGNAL(valueChanged(double)),
          this, SIGNAL(maxBurstTimeChanged(double)));
  connect(this->UI.flush, SIGNAL(clicked()),
          this, SIGNAL(flushRequested()));
}

//-----------------------------------------------------------------------------
vsFakeStreamControl::~vsFakeStreamControl()
{
}

//-----------------------------------------------------------------------------
void vsFakeStreamControl::jitterChanged(int newValue)
{
  emit this->jitterChanged(newValue * 0.01);
}
