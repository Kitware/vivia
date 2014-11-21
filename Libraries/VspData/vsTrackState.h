/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackState_h
#define __vsTrackState_h

#include <QPointF>
#include <QPolygon>
#include <QPolygonF>

#include <vvTrack.h>

#include <vtkVgTimeStamp.h>

struct vsTrackState
{
  vtkVgTimeStamp time;
  QPointF point;
  QPolygonF object;

  bool operator<(const vvTrackState& other) const
    { return this->time < vtkVgTimeStamp(other.TimeStamp); }
  bool operator<(const vtkVgTimeStamp& timestamp) const
    { return this->time < timestamp; }
  bool operator>(const vtkVgTimeStamp& timestamp) const
    { return timestamp < this->time; }
};

#endif
