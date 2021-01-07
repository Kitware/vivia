// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsArchiveSourceInfo_h
#define __vsArchiveSourceInfo_h

#include <QStringList>

//-----------------------------------------------------------------------------
struct vsArchiveFileType
{
  QString Description;
  QStringList Patterns;
};

//-----------------------------------------------------------------------------
struct vsArchivePluginInfo
{
  QList<vsArchiveFileType> SupportedFileTypes;
};

//-----------------------------------------------------------------------------
namespace vs
{
  enum ArchiveSourceType
    {
    ArchiveVideoSource      = 0x1,
    ArchiveTrackSource      = 0x2,
    ArchiveDescriptorSource = 0x4
    };
  Q_DECLARE_FLAGS(ArchiveSourceTypes, ArchiveSourceType)
}

Q_DECLARE_OPERATORS_FOR_FLAGS(vs::ArchiveSourceTypes)

typedef vs::ArchiveSourceType vsArchiveSourceType;
typedef vs::ArchiveSourceTypes vsArchiveSourceTypes;

#endif
