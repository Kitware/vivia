// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
