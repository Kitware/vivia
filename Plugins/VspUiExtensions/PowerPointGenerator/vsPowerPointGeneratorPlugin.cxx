/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
