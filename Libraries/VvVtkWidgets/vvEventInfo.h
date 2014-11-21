/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
