/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsViperArchiveSourcePlugin.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QUrl>
#include <QXmlStreamReader>
#include <QtPlugin>

#include <vgCheckArg.h>

#include <vsStaticSourceFactory.h>

// Need this so that QSharedPointer can find the dtor
#include <vsTrackSource.h>

#include "vsViperArchiveSource.h"

//-----------------------------------------------------------------------------
vsViperArchiveSourcePlugin::vsViperArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsViperArchiveSourcePlugin::~vsViperArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsArchiveSourceTypes vsViperArchiveSourcePlugin::archiveTypes() const
{
  return vs::ArchiveTrackSource;
}

//-----------------------------------------------------------------------------
vsArchivePluginInfo vsViperArchiveSourcePlugin::archivePluginInfo(
  vsArchiveSourceType type) const
{
  vsArchivePluginInfo info;
  if (type == vs::ArchiveTrackSource)
    {
    vsArchiveFileType fileType;
    fileType.Description = "ViPER ground truth";
    fileType.Patterns.append("*.xgtf");
    info.SupportedFileTypes.append(fileType);
    }
  return info;
}

//-----------------------------------------------------------------------------
#include <QDebug>
vsSimpleSourceFactory* vsViperArchiveSourcePlugin::createArchiveSource(
  vsArchiveSourceType type, const QUrl& uri, SourceCreateMode mode)
{
  // Quick test for acceptable archive
  if (mode == vsArchiveSourceInterface::QuickTest)
    {
    CHECK_ARG(type == vs::ArchiveTrackSource, 0);
    CHECK_ARG(uri.scheme() == "file", 0);
    CHECK_ARG(uri.toLocalFile().endsWith(".xgtf"), 0);
    }

  // Verify that file exists and is readable
  const QString fileName = uri.toLocalFile();
  CHECK_ARG(QFileInfo(fileName).exists(), 0);
  QFile file(fileName);
  CHECK_ARG(file.open(QIODevice::ReadOnly | QIODevice::Text), 0);
  QXmlStreamReader stream(&file);

  // Try to verify file contents
  forever
    {
    CHECK_ARG(!stream.atEnd(), 0);
    CHECK_ARG(stream.readNextStartElement(), 0);
    qDebug() << stream.name();

    if (stream.name() == "viper")
      {
      // This looks like a valid header; accept the file
      break;
      }
    }

  vsStaticSourceFactory* factory = new vsStaticSourceFactory;
  vsViperArchiveSource* source = new vsViperArchiveSource(uri);
  factory->addTrackSource(source->trackSource());
  factory->addDescriptorSource(source);
  return factory;
}
