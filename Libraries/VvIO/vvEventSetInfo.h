// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
