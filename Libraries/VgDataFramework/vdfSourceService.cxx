// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
