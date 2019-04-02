/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsEventCreationToolsPlugin.h"

#include <QtPlugin>

#include "vsEventCreationToolsInterface.h"

//-----------------------------------------------------------------------------
vsEventCreationToolsPlugin::vsEventCreationToolsPlugin()
{
}

//-----------------------------------------------------------------------------
vsEventCreationToolsPlugin::~vsEventCreationToolsPlugin()
{
}

//-----------------------------------------------------------------------------
void vsEventCreationToolsPlugin::createInterface(
  vsMainWindow* window, vsScene* scene)
{
  new vsEventCreationToolsInterface(window, scene, this->core());
}
