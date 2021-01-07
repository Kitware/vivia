// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
