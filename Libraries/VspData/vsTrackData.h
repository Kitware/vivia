// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsTrackData_h
#define __vsTrackData_h

#include <vtkVgTimeStamp.h>

#include <QString>
#include <QHash>

#include <map>

typedef std::map<vtkVgTimeStamp, double> vsTrackDataItem;
typedef QHash<QString, vsTrackDataItem> vsTrackData;

#endif
