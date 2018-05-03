/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackOracleArchiveSourcePlugin.h"

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QStringList>
#include <QUrl>
#include <QtPlugin>

#include <qtStlUtil.h>

#include <vgCheckArg.h>

#include <vsStaticSourceFactory.h>

#ifdef KWIVER_TRACK_ORACLE
#include <track_oracle/file_formats/file_format_base.h>
#include <track_oracle/file_formats/file_format_manager.h>
#else
#include <track_oracle/file_format_base.h>
#include <track_oracle/file_format_manager.h>
#endif

#include "visgui_event_type.h"
#include "visgui_track_type.h"
#include "vsTrackOracleTrackArchiveSource.h"
#include "vsTrackOracleDescriptorArchiveSource.h"

Q_EXPORT_PLUGIN2(vsTrackOracleArchiveSource, vsTrackOracleArchiveSourcePlugin)

QTE_IMPLEMENT_D_FUNC(vsTrackOracleArchiveSourcePlugin)

namespace // anonymous
{
using FileFormatMap =
  QMap<track_oracle::file_format_enum, track_oracle::file_format_base*>;
}

//-----------------------------------------------------------------------------
class vsTrackOracleArchiveSourcePluginPrivate
{
public:
  vsTrackOracleArchiveSourcePluginPrivate() : EmptyFormatSet() {}

  void addSchemas(FileFormatMap& map,
                  const track_oracle::track_base_impl& schema);
  const FileFormatMap& formatMap(vsArchiveSourceType type) const;

  bool quickTest(const std::string& fileName, vsArchiveSourceType type) const;
  track_oracle::file_format_base* inspect(const std::string& fileName,
                                          vsArchiveSourceType type) const;

  FileFormatMap TrackFormats;
  FileFormatMap DescriptorFormats;
  const FileFormatMap EmptyFormatSet;
};

//-----------------------------------------------------------------------------
void vsTrackOracleArchiveSourcePluginPrivate::addSchemas(
  FileFormatMap& map, const track_oracle::track_base_impl& schema)
{
  const auto& formats =
    track_oracle::file_format_manager::format_matches_schema(schema);
  for (const auto& format : formats)
    {
    auto* const format_instance =
      track_oracle::file_format_manager::get_format(format);
    if (format_instance)
      {
      map.insert(format, format_instance);
      }
    }
}

//-----------------------------------------------------------------------------
const FileFormatMap& vsTrackOracleArchiveSourcePluginPrivate::formatMap(
  vsArchiveSourceType type) const
{
  switch (type)
    {
    case vs::ArchiveTrackSource:
      return this->TrackFormats;
    case vs::ArchiveDescriptorSource:
      return this->DescriptorFormats;
    default:
      return this->EmptyFormatSet;
    }
}

//-----------------------------------------------------------------------------
bool vsTrackOracleArchiveSourcePluginPrivate::quickTest(
  const std::string& fileName, vsArchiveSourceType type) const
{
  const FileFormatMap& formats = this->formatMap(type);
  foreach (auto* const format, formats)
    {
    if (format->filename_matches_globs(fileName))
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
track_oracle::file_format_base*
vsTrackOracleArchiveSourcePluginPrivate::inspect(
  const std::string& fileName, vsArchiveSourceType type) const
{
  const FileFormatMap& formats = this->formatMap(type);
  foreach (auto* const format, formats)
    {
    if (format->inspect_file(fileName))
      {
      return format;
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
vsTrackOracleArchiveSourcePlugin::vsTrackOracleArchiveSourcePlugin() :
  d_ptr(new vsTrackOracleArchiveSourcePluginPrivate)
{
  QTE_D(vsTrackOracleArchiveSourcePlugin);

  d->addSchemas(d->TrackFormats, visgui_ts_track_type());
  // TODO Once core gets time mapping, also accept tracks with only frame number
  // d->addSchemas(d->TrackFormats, visgui_fn_track_type());

  d->addSchemas(d->DescriptorFormats, visgui_pvo_descriptor_type());
  d->addSchemas(d->DescriptorFormats, visgui_classifier_descriptor_type());
}

//-----------------------------------------------------------------------------
vsTrackOracleArchiveSourcePlugin::~vsTrackOracleArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vsArchiveSourceTypes vsTrackOracleArchiveSourcePlugin::archiveTypes() const
{
  return vs::ArchiveTrackSource | vs::ArchiveDescriptorSource;
}

//-----------------------------------------------------------------------------
vsArchivePluginInfo vsTrackOracleArchiveSourcePlugin::archivePluginInfo(
  vsArchiveSourceType type) const
{
  QTE_D_CONST(vsTrackOracleArchiveSourcePlugin);

  vsArchivePluginInfo info;
  const FileFormatMap& formats = d->formatMap(type);

  foreach (auto* const format, formats)
    {
    vsArchiveFileType fileType;
    fileType.Description = qtString(format->format_description());
    std::vector<std::string> globs = format->format_globs();
    for (size_t n = 0, k = globs.size(); n < k; ++n)
      {
      fileType.Patterns.append(qtString(globs[n]));
      }
    info.SupportedFileTypes.append(fileType);
    }
  return info;
}

//-----------------------------------------------------------------------------
vsSimpleSourceFactory* vsTrackOracleArchiveSourcePlugin::createArchiveSource(
  vsArchiveSourceType type, const QUrl& uri, SourceCreateMode mode)
{
  QTE_D_CONST(vsTrackOracleArchiveSourcePlugin);

  // Quick test for acceptable archive
  if (mode == vsArchiveSourceInterface::QuickTest)
    {
    CHECK_ARG(uri.scheme() == "file", 0);
    CHECK_ARG(d->quickTest(stdString(uri.toLocalFile()), type), 0);
    }

  // Verify that file exists and is readable
  const QString fileName = uri.toLocalFile();
  CHECK_ARG(QFileInfo(fileName).exists(), 0);
  QFile file(fileName);
  CHECK_ARG(file.open(QIODevice::ReadOnly | QIODevice::Text), 0);

  auto* const format_instance = d->inspect(stdString(fileName), type);

  if (type == vs::ArchiveTrackSource)
    {
    vsStaticSourceFactory* factory = new vsStaticSourceFactory;
    vsTrackSource* source =
      new vsTrackOracleTrackArchiveSource(uri, format_instance);
    factory->addTrackSource(source);
    return factory;
    }
  else if (type == vs::ArchiveDescriptorSource)
    {
    vsStaticSourceFactory* factory = new vsStaticSourceFactory;
    vsDescriptorSource* source =
      new vsTrackOracleDescriptorArchiveSource(uri, format_instance);
    factory->addDescriptorSource(source);
    return factory;
    }

  return 0;
}
