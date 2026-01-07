// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsRandomAlertDescriptorPlugin.h"

#include <QtPlugin>

#include "vsRandomAlertFactory.h"

namespace { static const int keyCreateAction = 0; }

//-----------------------------------------------------------------------------
vsRandomAlertDescriptorPlugin::vsRandomAlertDescriptorPlugin()
{
}

//-----------------------------------------------------------------------------
vsRandomAlertDescriptorPlugin::~vsRandomAlertDescriptorPlugin()
{
}

//-----------------------------------------------------------------------------
QString vsRandomAlertDescriptorPlugin::identifier() const
{
  return "RandomAlert";
}

//-----------------------------------------------------------------------------
void vsRandomAlertDescriptorPlugin::registerActions()
{
  this->registerAction(
    keyCreateAction, "&Create Random Alert Descriptor", QString(), QString(),
    "Create a live descriptor that will generate random alerts");
}

//-----------------------------------------------------------------------------
void vsRandomAlertDescriptorPlugin::insertActions(
  qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  Q_UNUSED(videoMenu);
  Q_UNUSED(trackMenu);
  descriptorMenu.insertAction(this->action(keyCreateAction), 1000);
}

//-----------------------------------------------------------------------------
vsSourceFactory* vsRandomAlertDescriptorPlugin::createFactory()
{
  return new vsRandomAlertFactory;
}
