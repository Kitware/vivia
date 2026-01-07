// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsPowerPointGeneratorPlugin.h"

#include <QtPlugin>

#include "vsPowerPointGeneratorInterface.h"

//-----------------------------------------------------------------------------
vsPowerPointGeneratorPlugin::vsPowerPointGeneratorPlugin()
{
}

//-----------------------------------------------------------------------------
vsPowerPointGeneratorPlugin::~vsPowerPointGeneratorPlugin()
{
}

//-----------------------------------------------------------------------------
void vsPowerPointGeneratorPlugin::createInterface(
  vsMainWindow* window, vsScene* scene)
{
  new vsPowerPointGeneratorInterface(window, scene, this->core());
}
