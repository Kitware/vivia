// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgTime_h
#define __vgTime_h

#include "vgTimeBase.h"

// NOTE: This class is not intended to be used directly. See vgUnixTime.h for
// a usable specialization of this template.

template <typename TimeTraits>
class vgTime : public vgTimeBase
{
public:
  vgTime(const vgTime& other) : vgTimeBase(other) {}

  explicit vgTime(qint64 microseconds) :
    vgTimeBase(microseconds + TimeTraits::EpochOffset) {}

  explicit vgTime(const QDateTime& dateTime) : vgTimeBase(dateTime) {}

  inline vgTime(double time, double unitsPerSecond = 1e6)
    : vgTimeBase(fromDouble(time, unitsPerSecond)) {}

  inline qint64 toInt64() const;
  inline double toDouble(double unitsPerSecond) const;

  inline vgTime fromDouble(double time, double unitsPerSecond = 1e6);
};

//-----------------------------------------------------------------------------
template <typename TimeTraits>
vgTime<TimeTraits> vgTime<TimeTraits>::fromDouble(
  double time, double unitsPerSecond)
{
  if (!qFuzzyCompare(unitsPerSecond, 1e6))
    {
    time *= 1e6 / unitsPerSecond;
    }
  return vgTime(qRound64(time));
}

//-----------------------------------------------------------------------------
template <typename TimeTraits>
qint64 vgTime<TimeTraits>::toInt64() const
{
  return this->unixTime - TimeTraits::EpochOffset;
}

//-----------------------------------------------------------------------------
template <typename TimeTraits>
double vgTime<TimeTraits>::toDouble(double unitsPerSecond) const
{
  double time = this->toInt64();
  if (!qFuzzyCompare(unitsPerSecond, 1e6))
    {
    time *= unitsPerSecond / 1e6;
    }
  return time;
}

#endif
