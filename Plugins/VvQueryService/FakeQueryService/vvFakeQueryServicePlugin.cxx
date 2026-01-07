// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvFakeQueryServicePlugin.h"

#include <QStringList>
#include <QUrl>
#include <QtPlugin>

#include <vvQueryServerDialog.h>

#include "vvFakeQueryServerChooser.h"
#include "vvFakeQuerySession.h"

//-----------------------------------------------------------------------------
vvFakeQueryServicePlugin::vvFakeQueryServicePlugin()
{
}

//-----------------------------------------------------------------------------
vvFakeQueryServicePlugin::~vvFakeQueryServicePlugin()
{
}

//-----------------------------------------------------------------------------
QStringList vvFakeQueryServicePlugin::supportedSchemes() const
{
  QStringList schemes;
  schemes.append("fake");
  return schemes;
}

//-----------------------------------------------------------------------------
void vvFakeQueryServicePlugin::registerChoosers(vvQueryServerDialog* dialog)
{
  dialog->registerServerType("Fake", QRegExp("fake", Qt::CaseInsensitive),
                             new vvFakeQueryServerChooser);
}

//-----------------------------------------------------------------------------
vvQuerySession* vvFakeQueryServicePlugin::createSession(const QUrl& server)
{
  return new vvFakeQuerySession(server);
}
