// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsKwaArchiveSourcePlugin.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QUrl>
#include <QtPlugin>

#include <vgCheckArg.h>

#include <vsStaticSourceFactory.h>

#include "vsKwaVideoArchiveSource.h"

//-----------------------------------------------------------------------------
vsKwaArchiveSourcePlugin::vsKwaArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsKwaArchiveSourcePlugin::~vsKwaArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsArchiveSourceTypes vsKwaArchiveSourcePlugin::archiveTypes() const
{
  return vs::ArchiveVideoSource;
}

//-----------------------------------------------------------------------------
vsArchivePluginInfo vsKwaArchiveSourcePlugin::archivePluginInfo(
  vsArchiveSourceType type) const
{
  vsArchivePluginInfo info;
  if (type == vs::ArchiveVideoSource)
    {
    vsArchiveFileType fileType;
    fileType.Description = "Kitware video archive";
    fileType.Patterns.append("*.index");
    info.SupportedFileTypes.append(fileType);
    }
  return info;
}

//-----------------------------------------------------------------------------
vsSimpleSourceFactory* vsKwaArchiveSourcePlugin::createArchiveSource(
  vsArchiveSourceType type, const QUrl& uri, SourceCreateMode mode)
{
  // Quick test for acceptable archive
  if (mode == vsArchiveSourceInterface::QuickTest)
    {
    CHECK_ARG(type == vs::ArchiveVideoSource, 0);
    CHECK_ARG(uri.scheme() == "file", 0);
    CHECK_ARG(uri.toLocalFile().endsWith(".index"), 0);
    }

  // Verify that file exists and is readable
  const QString fileName = uri.toLocalFile();
  CHECK_ARG(QFileInfo(fileName).exists(), 0);
  QFile file(fileName);
  CHECK_ARG(file.open(QIODevice::ReadOnly | QIODevice::Text), 0);

  // Try to verify file contents (this is a REALLY loose test)
  CHECK_ARG(!file.atEnd(), 0);
  const QByteArray line = file.readLine().trimmed();
  bool isVersionOkay;
  line.toInt(&isVersionOkay);
  CHECK_ARG(isVersionOkay, 0); // Does the first line look like an integer?

  // If we get this far, accept the file
  vsStaticSourceFactory* factory = new vsStaticSourceFactory;
  factory->setVideoSource(new vsKwaVideoArchiveSource(uri));
  return factory;
}
