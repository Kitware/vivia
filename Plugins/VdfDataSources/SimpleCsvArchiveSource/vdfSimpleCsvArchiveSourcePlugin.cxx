/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfSimpleCsvArchiveSourcePlugin.h"

#include "vdfSimpleCsvTrackArchiveSource.h"

#include <vgCheckArg.h>

#include <QFile>
#include <QFileInfo>
#include <QSharedPointer>
#include <QUrl>
#include <QtPlugin>

Q_EXPORT_PLUGIN2(vdfSimpleCsvArchiveSource,
                 vdfSimpleCsvArchiveSourcePlugin)

//-----------------------------------------------------------------------------
vdfSimpleCsvArchiveSourcePlugin::vdfSimpleCsvArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vdfSimpleCsvArchiveSourcePlugin::~vdfSimpleCsvArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vdfArchivePluginInfo
vdfSimpleCsvArchiveSourcePlugin::archivePluginInfo() const
{
  vdfArchivePluginInfo info;

  vdfArchiveFileType fileType;
  fileType.Description = "Comma-Separated Value files";
  fileType.Patterns.append("*.csv");
  info.SupportedFileTypes.append(fileType);

  return info;
}

//-----------------------------------------------------------------------------
vdfDataSource* vdfSimpleCsvArchiveSourcePlugin::createArchiveSource(
  const QUrl& uri, SourceCreateMode mode)
{
  // Quick test for acceptable archive
  if (mode == vdfArchiveSourceInterface::QuickTest)
    {
    CHECK_ARG(uri.scheme() == "file", nullptr);
    CHECK_ARG(QFileInfo{uri.toLocalFile()}.suffix() == "csv", nullptr);
    }

  // Verify that file exists and is readable
  const QString fileName = uri.toLocalFile();
  CHECK_ARG(QFileInfo(fileName).exists(), nullptr);
  QSharedPointer<QFile> file{new QFile{fileName}};
  CHECK_ARG(file->open(QIODevice::ReadOnly | QIODevice::Text), nullptr);

  return new vdfSimpleCsvTrackDataSource{uri, file};
}
