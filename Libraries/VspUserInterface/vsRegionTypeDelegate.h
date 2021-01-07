// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
