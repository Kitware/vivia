/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsArchiveSourceFactory.h"

#include <QFileInfo>
#include <QUrl>

#include <vgFileDialog.h>

#include <vsSourceService.h>

// Need these so that QSharedPointer can find their dtors
#include <vsDescriptorSource.h>
#include <vsTrackSource.h>
#include <vsVideoSource.h>

QTE_IMPLEMENT_D_FUNC(vsArchiveSourceFactory)

//-----------------------------------------------------------------------------
class vsArchiveSourceFactoryPrivate
{
public:
  vsArchiveSourceFactoryPrivate(vsArchiveSourceType type) : Type(type) {}

  QString typeString() const;

  const vsArchiveSourceType Type;
  vsSimpleSourceFactoryPtr Sources;
};

//-----------------------------------------------------------------------------
QString vsArchiveSourceFactoryPrivate::typeString() const
{
  switch (this->Type)
    {
    case vs::ArchiveVideoSource:      return "video";
    case vs::ArchiveTrackSource:      return "track";
    case vs::ArchiveDescriptorSource: return "descriptor";
    default: return QString("(unknown archive type: %1)").arg(this->Type);
    }
}

//-----------------------------------------------------------------------------
vsArchiveSourceFactory::vsArchiveSourceFactory(vsArchiveSourceType type) :
  d_ptr(new vsArchiveSourceFactoryPrivate(type))
{
}

//-----------------------------------------------------------------------------
vsArchiveSourceFactory::~vsArchiveSourceFactory()
{
}

//-----------------------------------------------------------------------------
bool vsArchiveSourceFactory::initialize(QWidget* dialogParent)
{
  QTE_D_CONST(vsArchiveSourceFactory);

  // Get information on available plugins
  QList<vsArchivePluginInfo> pluginInfo =
    vsSourceService::archivePluginInfo(d->Type);

  // Build filters
  QStringList filters;
  QStringList allTypes;
  foreach (const vsArchivePluginInfo& pi, pluginInfo)
    {
    foreach (const vsArchiveFileType& supportedType, pi.SupportedFileTypes)
      {
      allTypes.append(supportedType.Patterns);
      filters.append(QString("%1 (%2)").arg(supportedType.Description,
                                            supportedType.Patterns.join(" ")));
      }
    }

  // Create merged filter
  const QString type = d->typeString();
  const QString dialogTitle = QString("Load %1 archive...").arg(type);
  const QString mergedFilter = "Supported %1 formats (%2)";

  filters.prepend(mergedFilter.arg(type, allTypes.join(" ")));
  filters.append("All files (*)");

  // Get name of archive file to open
  const QString fileName = vgFileDialog::getOpenFileName(
                             dialogParent, dialogTitle, QString(),
                             filters.join(";;"));

  if (fileName.isEmpty())
    {
    return false;
    }

  return this->initialize(QUrl::fromLocalFile(fileName));
}

//-----------------------------------------------------------------------------
bool vsArchiveSourceFactory::initialize(const QUrl& uri)
{
  // Check that we are being asked to open a file
  if (uri.scheme() != "file")
    {
    QTE_D_CONST(vsArchiveSourceFactory);

    const QString message =
      "Error loading " + d->typeString() + " archive from URI \"" +
      uri.toString() + "\": the scheme " + uri.scheme() + " is not supported.";
    this->warn(0, "Not Supported", message);

    return false;
    }

  return this->initialize(uri, 0);
}

//-----------------------------------------------------------------------------
bool vsArchiveSourceFactory::initialize(const QUrl& uri, QWidget* dialogParent)
{
  QTE_D(vsArchiveSourceFactory);

  // Check that file exists
  QFileInfo fi(uri.toLocalFile());
  if (!fi.exists())
    {
    const QString message =
      "Error loading " + d->typeString() + " archive from file \"" +
      uri.toLocalFile() + "\": the file specified does not exist.";
    this->warn(dialogParent, "Error loading archive", message);

    return false;
    }

  // Get canonical path to file
  QUrl canonicalUri = QUrl::fromLocalFile(fi.canonicalFilePath());

  // Create source(s)
  vsSimpleSourceFactoryPtr sources =
    vsSourceService::createArchiveSource(d->Type, canonicalUri);
  if (!sources)
    {
    const QString message =
      "Error loading " + d->typeString() + " archive from file \"" +
      uri.toLocalFile() + "\": none of the loaded handlers accepted the file.";
    this->warn(dialogParent, "Error loading archive", message);

    return false;
    }

  d->Sources = sources;
  return true;
}

//-----------------------------------------------------------------------------
vsVideoSourcePtr vsArchiveSourceFactory::videoSource() const
{
  QTE_D_CONST(vsArchiveSourceFactory);
  return d->Sources->videoSource();
}

//-----------------------------------------------------------------------------
QList<vsTrackSourcePtr> vsArchiveSourceFactory::trackSources() const
{
  QTE_D_CONST(vsArchiveSourceFactory);
  return d->Sources->trackSources();
}

//-----------------------------------------------------------------------------
QList<vsDescriptorSourcePtr> vsArchiveSourceFactory::descriptorSources() const
{
  QTE_D_CONST(vsArchiveSourceFactory);
  return d->Sources->descriptorSources();
}
