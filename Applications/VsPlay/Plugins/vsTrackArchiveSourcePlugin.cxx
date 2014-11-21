/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackArchiveSourcePlugin.h"

#include <QtPlugin>

Q_EXPORT_PLUGIN2(vsTrackArchiveSource, vsTrackArchiveSourcePlugin)

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
