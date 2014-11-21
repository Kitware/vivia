/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfSelectorSet.h"

#include "vdfSelector.h"

#include <QHash>

QTE_IMPLEMENT_D_FUNC_SHARED(vdfSelectorSet)

namespace // anonymous
{
  typedef QHash<vdfSelectorType, const vdfSelector*> SelectorCollection;
}

//-----------------------------------------------------------------------------
class vdfSelectorSetData : public QSharedData
{
public:
  vdfSelectorSetData() {}
  vdfSelectorSetData(const vdfSelectorSetData&);
  ~vdfSelectorSetData() { qDeleteAll(this->Selectors); }

  SelectorCollection Selectors;
};

//-----------------------------------------------------------------------------
vdfSelectorSetData::vdfSelectorSetData(const vdfSelectorSetData& other) :
  QSharedData()
{
  foreach (const vdfSelector* s, other.Selectors)
    {
    const vdfSelector* const sc = s->clone();
    this->Selectors.insert(sc->type(), sc);
    }
}

//-----------------------------------------------------------------------------
vdfSelectorSet::vdfSelectorSet() : d_ptr(new vdfSelectorSetData)
{
}

//-----------------------------------------------------------------------------
vdfSelectorSet::vdfSelectorSet(const vdfSelectorSet& other) :
  d_ptr(other.d_ptr)
{
}

//-----------------------------------------------------------------------------
vdfSelectorSet::~vdfSelectorSet()
{
}

//-----------------------------------------------------------------------------
vdfSelectorSet& vdfSelectorSet::operator=(const vdfSelectorSet& other)
{
  this->d_ptr = other.d_ptr;
  return *this;
}

//-----------------------------------------------------------------------------
void vdfSelectorSet::insert(const vdfSelector* selector)
{
  QTE_D_MUTABLE(vdfSelectorSet);

  const vdfSelectorType type = selector->type();
  delete d->Selectors.take(type);
  d->Selectors.insert(type, selector);
}

//-----------------------------------------------------------------------------
void vdfSelectorSet::remove(const vdfSelectorType& type)
{
  QTE_D_MUTABLE(vdfSelectorSet);
  delete d->Selectors.take(type);
}

//-----------------------------------------------------------------------------
QList<const vdfSelector*> vdfSelectorSet::selectors() const
{
  QTE_D_SHARED(vdfSelectorSet);
  return d->Selectors.values();
}
