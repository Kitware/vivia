/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsReportGeneratorPlugin.h"

#include <QtPlugin>

#include "vsReportGeneratorInterface.h"

Q_EXPORT_PLUGIN2(vsReportGeneratorExtension, vsReportGeneratorPlugin)

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
