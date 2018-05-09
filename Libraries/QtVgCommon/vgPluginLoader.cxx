/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgPluginLoader.h"

#include "qtMap.h"

#include <qtDebugImpl.h>
#include <qtGlobal.h>

#include <QDir>
#include <QHash>
#include <QLibrary>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QPluginLoader>

QTE_DEBUG_AREA(vgDebug, PluginLoader, false);

#define vgplDebug() qtDebug(vgDebugPluginLoader)

namespace vgPluginLoader
{
  extern const char* const builtinPaths[];
}

//-----------------------------------------------------------------------------
class vgPluginLoaderInstance
{
public:
  vgPluginLoaderInstance();

  void loadAll(const QDir& path);
  void load(const QString& name);
  void load(QObject*);

  QObjectList PluginInstanceObjects;
  QHash<const char*, QObjectList> PluginInterfaceMap;
  QMutex Mutex;
};

QTE_PRIVATE_SINGLETON(vgPluginLoaderInstance, plInstance)

//-----------------------------------------------------------------------------
vgPluginLoaderInstance::vgPluginLoaderInstance()
{
  // Load plugins, starting with those in our built-in paths...
  for (const char* const* dirp = vgPluginLoader::builtinPaths; *dirp; ++dirp)
    {
    const QString dir = QString::fromLocal8Bit(*dirp);
    vgplDebug() << "Searching built-in path" << dir << "for plugins";
    this->loadAll(dir + "/plugins");
    }
  // ...then those in VG_PLUGIN_PATH
  QByteArray envPath = qgetenv("VG_PLUGIN_PATH");
  if (!envPath.isEmpty())
    {
    vgplDebug() << "Searching environment path(s)" << envPath << "for plugins";
#if defined(Q_OS_WIN) || defined(Q_OS_SYMBIAN)
    QChar pathSep(';');
#else
    QChar pathSep(':');
#endif
    const QStringList paths =
      QString::fromLocal8Bit(envPath).split(pathSep, QString::SkipEmptyParts);
    foreach (const QString& path, paths)
      {
      this->loadAll(path + "/plugins");
      }
    }

  // Also load plugins specified by QSettings but not in VG_PLUGIN_PATH
  QSettings settings;
  settings.beginGroup("Plugins");
  foreach (const QString& key, settings.childKeys())
    {
    const QString name = settings.value(key).toString();
    vgplDebug() << "Attempting to load plugin" << name
                << "specified in settings";
    this->load(name);
    }

  // Finally, load any imported plugins
  foreach (QObject* plugin, QPluginLoader::staticInstances())
    {
    vgplDebug() << "Registering static plugin" << plugin;
    this->load(plugin);
    }
}

//-----------------------------------------------------------------------------
void vgPluginLoaderInstance::loadAll(const QDir& path)
{
  foreach (const QString& file, path.entryList(QDir::Files))
    {
    if (QLibrary::isLibrary(file))
      {
      vgplDebug() << "Attempting to load possible plugin" << file
                  << "from environment path";
      this->load(path.absoluteFilePath(file));
      }
    }
}

//-----------------------------------------------------------------------------
void vgPluginLoaderInstance::load(const QString& name)
{
  QPluginLoader loader(name);
  this->load(loader.instance());
  if (qtDebug::isAreaActive(vgDebugPluginLoader))
    {
    if (loader.isLoaded())
      {
      vgplDebug() << "Loading of plugin" << name << "succeeded";
      }
    else
      {
      vgplDebug() << "Loading of plugin" << name << "failed:"
                  << loader.errorString();
      }
    }
}

//-----------------------------------------------------------------------------
void vgPluginLoaderInstance::load(QObject* plugin)
{
  if (plugin)
    {
    this->PluginInstanceObjects.append(plugin);
    vgplDebug() << "Plugin" << plugin << "registered";
    }
}

//-----------------------------------------------------------------------------
QObjectList vgPluginLoader::plugins(const char* interfaceId)
{
  vgPluginLoaderInstance* const pl = plInstance();

  if (!interfaceId)
    {
    return pl->PluginInstanceObjects;
    }

  QMutexLocker lock(&pl->Mutex);
  if (!pl->PluginInterfaceMap.contains(interfaceId))
    {
    QObjectList& result = pl->PluginInterfaceMap[interfaceId];
    foreach (QObject* plugin, pl->PluginInstanceObjects)
      {
      if (plugin->qt_metacast(interfaceId))
        {
        result.append(plugin);
        }
      }
    return result;
    }

  return pl->PluginInterfaceMap[interfaceId];
}
