/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
