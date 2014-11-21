/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsDescriptorArchiveSourcePlugin.h"

#include <QtPlugin>

Q_EXPORT_PLUGIN2(vsDescriptorArchiveSource, vsDescriptorArchiveSourcePlugin)

namespace { static const int keyLoadAction = 0; }

//-----------------------------------------------------------------------------
vsDescriptorArchiveSourcePlugin::vsDescriptorArchiveSourcePlugin() :
  vsArchiveSourcePlugin(vs::ArchiveDescriptorSource)
{
}

//-----------------------------------------------------------------------------
vsDescriptorArchiveSourcePlugin::~vsDescriptorArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
QString vsDescriptorArchiveSourcePlugin::identifier() const
{
  return "DescriptorArchive";
}

//-----------------------------------------------------------------------------
void vsDescriptorArchiveSourcePlugin::registerFactoryCliOptions(
  qtCliOptions& options)
{
  if (this->areBackendsAvailable())
    {
    this->addArchiveOption(options, "descriptors", "df");
    }
}

//-----------------------------------------------------------------------------
void vsDescriptorArchiveSourcePlugin::registerActions()
{
  if (this->areBackendsAvailable())
    {
    this->registerAction(
      keyLoadAction, "&Load Archive", "load-events", "Ctrl+O, D",
      "Load a descriptor (event) archive");
    }
}

//-----------------------------------------------------------------------------
void vsDescriptorArchiveSourcePlugin::insertActions(
  qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  if (this->areBackendsAvailable())
    {
    Q_UNUSED(videoMenu);
    Q_UNUSED(trackMenu);
    descriptorMenu.insertAction(this->action(keyLoadAction));
    }
}
