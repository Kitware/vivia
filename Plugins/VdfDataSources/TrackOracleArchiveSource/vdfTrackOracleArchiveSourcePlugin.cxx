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

#include <vdfEventSource.h>
#include <vdfTrackSource.h>

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
  void addFormat(
    const track_oracle::file_format_enum& format, const QMetaObject* interface,
    ArchiveSourceConstructor sourceConstructor);
  void addSchemas(
    const track_oracle::track_base_impl& schema, const QMetaObject* interface,
    ArchiveSourceConstructor sourceConstructor);

  bool quickTest(
    const std::string& fileName,
    const QList<const QMetaObject*>& desiredInterfaces) const;
  bool quickTest(
    const std::string& fileName,
    const FileFormatMap& candidateFormats) const;
  FormatInfo inspect(
    const std::string& fileName,
    const QList<const QMetaObject*>& desiredInterfaces) const;
  FormatInfo inspect(
    const std::string& fileName,
    const FileFormatMap& candidateFormats) const;

  FileFormatMap Formats;
  QHash<QMetaObject const*, FileFormatMap> FormatsByInterface;
};

//-----------------------------------------------------------------------------
void vdfTrackOracleArchiveSourcePluginPrivate::addFormat(
  const track_oracle::file_format_enum& format, const QMetaObject* interface,
  ArchiveSourceConstructor sourceConstructor)
{
  auto* const format_instance =
    track_oracle::file_format_manager::get_format(format);
  if (format_instance)
  {
    const auto formatInfo = FormatInfo{format_instance, sourceConstructor};
    this->FormatsByInterface[interface].insert(format, formatInfo);
    this->Formats.insert(format, formatInfo);
  }
}

//-----------------------------------------------------------------------------
void vdfTrackOracleArchiveSourcePluginPrivate::addSchemas(
  const track_oracle::track_base_impl& schema, const QMetaObject* interface,
  ArchiveSourceConstructor sourceConstructor)
{
  const auto& formats =
    track_oracle::file_format_manager::format_matches_schema(schema);
  for (const auto& format : formats)
  {
    this->addFormat(format, interface, sourceConstructor);
  }
}

//-----------------------------------------------------------------------------
bool vdfTrackOracleArchiveSourcePluginPrivate::quickTest(
  const std::string& fileName,
  const QList<const QMetaObject*>& desiredInterfaces) const
{
  if (desiredInterfaces.isEmpty())
  {
    return this->quickTest(fileName, this->Formats);
  }
  else
  {
    for (auto* const interface : desiredInterfaces)
    {
      const auto& candidateFormats =
        this->FormatsByInterface.value(interface);
      if (this->quickTest(fileName, candidateFormats))
      {
        return true;
      }
    }
    return false;
  }
}

//-----------------------------------------------------------------------------
bool vdfTrackOracleArchiveSourcePluginPrivate::quickTest(
  const std::string& fileName, const FileFormatMap& candidateFormats) const
{
  for (const auto& format : candidateFormats)
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
  const std::string& fileName,
  const QList<const QMetaObject*>& desiredInterfaces) const
{
  // First try formats according to the interfaces they provide based on the
  // interfaces desired by the user
  for (auto* const interface : desiredInterfaces)
  {
    const auto& candidateFormats =
      this->FormatsByInterface.value(interface);
    const auto& result = this->inspect(fileName, candidateFormats);

    // Did we find something?
    if (result.OracleFormat)
    {
      // Yes; return that
      return result;
    }
  }

  // If we didn't find a format that provides the desired interface, or if the
  // user didn't specify any desired interfaces, try everything
  return this->inspect(fileName, this->Formats);
}

//-----------------------------------------------------------------------------
FormatInfo vdfTrackOracleArchiveSourcePluginPrivate::inspect(
  const std::string& fileName, const FileFormatMap& candidateFormats) const
{
  for (const auto& format : candidateFormats)
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

  // KWIVER and CSV are open-ended and should always be tried
  d->addFormat(track_oracle::TF_CSV, &vdfTrackSource::staticMetaObject,
               &::createArchiveSource<vdfTrackOracleTrackDataSource>);
  d->addFormat(track_oracle::TF_KWIVER, &vdfTrackSource::staticMetaObject,
               &::createArchiveSource<vdfTrackOracleTrackDataSource>);
#ifdef KWIVER_TRACK_ORACLE
  d->addFormat(track_oracle::TF_CSV, &vdfEventSource::staticMetaObject,
               &::createArchiveSource<vdfTrackOracleEventDataSource>);
  d->addFormat(track_oracle::TF_KWIVER, &vdfEventSource::staticMetaObject,
               &::createArchiveSource<vdfTrackOracleEventDataSource>);
#endif

  // Find additional formats that support our schemas and add them
  d->addSchemas(visgui_minimal_track_type_with_time_usecs{},
                &vdfTrackSource::staticMetaObject,
                &::createArchiveSource<vdfTrackOracleTrackDataSource>);
  d->addSchemas(visgui_minimal_track_type_with_frame_number{},
                &vdfTrackSource::staticMetaObject,
                &::createArchiveSource<vdfTrackOracleTrackDataSource>);
#ifdef KWIVER_TRACK_ORACLE
  d->addSchemas(visgui_minimal_event_type{},
                &vdfEventSource::staticMetaObject,
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
  const QUrl& uri, const QList<const QMetaObject*>& desiredInterfaces,
  SourceCreateMode mode)
{
  QTE_D();

  const auto& fileName = uri.toLocalFile();
  const auto& stdFileName = stdString(fileName);

  // Quick test for acceptable archive
  if (mode == vdfArchiveSourceInterface::QuickTest)
  {
    CHECK_ARG(uri.scheme() == "file", nullptr);
    CHECK_ARG(d->quickTest(stdFileName, desiredInterfaces), nullptr);
  }

  // Verify that file exists and is readable
  CHECK_ARG(QFileInfo::exists(fileName), nullptr);
  QFile file(fileName);
  CHECK_ARG(file.open(QIODevice::ReadOnly | QIODevice::Text), nullptr);

  const auto& format = d->inspect(stdFileName, desiredInterfaces);
  CHECK_ARG(format.SourceConstructor, nullptr);

  return format.createArchiveSource(uri);
}
