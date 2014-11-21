/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvEventSetInfo_h
#define __vvEventSetInfo_h

#include <QColor>
#include <QString>

//-----------------------------------------------------------------------------
struct vvEventSetInfo
{
  vvEventSetInfo() : DisplayThreshold(0.0) {}

  QString Name;
  QColor PenColor;
  QColor BackgroundColor;
  QColor ForegroundColor;
  double DisplayThreshold;
};

#endif
