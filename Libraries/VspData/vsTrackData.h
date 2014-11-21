/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackData_h
#define __vsTrackData_h

#include <vtkVgTimeStamp.h>

#include <QString>
#include <QHash>

#include <map>

typedef std::map<vtkVgTimeStamp, double> vsTrackDataItem;
typedef QHash<QString, vsTrackDataItem> vsTrackData;

#endif
