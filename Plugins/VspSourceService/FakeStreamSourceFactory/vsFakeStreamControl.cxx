/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
