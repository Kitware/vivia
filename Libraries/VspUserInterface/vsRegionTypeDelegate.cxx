// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsRegionTypeDelegate.h"

#include <QComboBox>

#include <vsContour.h>

//-----------------------------------------------------------------------------
vsRegionTypeDelegate::vsRegionTypeDelegate(QObject* parent)
  : qtComboBoxDelegate(parent)
{
}

//-----------------------------------------------------------------------------
vsRegionTypeDelegate::~vsRegionTypeDelegate()
{
}

//-----------------------------------------------------------------------------
QWidget* vsRegionTypeDelegate::createListEditor(QWidget* parent) const
{
  QWidget* widget = qtComboBoxDelegate::createListEditor(parent);
  vsContour::populateTypeWidget(qobject_cast<QComboBox*>(widget));
  return widget;
}

//-----------------------------------------------------------------------------
bool vsRegionTypeDelegate::compareData(
  const QVariant& a, const QVariant& b) const
{
  if (a.canConvert<vsContour::Type>() && b.canConvert<vsContour::Type>())
    return a.value<vsContour::Type>() == b.value<vsContour::Type>();
  return qtComboBoxDelegate::compareData(a, b);
}
