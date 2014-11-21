/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvQueryService.h"

#include <QHash>
#include <QStringList>

#include <qtGlobal.h>
#include <qtMap.h>

#include <vgPluginLoader.h>

#include "vvQueryServiceInterface.h"

//-----------------------------------------------------------------------------
class vvQueryServiceInstance
{
public:
  vvQueryServiceInstance();

  void load(QObject*);

  QList<vvQueryServiceInterface*> Plugins;
  QHash<QString, vvQueryServiceInterface*> PluginMap;
};

QTE_PRIVATE_SINGLETON(vvQueryServiceInstance, qsInstance)

//-----------------------------------------------------------------------------
vvQueryServiceInstance::vvQueryServiceInstance()
{
  qtUtil::mapBound(vgPluginLoader::plugins(), this,
                   &vvQueryServiceInstance::load);
}

//-----------------------------------------------------------------------------
void vvQueryServiceInstance::load(QObject* object)
{
  vvQueryServiceInterface* plugin =
    qobject_cast<vvQueryServiceInterface*>(object);
  if (plugin)
    {
    this->Plugins.append(plugin);
    const QStringList& schemes = plugin->supportedSchemes();
    foreach (const QString& scheme, schemes)
      {
      this->PluginMap.insert(scheme, plugin);
      }
    }
}

//-----------------------------------------------------------------------------
QStringList vvQueryService::supportedSchemes()
{
  return qsInstance()->PluginMap.keys();
}

//-----------------------------------------------------------------------------
void vvQueryService::registerChoosers(vvQueryServerDialog* dialog)
{
  foreach (vvQueryServiceInterface* plugin, qsInstance()->Plugins)
    {
    plugin->registerChoosers(dialog);
    }
}

//-----------------------------------------------------------------------------
vvQuerySession* vvQueryService::createSession(const QUrl& uri)
{
  vvQueryServiceInterface* plugin =
    qsInstance()->PluginMap.value(uri.scheme().toLower());
  return (plugin ? plugin->createSession(uri) : 0);
}
