/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsRegionTypeDelegate_h
#define __vsRegionTypeDelegate_h

#include <qtComboBoxDelegate.h>

class vsRegionTypeDelegate : public qtComboBoxDelegate
{
public:
  vsRegionTypeDelegate(QObject* parent = 0);
  virtual ~vsRegionTypeDelegate();

protected:
  virtual QWidget* createListEditor(QWidget* parent) const;

protected:
  virtual bool compareData(const QVariant&, const QVariant&) const;

private:
  Q_DISABLE_COPY(vsRegionTypeDelegate)
};

#endif
