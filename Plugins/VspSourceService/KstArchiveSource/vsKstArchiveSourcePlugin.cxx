/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsKstArchiveSourcePlugin.h"

#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include <QtPlugin>

#include <vgCheckArg.h>

#include <vsStaticSourceFactory.h>

#include "vsKstDescriptorArchiveSource.h"

Q_EXPORT_PLUGIN2(vsKstArchiveSource, vsKstArchiveSourcePlugin)

namespace // anonymous
{

//-----------------------------------------------------------------------------
bool regexMatch(const QRegExp& re, const QString& str)
{
  return re.indexIn(str) >= 0;
}

//-----------------------------------------------------------------------------
bool regexMatch(const QString& re, const QString& str)
{
  return regexMatch(QRegExp(re), str);
}

//-----------------------------------------------------------------------------
bool regexMatchHeader(const QString& reHeader, const QString& str)
{
  return regexMatch(QRegExp(reHeader + "\\s*[,;]"), str);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vsKstArchiveSourcePlugin::vsKstArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsKstArchiveSourcePlugin::~vsKstArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsArchiveSourceTypes vsKstArchiveSourcePlugin::archiveTypes() const
{
  // TODO: support tracks
  return vs::ArchiveDescriptorSource;
}

//-----------------------------------------------------------------------------
vsArchivePluginInfo vsKstArchiveSourcePlugin::archivePluginInfo(
  vsArchiveSourceType type) const
{
  // TODO: support tracks, raw descriptors
  vsArchivePluginInfo info;
  if (type == vs::ArchiveDescriptorSource)
    {
    vsArchiveFileType fileType;
    fileType.Description = "Kitware saved result sets";
    fileType.Patterns.append("*.vsr");
    fileType.Patterns.append("*.vsrx");
    fileType.Patterns.append("*.vsr.xml");
    info.SupportedFileTypes.append(fileType);
    }
  return info;
}

//-----------------------------------------------------------------------------
vsSimpleSourceFactory* vsKstArchiveSourcePlugin::createArchiveSource(
  vsArchiveSourceType type, const QUrl& uri, SourceCreateMode mode)
{
  // Quick test for acceptable archive
  if (mode == vsArchiveSourceInterface::QuickTest)
    {
    CHECK_ARG(type == vs::ArchiveDescriptorSource, 0);
    CHECK_ARG(uri.scheme() == "file", 0);
    CHECK_ARG(regexMatch(uri.toLocalFile(), "\\.vsr(x|\\.xml)?$"), 0);
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
    const QString line = QString::fromLocal8Bit(file.readLine()).trimmed();
    if (!line.startsWith("#"))
      {
      // Not a comment; check for valid header
      // TODO: support tracks, raw descriptors
      CHECK_ARG(regexMatchHeader("EVENT_META", line), 0);

      // If we get this far, accept the file
      break;
      }
    }

  vsStaticSourceFactory* factory = new vsStaticSourceFactory;
  factory->addDescriptorSource(new vsKstDescriptorArchiveSource(uri));
  return factory;
}
