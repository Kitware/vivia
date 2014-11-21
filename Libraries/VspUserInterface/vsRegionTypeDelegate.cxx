/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
