/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFilterTypeDelegate_h
#define __vpFilterTypeDelegate_h

#include <qtComboBoxDelegate.h>

class vpFilterTypeDelegate : public qtComboBoxDelegate
{
public:
  vpFilterTypeDelegate(QObject* parent = 0);
  virtual ~vpFilterTypeDelegate();

private:
  Q_DISABLE_COPY(vpFilterTypeDelegate)
};

#endif
