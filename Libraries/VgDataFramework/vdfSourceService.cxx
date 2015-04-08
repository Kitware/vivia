/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfSourceService.h"

#include "vdfArchiveSourceInterface.h"

#include <vgPluginLoader.h>

#include <qtGlobal.h>

#include <QHash>

//-----------------------------------------------------------------------------
class vdfSourceServiceInstance : public QObject
{
public:
  vdfSourceServiceInstance();

  QList<vdfArchiveSourceInterface*> ArchiveSources;
};

QTE_PRIVATE_SINGLETON(vdfSourceServiceInstance, ssInstance)

//-----------------------------------------------------------------------------
vdfSourceServiceInstance::vdfSourceServiceInstance()
{
  // Load plugins
  foreach (QObject* object, vgPluginLoader::plugins())
    {
    // Check if plugin provides (an) archive source(s)
    vdfArchiveSourceInterface* archiveInterface =
      qobject_cast<vdfArchiveSourceInterface*>(object);
    if (archiveInterface)
      {
      this->ArchiveSources.append(archiveInterface);
      }
    }
}

//-----------------------------------------------------------------------------
QList<vdfArchivePluginInfo> vdfSourceService::archivePluginInfo()
{
  QList<vdfArchivePluginInfo> result;
  foreach (vdfArchiveSourceInterface* const archiveSource,
           ssInstance()->ArchiveSources)
    {
    result.append(archiveSource->archivePluginInfo());
    }
  return result;
}

//-----------------------------------------------------------------------------
vdfDataSource* vdfSourceService::createArchiveSource(const QUrl& archive)
{
  vdfArchiveSourceInterface::SourceCreateMode mode =
    vdfArchiveSourceInterface::QuickTest;

  forever
    {
    // Run through available plugins, first in QuickTest mode, then if that
    // fails, in ThoroughTest mode
    foreach (vdfArchiveSourceInterface* archiveSource,
             ssInstance()->ArchiveSources)
      {
      vdfDataSource* const source =
        archiveSource->createArchiveSource(archive, mode);
      if (source)
        {
        return source;
        }
      }

    if (mode != vdfArchiveSourceInterface::ThoroughTest)
      {
      // Try again in ThoroughTest mode
      mode = vdfArchiveSourceInterface::ThoroughTest;
      continue;
      }

    // If we failed to create a source in ThoroughTest mode, give up
    break;
    }
  return 0;
}
