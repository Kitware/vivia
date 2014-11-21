/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsSourceService.h"

#include <QHash>

#include <qtGlobal.h>

#include <vgPluginLoader.h>

#include "vsArchiveSourceInterface.h"
#include "vsFactoryAction.h"
#include "vsSourceFactoryInterface.h"

//-----------------------------------------------------------------------------
class vsSourceServiceInstance : public QObject
{
public:
  vsSourceServiceInstance();

  QHash<QString, vsSourceFactoryInterface*> Factories;
  QHash<vsArchiveSourceType, QList<vsArchiveSourceInterface*> > ArchiveSources;
};

QTE_PRIVATE_SINGLETON(vsSourceServiceInstance, ssInstance)

//-----------------------------------------------------------------------------
vsSourceServiceInstance::vsSourceServiceInstance()
{
  // Load plugins
  foreach (QObject* object, vgPluginLoader::plugins())
    {
    // Check if plugin provides a source factory
    vsSourceFactoryInterface* factoryInterface =
      qobject_cast<vsSourceFactoryInterface*>(object);
    if (factoryInterface)
      {
      // Add to registered factories
      this->Factories.insert(factoryInterface->identifier(), factoryInterface);
      }

    // Check if plugin provides (an) archive source(s)
    vsArchiveSourceInterface* archiveInterface =
      qobject_cast<vsArchiveSourceInterface*>(object);
    if (archiveInterface)
      {
      vsArchiveSourceTypes supportedTypes = archiveInterface->archiveTypes();
      int bit = 1;
      // Iterate over possible source types
      while (supportedTypes)
        {
        vsArchiveSourceType type = static_cast<vsArchiveSourceType>(bit);
        if (supportedTypes.testFlag(type))
          {
          this->ArchiveSources[type].append(archiveInterface);
          supportedTypes ^= type;
          }
        bit <<= 1;
        }
      }

    }
}

//-----------------------------------------------------------------------------
void vsSourceService::registerCliOptions(qtCliOptions& options)
{
  foreach (vsSourceFactoryInterface* factoryInterface, ssInstance()->Factories)
    {
    factoryInterface->registerFactoryCliOptions(options);
    }
}

//-----------------------------------------------------------------------------
QList<vsPendingFactoryAction> vsSourceService::parseArguments(
  const qtCliArgs& args)
{
  QList<vsPendingFactoryAction> actions;
  foreach (vsSourceFactoryInterface* factoryInterface, ssInstance()->Factories)
    {
    actions += factoryInterface->parseFactoryArguments(args);
    }
  return actions;
}

//-----------------------------------------------------------------------------
void vsSourceService::registerActions()
{
  foreach (vsSourceFactoryInterface* factoryInterface, ssInstance()->Factories)
    {
    factoryInterface->registerActions();
    }
}

//-----------------------------------------------------------------------------
void vsSourceService::createActions(
  QObject* parent, QSignalMapper* signalMapper,
  qtPrioritizedMenuProxy& videoMenu,
  qtPrioritizedMenuProxy& trackMenu,
  qtPrioritizedMenuProxy& descriptorMenu)
{
  foreach (vsSourceFactoryInterface* factoryInterface, ssInstance()->Factories)
    {
    factoryInterface->createActions(parent, signalMapper,
                                    videoMenu, trackMenu, descriptorMenu);
    }
}

//-----------------------------------------------------------------------------
vsSourceFactoryPtr vsSourceService::createFactory(
  const QString& identifier)
{
  vsSourceFactoryInterface* factoryInterface =
    ssInstance()->Factories.value(identifier, 0);
  if (factoryInterface)
    {
    vsSourceFactory* factory = factoryInterface->createFactory();
    return vsSourceFactoryPtr(factory);
    }

  return vsSourceFactoryPtr();
}

//-----------------------------------------------------------------------------
QList<vsArchivePluginInfo> vsSourceService::archivePluginInfo(
  vsArchiveSourceType type)
{
  QList<vsArchivePluginInfo> result;
  const QList<vsArchiveSourceInterface*>& sources =
    ssInstance()->ArchiveSources[type];
  foreach (vsArchiveSourceInterface* archiveSource, sources)
    {
    result.append(archiveSource->archivePluginInfo(type));
    }
  return result;
}

//-----------------------------------------------------------------------------
vsSimpleSourceFactoryPtr vsSourceService::createArchiveSource(
  vsArchiveSourceType type, const QUrl& archive)
{
  vsArchiveSourceInterface::SourceCreateMode mode =
    vsArchiveSourceInterface::QuickTest;

  forever
    {
    // Run through available plugins, first in QuickTest mode, then if that
    // fails, in ThoroughTest mode
    const QList<vsArchiveSourceInterface*>& sources =
      ssInstance()->ArchiveSources[type];
    foreach (vsArchiveSourceInterface* archiveSource, sources)
      {
      vsSimpleSourceFactory* source =
        archiveSource->createArchiveSource(type, archive, mode);
      if (source)
        {
        return vsSimpleSourceFactoryPtr(source);
        }
      }

    if (mode != vsArchiveSourceInterface::ThoroughTest)
      {
      // Try again in ThoroughTest mode
      mode = vsArchiveSourceInterface::ThoroughTest;
      continue;
      }

    // If we failed to create a source in ThoroughTest mode, give up
    break;
    }
  return vsSimpleSourceFactoryPtr();
}
