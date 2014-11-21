/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFilterTypeDelegate.h"

#include "vtkVgTemporalFilters.h"

#include <QComboBox>

namespace
{

const vtkVgTemporalFilters::FilterType Types[] =
{
  vtkVgTemporalFilters::FT_Select,
  vtkVgTemporalFilters::FT_Exclude
};

enum { NumTypes = sizeof(Types) / sizeof(Types[0]) };

} // end anonymous namespace

//-----------------------------------------------------------------------------
vpFilterTypeDelegate::vpFilterTypeDelegate(QObject* parent)
  : qtComboBoxDelegate(parent)
{
  QVariantList vl;
  QStringList sl;
  for (int i = 0; i < NumTypes; ++i)
    {
    vl << Types[i];
    sl << vtkVgTemporalFilters::StringForType(Types[i]);
    }
  this->setMapping(sl, vl);
}

//-----------------------------------------------------------------------------
vpFilterTypeDelegate::~vpFilterTypeDelegate()
{
}
