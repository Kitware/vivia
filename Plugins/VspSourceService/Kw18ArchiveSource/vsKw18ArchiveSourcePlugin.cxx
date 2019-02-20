/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsKw18ArchiveSourcePlugin.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QUrl>
#include <QtPlugin>

#include <vgCheckArg.h>

#include <vsStaticSourceFactory.h>

#include "vsKw18TrackArchiveSource.h"

//-----------------------------------------------------------------------------
vsKw18ArchiveSourcePlugin::vsKw18ArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsKw18ArchiveSourcePlugin::~vsKw18ArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsArchiveSourceTypes vsKw18ArchiveSourcePlugin::archiveTypes() const
{
  return vs::ArchiveTrackSource;
}

//-----------------------------------------------------------------------------
vsArchivePluginInfo vsKw18ArchiveSourcePlugin::archivePluginInfo(
  vsArchiveSourceType type) const
{
  vsArchivePluginInfo info;
  if (type == vs::ArchiveTrackSource)
    {
    vsArchiveFileType fileType;
    fileType.Description = "Kitware CSV tracks";
    fileType.Patterns.append("*.kw18");
    info.SupportedFileTypes.append(fileType);
    }
  return info;
}

//-----------------------------------------------------------------------------
vsSimpleSourceFactory* vsKw18ArchiveSourcePlugin::createArchiveSource(
  vsArchiveSourceType type, const QUrl& uri, SourceCreateMode mode)
{
  // Quick test for acceptable archive
  if (mode == vsArchiveSourceInterface::QuickTest)
    {
    CHECK_ARG(type == vs::ArchiveTrackSource, 0);
    CHECK_ARG(uri.scheme() == "file", 0);
    CHECK_ARG(uri.toLocalFile().endsWith(".kw18"), 0);
    }

  // Verify that file exists and is readable
  const QString fileName = uri.toLocalFile();
  CHECK_ARG(QFileInfo(fileName).exists(), 0);
  QFile file(fileName);
  CHECK_ARG(file.open(QIODevice::ReadOnly | QIODevice::Text), 0);

  // Try to verify file contents
  forever
    {
    CHECK_ARG(!file.atEnd(), 0);
    const QString line = QString::fromUtf8(file.readLine());
    if (line.startsWith("# 1:Track-id"))
      {
      // This looks like a valid header; accept the file
      break;
      }
    if (!line.startsWith("#"))
      {
      // Not a comment; test if it looks like a valid record
      const QStringList fields = line.split(" ", QString::SkipEmptyParts);
      CHECK_ARG(fields.count() >= 18, 0); // Do we have enough fields?
      bool isIdOkay;
      fields[0].toInt(&isIdOkay);
      CHECK_ARG(isIdOkay, 0); // Does the first field look like an integer?

      // If we get this far, accept the file
      break;
      }
    }

  vsStaticSourceFactory* factory = new vsStaticSourceFactory;
  factory->addTrackSource(new vsKw18TrackArchiveSource(uri));
  return factory;
}
