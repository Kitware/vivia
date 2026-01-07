// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vdfSelector.h"

///////////////////////////////////////////////////////////////////////////////

//BEGIN vdfSelectorType

//-----------------------------------------------------------------------------
vdfSelectorType::vdfSelectorType(const QMetaObject* d) : d_ptr(d)
{
}

//-----------------------------------------------------------------------------
vdfSelectorType::~vdfSelectorType()
{
}

//-----------------------------------------------------------------------------
vdfSelectorType& vdfSelectorType::operator=(const QMetaObject* d)
{
  this->d_ptr = d;
  return *this;
}

//-----------------------------------------------------------------------------
vdfSelectorType::operator const QMetaObject*() const
{
  return this->d_ptr;
}

//-----------------------------------------------------------------------------
const QMetaObject& vdfSelectorType::operator*() const
{
  return *this->d_ptr;
}

//-----------------------------------------------------------------------------
const QMetaObject& vdfSelectorType::operator->() const
{
  return *this->d_ptr;
}

//-----------------------------------------------------------------------------
uint qHash(const vdfSelectorType& st)
{
  const QMetaObject* mo = st;
  return qHash(mo);
}

//END vdfSelectorType

///////////////////////////////////////////////////////////////////////////////

//BEGIN vdfSelector

//-----------------------------------------------------------------------------
vdfSelector::vdfSelector(): QObject()
{
}

//-----------------------------------------------------------------------------
vdfSelector::~vdfSelector()
{
}

//-----------------------------------------------------------------------------
vdfSelectorType vdfSelector::type() const
{
  return this->metaObject();
}

//END vdfSelector
