/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
