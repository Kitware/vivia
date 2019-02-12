/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsPvoArchiveSourcePlugin.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QUrl>
#include <QtPlugin>

#include <vgCheckArg.h>

#include <vsStaticSourceFactory.h>

#include "vsPvoDescriptorArchiveSource.h"

Q_EXPORT_PLUGIN2(vsPvoArchiveSource, vsPvoArchiveSourcePlugin)

//-----------------------------------------------------------------------------
vsPvoArchiveSourcePlugin::vsPvoArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsPvoArchiveSourcePlugin::~vsPvoArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsArchiveSourceTypes vsPvoArchiveSourcePlugin::archiveTypes() const
{
  return vs::ArchiveDescriptorSource;
}

//-----------------------------------------------------------------------------
vsArchivePluginInfo vsPvoArchiveSourcePlugin::archivePluginInfo(
  vsArchiveSourceType type) const
{
  vsArchivePluginInfo info;
  if (type == vs::ArchiveDescriptorSource)
    {
    vsArchiveFileType fileType;
    fileType.Description = "Kitware collected P/V/O's";
    fileType.Patterns.append("*.pvo.txt");
    info.SupportedFileTypes.append(fileType);
    }
  return info;
}

//-----------------------------------------------------------------------------
vsSimpleSourceFactory* vsPvoArchiveSourcePlugin::createArchiveSource(
  vsArchiveSourceType type, const QUrl& uri, SourceCreateMode mode)
{
  // Quick test for acceptable archive
  if (mode == vsArchiveSourceInterface::QuickTest)
    {
    CHECK_ARG(type == vs::ArchiveDescriptorSource, 0);
    CHECK_ARG(uri.scheme() == "file", 0);
    CHECK_ARG(uri.toLocalFile().endsWith(".pvo.txt"), 0);
    }

  // Verify that file exists and is readable
  const QString fileName = uri.toLocalFile();
  CHECK_ARG(QFileInfo(fileName).exists(), 0);
  QFile file(fileName);
  CHECK_ARG(file.open(QIODevice::ReadOnly | QIODevice::Text), 0);

  // Try to verify file contents
  CHECK_ARG(!file.atEnd(), 0);
  const QString line = QString::fromUtf8(file.readLine());
  const QStringList fields = line.split(" ", QString::SkipEmptyParts);
  CHECK_ARG(fields.count() >= 4, 0); // Do we have enough fields?
  bool isIdOkay;
  fields[0].toInt(&isIdOkay);
  CHECK_ARG(isIdOkay, 0); // Does the first field look like an integer?

  // If we get this far, accept the file
  vsStaticSourceFactory* factory = new vsStaticSourceFactory;
  factory->addDescriptorSource(new vsPvoDescriptorArchiveSource(uri));
  return factory;
}
