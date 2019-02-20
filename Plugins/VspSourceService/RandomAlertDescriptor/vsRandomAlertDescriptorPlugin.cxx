/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
