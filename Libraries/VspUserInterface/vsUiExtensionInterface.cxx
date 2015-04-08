/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsUiExtensionInterface.h"

QTE_IMPLEMENT_D_FUNC(vsUiExtensionInterface)

//-----------------------------------------------------------------------------
class vsUiExtensionInterfacePrivate
{
public:
  vsCore* Core;
};

//-----------------------------------------------------------------------------
vsUiExtensionInterface::vsUiExtensionInterface() :
  d_ptr(new vsUiExtensionInterfacePrivate)
{
}

//-----------------------------------------------------------------------------
vsUiExtensionInterface::~vsUiExtensionInterface()
{
}

//-----------------------------------------------------------------------------
void vsUiExtensionInterface::initialize(vsCore* core)
{
  QTE_D(vsUiExtensionInterface);
  d->Core = core;
}

//-----------------------------------------------------------------------------
vsCore* vsUiExtensionInterface::core() const
{
  QTE_D_CONST(vsUiExtensionInterface);
  return d->Core;
}

//-----------------------------------------------------------------------------
void vsUiExtensionInterface::createInterface(
  vsMainWindow* window, vsScene* scene)
{
  Q_UNUSED(window);
  Q_UNUSED(scene);
}

//-----------------------------------------------------------------------------
void vsUiExtensionInterface::registerExtensionCliOptions(qtCliOptions&)
{
}

//-----------------------------------------------------------------------------
void vsUiExtensionInterface::parseExtensionArguments(const qtCliArgs&)
{
}
