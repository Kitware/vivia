// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfArchiveSourceInfo_h
#define __vdfArchiveSourceInfo_h

#include <QStringList>

//-----------------------------------------------------------------------------
struct vdfArchiveFileType
{
  QString Description;
  QStringList Patterns;
};

//-----------------------------------------------------------------------------
struct vdfArchivePluginInfo
{
  QList<vdfArchiveFileType> SupportedFileTypes;
};

#endif
