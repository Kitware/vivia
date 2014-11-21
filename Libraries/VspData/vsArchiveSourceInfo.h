/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
