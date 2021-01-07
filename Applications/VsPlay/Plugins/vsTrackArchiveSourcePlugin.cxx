// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsTrackArchiveSourcePlugin.h"
#include "moc_vsTrackArchiveSourcePlugin.cpp"

#include <QtPlugin>

namespace { static const int keyLoadAction = 0; }

//-----------------------------------------------------------------------------
vsTrackArchiveSourcePlugin::vsTrackArchiveSourcePlugin() :
  vsArchiveSourcePlugin(vs::ArchiveTrackSource)
{
}

//-----------------------------------------------------------------------------
vsTrackArchiveSourcePlugin::~vsTrackArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
QString vsTrackArchiveSourcePlugin::identifier() const
{
  return "TrackArchive";
}

//-----------------------------------------------------------------------------
void vsTrackArchiveSourcePlugin::registerFactoryCliOptions(
  qtCliOptions& options)
{
  if (this->areBackendsAvailable())
    {
    this->addArchiveOption(options, "tracks", "tf");
    }
}

//-----------------------------------------------------------------------------
void vsTrackArchiveSourcePlugin::registerActions()
{
  if (this->areBackendsAvailable())
    {
    this->registerAction(
      keyLoadAction, "&Load Archive", "load-tracks", "Ctrl+O, T",
      "Load a track archive");
    }
}

//-----------------------------------------------------------------------------
void vsTrackArchiveSourcePlugin::insertActions(
  qtPrioritizedMenuProxy& videoMenu, qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  if (this->areBackendsAvailable())
    {
    Q_UNUSED(videoMenu);
    trackMenu.insertAction(this->action(keyLoadAction));
    Q_UNUSED(descriptorMenu);
    }
}
