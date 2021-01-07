// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
    Person      = 0x1,
    Vehicle     = 0x2,
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
