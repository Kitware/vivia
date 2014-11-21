/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvFakeQueryServicePlugin.h"

#include <QStringList>
#include <QUrl>
#include <QtPlugin>

#include <vvQueryServerDialog.h>

#include "vvFakeQueryServerChooser.h"
#include "vvFakeQuerySession.h"

Q_EXPORT_PLUGIN2(vvFakeQueryService, vvFakeQueryServicePlugin)

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
