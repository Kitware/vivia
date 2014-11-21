/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
