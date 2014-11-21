/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
