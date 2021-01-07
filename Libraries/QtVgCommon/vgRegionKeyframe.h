// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgRegionKeyframe_h
#define __vgRegionKeyframe_h

#include <QList>
#include <QRect>
#include <QString>

#include <vgExport.h>

#include <vgTimeStamp.h>

struct QTVG_COMMON_EXPORT vgRegionKeyframe
{
  vgTimeStamp Time;
  QRect Region;

  static QList<vgRegionKeyframe> readFromFile(QString fileName);
};

#endif
