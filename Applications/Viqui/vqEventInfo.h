/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqEventInfo_h
#define __vqEventInfo_h

#include <QList>
#include <QSet>
#include <QString>

struct vqEventInfo
{
  int Type;
  QString Name;

  enum Group
    {
    Fish      = 0x1,
    Scallop     = 0x2,
    All         = 0xff
    };
  Q_DECLARE_FLAGS(Groups, Group)

  static QList<vqEventInfo> types(Groups = All);
  static QSet<int> searchableTypes();
  static int staticTypeArraySize();

  static QString name(int type);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vqEventInfo::Groups)

#endif
