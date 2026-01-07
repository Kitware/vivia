// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
