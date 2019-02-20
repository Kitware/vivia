/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsReportGeneratorPlugin.h"

#include <QtPlugin>

#include "vsReportGeneratorInterface.h"

//-----------------------------------------------------------------------------
vsReportGeneratorPlugin::vsReportGeneratorPlugin()
{
}

//-----------------------------------------------------------------------------
vsReportGeneratorPlugin::~vsReportGeneratorPlugin()
{
}

//-----------------------------------------------------------------------------
void vsReportGeneratorPlugin::createInterface(
  vsMainWindow* window, vsScene* scene)
{
  new vsReportGeneratorInterface(window, scene, this->core());
}
