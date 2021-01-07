// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
