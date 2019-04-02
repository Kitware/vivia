/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsFakeStreamSourceFactoryPlugin.h"

#include <QtPlugin>

#include "vsFakeStreamFactory.h"

namespace { static const int keyCreateAction = 0; }

//-----------------------------------------------------------------------------
vsFakeStreamSourceFactoryPlugin::vsFakeStreamSourceFactoryPlugin()
{
}

//-----------------------------------------------------------------------------
vsFakeStreamSourceFactoryPlugin::~vsFakeStreamSourceFactoryPlugin()
{
}

//-----------------------------------------------------------------------------
QString vsFakeStreamSourceFactoryPlugin::identifier() const
{
  return "FakeStream";
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourceFactoryPlugin::registerActions()
{
  this->registerAction(
    keyCreateAction, "Create Fa&ke Stream", QString(), "Ctrl+O, K",
    "Create a fake stream");
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourceFactoryPlugin::insertActions(
  qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  videoMenu.insertAction(this->action(keyCreateAction), 100);
  Q_UNUSED(trackMenu);
  Q_UNUSED(descriptorMenu);
}

//-----------------------------------------------------------------------------
vsSourceFactory* vsFakeStreamSourceFactoryPlugin::createFactory()
{
  return new vsFakeStreamFactory;
}
