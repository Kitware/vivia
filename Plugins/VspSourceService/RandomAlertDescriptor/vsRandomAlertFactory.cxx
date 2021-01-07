// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsRandomAlertFactory.h"

#include "vsRandomAlertDescriptor.h"

QTE_IMPLEMENT_D_FUNC(vsRandomAlertFactory)

//-----------------------------------------------------------------------------
class vsRandomAlertFactoryPrivate
{
public:
  vsDescriptorSourcePtr descriptor;
};

//-----------------------------------------------------------------------------
vsRandomAlertFactory::vsRandomAlertFactory() :
  d_ptr(new vsRandomAlertFactoryPrivate)
{
}

//-----------------------------------------------------------------------------
vsRandomAlertFactory::~vsRandomAlertFactory()
{
}

//-----------------------------------------------------------------------------
bool vsRandomAlertFactory::initialize(QWidget*)
{
  return this->initialize();
}

//-----------------------------------------------------------------------------
bool vsRandomAlertFactory::initialize(const QUrl&)
{
  return this->initialize();
}

//-----------------------------------------------------------------------------
bool vsRandomAlertFactory::initialize()
{
  QTE_D(vsRandomAlertFactory);
  d->descriptor = vsDescriptorSourcePtr(new vsRandomAlertDescriptor());
  return true;
}

//-----------------------------------------------------------------------------
QList<vsDescriptorSourcePtr> vsRandomAlertFactory::descriptorSources() const
{
  QTE_D_CONST(vsRandomAlertFactory);

  if (!d->descriptor)
    {
    return vsSourceFactory::descriptorSources();
    }

  QList<vsDescriptorSourcePtr> list;
  list.append(d->descriptor);
  return list;
}
