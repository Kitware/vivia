// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsVideoArchiveSourcePlugin.h"
#include "moc_vsVideoArchiveSourcePlugin.cpp"

#include <QtPlugin>

namespace { static const int keyLoadAction = 0; }

//-----------------------------------------------------------------------------
vsVideoArchiveSourcePlugin::vsVideoArchiveSourcePlugin() :
  vsArchiveSourcePlugin(vs::ArchiveVideoSource)
{
}

//-----------------------------------------------------------------------------
vsVideoArchiveSourcePlugin::~vsVideoArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
QString vsVideoArchiveSourcePlugin::identifier() const
{
  return "VideoArchive";
}

//-----------------------------------------------------------------------------
void vsVideoArchiveSourcePlugin::registerFactoryCliOptions(
  qtCliOptions& options)
{
  if (this->areBackendsAvailable())
    {
    this->addArchiveOption(options, "video", "vf");
    }
}

//-----------------------------------------------------------------------------
void vsVideoArchiveSourcePlugin::registerActions()
{
  if (this->areBackendsAvailable())
    {
    this->registerAction(
      keyLoadAction, "&Load Archive", "load-video", "Ctrl+O, V",
      "Load a video archive");
    }
}

//-----------------------------------------------------------------------------
void vsVideoArchiveSourcePlugin::insertActions(
  qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  if (this->areBackendsAvailable())
    {
    videoMenu.insertAction(this->action(keyLoadAction));
    Q_UNUSED(trackMenu);
    Q_UNUSED(descriptorMenu);
    }
}
