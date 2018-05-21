/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTrackOracleArchiveSourcePlugin.h"

#include "vdfTrackOracleTrackArchiveSource.h"
#include "visgui_track_type.h"

#include <vgCheckArg.h>

#include <qtStlUtil.h>

#ifdef KWIVER_TRACK_ORACLE
#include <track_oracle/file_formats/file_format_base.h>
#include <track_oracle/file_formats/file_format_manager.h>
#else
#include <track_oracle/file_format_base.h>
#include <track_oracle/file_format_manager.h>
#endif

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QStringList>
#include <QUrl>
#include <QtPlugin>

Q_EXPORT_PLUGIN2(vdfTrackOracleArchiveSource,
                 vdfTrackOracleArchiveSourcePlugin)

QTE_IMPLEMENT_D_FUNC(vdfTrackOracleArchiveSourcePlugin)

namespace // anonymous
{
using FileFormatMap =
  QMap<track_oracle::file_format_enum, track_oracle::file_format_base*>;
}

//-----------------------------------------------------------------------------
class vdfTrackOracleArchiveSourcePluginPrivate
{
public:
  void addSchemas(FileFormatMap& map,
                  const track_oracle::track_base_impl& schema);

  bool quickTest(const std::string& fileName) const;
  track_oracle::file_format_base* inspect(const std::string& fileName) const;

  FileFormatMap Formats;
};

//-----------------------------------------------------------------------------
void vdfTrackOracleArchiveSourcePluginPrivate::addSchemas(
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
bool vdfTrackOracleArchiveSourcePluginPrivate::quickTest(
  const std::string& fileName) const
{
  foreach (auto* const format, this->Formats)
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
vdfTrackOracleArchiveSourcePluginPrivate::inspect(
  const std::string& fileName) const
{
  foreach (auto* const format, this->Formats)
    {
    if (format->inspect_file(fileName))
      {
      return format;
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
vdfTrackOracleArchiveSourcePlugin::vdfTrackOracleArchiveSourcePlugin() :
  d_ptr(new vdfTrackOracleArchiveSourcePluginPrivate)
{
  QTE_D(vdfTrackOracleArchiveSourcePlugin);

  d->addSchemas(d->Formats, visgui_minimal_track_type());
}

//-----------------------------------------------------------------------------
vdfTrackOracleArchiveSourcePlugin::~vdfTrackOracleArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vdfArchivePluginInfo
vdfTrackOracleArchiveSourcePlugin::archivePluginInfo() const
{
  QTE_D_CONST(vdfTrackOracleArchiveSourcePlugin);

  vdfArchivePluginInfo info;

  foreach (auto* const format, d->Formats)
    {
    vdfArchiveFileType fileType;
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
vdfDataSource* vdfTrackOracleArchiveSourcePlugin::createArchiveSource(
  const QUrl& uri, SourceCreateMode mode)
{
  QTE_D_CONST(vdfTrackOracleArchiveSourcePlugin);

  // Quick test for acceptable archive
  if (mode == vdfArchiveSourceInterface::QuickTest)
    {
    CHECK_ARG(uri.scheme() == "file", 0);
    CHECK_ARG(d->quickTest(stdString(uri.toLocalFile())), 0);
    }

  // Verify that file exists and is readable
  const QString fileName = uri.toLocalFile();
  CHECK_ARG(QFileInfo(fileName).exists(), 0);
  QFile file(fileName);
  CHECK_ARG(file.open(QIODevice::ReadOnly | QIODevice::Text), 0);

  auto* const format_instance = d->inspect(stdString(fileName));

  return new vdfTrackOracleTrackDataSource(uri, format_instance);
}
