// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
