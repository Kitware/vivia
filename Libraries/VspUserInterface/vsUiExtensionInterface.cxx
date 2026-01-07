// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
