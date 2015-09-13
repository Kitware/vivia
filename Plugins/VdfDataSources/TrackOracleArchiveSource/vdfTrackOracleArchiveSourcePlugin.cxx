/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTrackOracleArchiveSourcePlugin.h"

#include "vdfTrackOracleTrackArchiveSource.h"
#include "visgui_track_type.h"

#include <vcl_string.h>

#include <vgCheckArg.h>

#include <qtStlUtil.h>


#include <track_oracle/file_format_base.h>
#include <track_oracle/file_format_manager.h>

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
typedef QMap<vidtk::file_format_enum, vidtk::file_format_base*> FileFormatMap;
}

//-----------------------------------------------------------------------------
class vdfTrackOracleArchiveSourcePluginPrivate
{
public:
  void addSchemas(FileFormatMap& map, const vidtk::track_base_impl& schema);

  bool quickTest(const std::string& fileName) const;
  vidtk::file_format_base* inspect(const std::string& fileName) const;

  FileFormatMap Formats;
};

//-----------------------------------------------------------------------------
void vdfTrackOracleArchiveSourcePluginPrivate::addSchemas(
  FileFormatMap& map, const vidtk::track_base_impl& schema)
{
  vcl_vector<vidtk::file_format_enum> formats =
    vidtk::file_format_manager::format_matches_schema(schema);
  for (size_t n = 0, k = formats.size(); n < k; ++n)
    {
    vidtk::file_format_enum format = formats[n];
    vidtk::file_format_base* format_instance =
      vidtk::file_format_manager::get_format(format);
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
  foreach (vidtk::file_format_base* format, this->Formats)
    {
    if (format->filename_matches_globs(fileName))
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
vidtk::file_format_base* vdfTrackOracleArchiveSourcePluginPrivate::inspect(
  const std::string& fileName) const
{
  foreach (vidtk::file_format_base* format, this->Formats)
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

  foreach (vidtk::file_format_base* format, d->Formats)
    {
    vdfArchiveFileType fileType;
    fileType.Description = qtString(format->format_description());
    vcl_vector<vcl_string> globs = format->format_globs();
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

  vidtk::file_format_base* format_instance = d->inspect(stdString(fileName));

  return new vdfTrackOracleTrackDataSource(uri, format_instance);
}
