// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
