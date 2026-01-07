// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgTimeBase_h
#define __vgTimeBase_h

#include <QtGlobal> // for qint64

#include <vgExport.h>

class QDateTime;
class QString;

// NOTE
// The time values used by this class are based on the UNIX standard epoch
// (1970-01-01).

class QTVG_COMMON_EXPORT vgTimeBase
{
public:
  vgTimeBase(const vgTimeBase& other) : unixTime(other.unixTime) {}

  explicit vgTimeBase(qint64 microseconds) : unixTime(microseconds) {}

  explicit vgTimeBase(const QDateTime& dateTime);

  qint64    toInt64() const { return this->unixTime; }
  QDateTime toDateTime() const;

  QString dateString() const;
  QString timeString() const;

protected:
  qint64 unixTime;
};

#endif
