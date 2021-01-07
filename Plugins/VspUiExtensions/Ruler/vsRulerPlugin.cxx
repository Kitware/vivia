// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsRulerPlugin.h"

#include <QtPlugin>

#include "vsRulerInterface.h"

//-----------------------------------------------------------------------------
vsRulerPlugin::vsRulerPlugin()
{
}

//-----------------------------------------------------------------------------
vsRulerPlugin::~vsRulerPlugin()
{
}

//-----------------------------------------------------------------------------
void vsRulerPlugin::createInterface(
  vsMainWindow* window, vsScene* scene)
{
  new vsRulerInterface(window, scene, this->core());
}
