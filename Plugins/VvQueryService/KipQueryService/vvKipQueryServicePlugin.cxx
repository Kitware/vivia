/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvKipQueryServicePlugin.h"

#include <QStringList>
#include <QUrl>
#include <QtPlugin>

#include <vvQueryServerDialog.h>

#include "vvKipQueryServerChooser.h"
#include "vvKipQuerySession.h"

Q_EXPORT_PLUGIN2(vvKipQueryService, vvKipQueryServicePlugin)

//-----------------------------------------------------------------------------
vvKipQueryServicePlugin::vvKipQueryServicePlugin()
{
}

//-----------------------------------------------------------------------------
vvKipQueryServicePlugin::~vvKipQueryServicePlugin()
{
}

//-----------------------------------------------------------------------------
QStringList vvKipQueryServicePlugin::supportedSchemes() const
{
  QStringList schemes;
  schemes.append("kip");
  return schemes;
}

//-----------------------------------------------------------------------------
void vvKipQueryServicePlugin::registerChoosers(vvQueryServerDialog* dialog)
{
  dialog->registerServerType("KWIVER In-Process",
                             QRegExp("kip", Qt::CaseInsensitive),
                             new vvKipQueryServerChooser);
}

//-----------------------------------------------------------------------------
vvQuerySession* vvKipQueryServicePlugin::createSession(const QUrl& server)
{
  return new vvKipQuerySession(server);
}
