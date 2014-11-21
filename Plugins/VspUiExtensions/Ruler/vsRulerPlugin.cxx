/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsRulerPlugin.h"

#include <QtPlugin>

#include "vsRulerInterface.h"

Q_EXPORT_PLUGIN2(vsRulerExtension, vsRulerPlugin)

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
