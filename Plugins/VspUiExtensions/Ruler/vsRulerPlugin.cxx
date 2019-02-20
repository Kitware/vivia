/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
