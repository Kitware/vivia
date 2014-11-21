/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
