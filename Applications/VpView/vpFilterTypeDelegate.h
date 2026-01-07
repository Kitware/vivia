// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
