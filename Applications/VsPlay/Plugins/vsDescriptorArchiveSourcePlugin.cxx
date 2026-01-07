// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsDescriptorArchiveSourcePlugin.h"
#include "moc_vsDescriptorArchiveSourcePlugin.cpp"

#include <QtPlugin>

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
