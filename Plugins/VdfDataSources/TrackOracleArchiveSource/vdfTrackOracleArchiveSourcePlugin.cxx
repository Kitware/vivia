/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTrackOracleArchiveSourcePlugin.h"

#include "vdfTrackOracleTrackDataSource.h"
#ifdef KWIVER_TRACK_ORACLE
#include "vdfTrackOracleEventDataSource.h"
#endif

#ifdef KWIVER_TRACK_ORACLE
#include <visgui_event_type.h>
#endif

#include <visgui_track_type.h>

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

QTE_IMPLEMENT_D_FUNC(vdfTrackOracleArchiveSourcePlugin)

namespace // anonymous
{

using ArchiveSourceConstructor =
  vdfDataSource* (*)(const QUrl&, track_oracle::file_format_base*);

//-----------------------------------------------------------------------------
template <typename T>
vdfDataSource* createArchiveSource(
  const QUrl& uri, track_oracle::file_format_base* format)
{
  return new T{uri, format};
}

//-----------------------------------------------------------------------------
struct FormatInfo
{
  FormatInfo() = default;
  FormatInfo(track_oracle::file_format_base* of, ArchiveSourceConstructor sc)
    : OracleFormat{of}, SourceConstructor{sc} {}

  vdfDataSource* createArchiveSource(const QUrl& uri) const
  {
    return (*this->SourceConstructor)(uri, this->OracleFormat);
  }

  track_oracle::file_format_base* OracleFormat = nullptr;
  ArchiveSourceConstructor SourceConstructor = nullptr;
};

using FileFormatMap = QMap<track_oracle::file_format_enum, FormatInfo>;

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vdfTrackOracleArchiveSourcePluginPrivate
{
public:
  void addSchemas(const track_oracle::track_base_impl& schema,
                  ArchiveSourceConstructor sourceConstructor);

  bool quickTest(const std::string& fileName) const;
  FormatInfo inspect(const std::string& fileName) const;

  FileFormatMap Formats;
};

//-----------------------------------------------------------------------------
void vdfTrackOracleArchiveSourcePluginPrivate::addSchemas(
  const track_oracle::track_base_impl& schema,
  ArchiveSourceConstructor sourceConstructor)
{
  const auto& formats =
    track_oracle::file_format_manager::format_matches_schema(schema);
  for (const auto& format : formats)
  {
    auto* const format_instance =
      track_oracle::file_format_manager::get_format(format);
    if (format_instance)
    {
      this->Formats.insert(format, {format_instance, sourceConstructor});
    }
  }
}

//-----------------------------------------------------------------------------
bool vdfTrackOracleArchiveSourcePluginPrivate::quickTest(
  const std::string& fileName) const
{
  for (const auto& format : this->Formats)
  {
    if (format.OracleFormat->filename_matches_globs(fileName))
    {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
FormatInfo vdfTrackOracleArchiveSourcePluginPrivate::inspect(
  const std::string& fileName) const
{
  for (const auto& format : this->Formats)
  {
    if (format.OracleFormat->inspect_file(fileName))
    {
      return format;
    }
  }
  return {};
}

//-----------------------------------------------------------------------------
vdfTrackOracleArchiveSourcePlugin::vdfTrackOracleArchiveSourcePlugin() :
  d_ptr(new vdfTrackOracleArchiveSourcePluginPrivate)
{
  QTE_D();

  /* FIXME
  // kwiver and csv are open-ended and should always be tried
  d->Formats.insert(
    track_oracle::TF_CSV,
    track_oracle::file_format_manager::get_format(track_oracle::TF_CSV));
  d->Formats.insert(
    track_oracle::TF_KWIVER,
    track_oracle::file_format_manager::get_format(track_oracle::TF_KWIVER));
  */

  d->addSchemas(visgui_minimal_track_type{},
                &::createArchiveSource<vdfTrackOracleTrackDataSource>);
#ifdef KWIVER_TRACK_ORACLE
  d->addSchemas(visgui_minimal_event_type{},
                &::createArchiveSource<vdfTrackOracleEventDataSource>);
#endif
}

//-----------------------------------------------------------------------------
vdfTrackOracleArchiveSourcePlugin::~vdfTrackOracleArchiveSourcePlugin()
{
}

//-----------------------------------------------------------------------------
vdfArchivePluginInfo
vdfTrackOracleArchiveSourcePlugin::archivePluginInfo() const
{
  QTE_D();

  vdfArchivePluginInfo info;

  for (const auto& format : d->Formats)
  {
    vdfArchiveFileType fileType;
    fileType.Description = qtString(format.OracleFormat->format_description());
    std::vector<std::string> globs = format.OracleFormat->format_globs();
    for (auto const n : qtIndexRange(globs.size()))
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
  QTE_D();

  // Quick test for acceptable archive
  if (mode == vdfArchiveSourceInterface::QuickTest)
  {
    CHECK_ARG(uri.scheme() == "file", nullptr);
    CHECK_ARG(d->quickTest(stdString(uri.toLocalFile())), nullptr);
  }

  // Verify that file exists and is readable
  const QString fileName = uri.toLocalFile();
  CHECK_ARG(QFileInfo(fileName).exists(), nullptr);
  QFile file(fileName);
  CHECK_ARG(file.open(QIODevice::ReadOnly | QIODevice::Text), nullptr);

  const auto& format = d->inspect(stdString(fileName));
  CHECK_ARG(format.SourceConstructor, nullptr);

  return format.createArchiveSource(uri);
}
