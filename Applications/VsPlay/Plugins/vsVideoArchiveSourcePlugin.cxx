/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVideoArchiveSourcePlugin.h"

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
