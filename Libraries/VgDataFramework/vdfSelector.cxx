/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
