// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvEventInfo_h
#define __vvEventInfo_h

#include <QString>
#include <QList>

#include <vgExport.h>

#include <vgColor.h>

struct VV_VTKWIDGETS_EXPORT vvEventInfo
{
  int Type;
  QString Name;
  vgColor PenColor;
  vgColor ForegroundColor;
  vgColor BackgroundColor;

  enum Group
    {
    Person      = 0x1,
    Vehicle     = 0x2,
    All         = 0xff
    };
  Q_DECLARE_FLAGS(Groups, Group)

public:
  static QList<vvEventInfo> eventTypes(vvEventInfo::Groups);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vvEventInfo::Groups)

#endif
