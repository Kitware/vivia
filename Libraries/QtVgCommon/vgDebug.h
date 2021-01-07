// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgDebug_h
#define __vgDebug_h

#include <QDebug>

#ifdef __vgTimeStamp_h

//-----------------------------------------------------------------------------
inline QDebug& operator<<(QDebug& dbg, const vgTimeStamp& ts)
{
  dbg.nospace() << "(time ";
  if (ts.HasTime())
    {
    dbg << qPrintable(QString::number(ts.Time, 'g', 12));
    }
  else
    {
    dbg << "<invalid>";
    }
  dbg << ", frame ";
  if (ts.HasFrameNumber())
    {
    dbg << ts.FrameNumber;
    }
  else
    {
    dbg << "<invalid>";
    }
  dbg << ')';
  return dbg.space();
}

#endif

#ifdef __vgRange_h

//-----------------------------------------------------------------------------
template <typename T>
inline QDebug& operator<<(QDebug& dbg, const vgRange<T>& r)
{
  dbg.nospace() << "(lower " << r.lower << ", upper " << r.upper << ')';
  return dbg.space();
}

#endif

#endif
