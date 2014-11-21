/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackInfo_h
#define __vsTrackInfo_h

#include <QString>
#include <QList>

#include <vgExport.h>

struct VSP_DATA_EXPORT vsTrackInfo
{
  int id;
  int source;
  int type;
  QString name;
  double pcolor[3];
  double bcolor[3];
  double fcolor[3];

  enum Type
    {
    Person       = -5001,
    Vehicle      = -5002,
    Other        = -5003,
    Unclassified = -5000
    };

public:
  static const int GroundTruthSource;

  static QList<vsTrackInfo> trackTypes();
  static QList<vsTrackInfo> trackSources();
};

#endif
