// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgTimeBase.h"

#include <QDateTime>
#include <QString>

//-----------------------------------------------------------------------------
vgTimeBase::vgTimeBase(const QDateTime& dateTime) :
  unixTime(dateTime.toMSecsSinceEpoch() * 1000)
{
}

//-----------------------------------------------------------------------------
QDateTime vgTimeBase::toDateTime() const
{
  return QDateTime::fromMSecsSinceEpoch(this->unixTime / 1000);
}

//-----------------------------------------------------------------------------
QString vgTimeBase::dateString() const
{
  return this->toDateTime().toUTC().toString("yyyy-MM-dd");
}

//-----------------------------------------------------------------------------
QString vgTimeBase::timeString() const
{
  return this->toDateTime().toUTC().toString("hh:mm:ss.zzz");
}
