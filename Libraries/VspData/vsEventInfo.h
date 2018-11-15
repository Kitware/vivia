/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsEventInfo_h
#define __vsEventInfo_h

#include <QString>
#include <QList>

#include <vgExport.h>

class vgEventType;

struct vvEventSetInfo;

struct VSP_DATA_EXPORT vsEventInfo
{
  int type;
  QString name;
  double pcolor[3];
  double bcolor[3];
  double fcolor[3];

  enum Group
    {
    Unknown     = 0x0,
    Fish        = 0x1,
    Scallop     = 0x2,
    Classifier  = 0xf,
    Alert       = 0x10,
    General     = 0x20,
    User        = 0x80,
    NonUser     = 0x7f,
    All         = 0xff
    };
  Q_DECLARE_FLAGS(Groups, Group)

  enum Type
    {
    Tripwire        = -3000,
    EnteringRegion  = -3001,
    ExitingRegion   = -3002,
    Annotation      = -4000,
    // NOTE: -5000 - -5??? reserved by vsTrackInfo
    QueryAlert      = -10000,
    UserType        = -20000
    };

public:
  vgEventType toVgEventType() const;

  static vsEventInfo fromEventSetInfo(const vvEventSetInfo&, int type = -1);

  static QList<vsEventInfo> events(Groups = All);
  static Group eventGroup(int eventType);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vsEventInfo::Groups)

#endif
