// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpUtils.h"

#include <vgUnixTime.h>

#include <vtkVgTimeStamp.h>

#include <QString>

namespace vpUtils
{

//-----------------------------------------------------------------------------
QString GetTimeAndFrameNumberString(const vtkVgTimeStamp& ts, int frameOffset)
{
  QString fmt("%1 (%2)");
  return fmt.arg(vgUnixTime(ts.GetTime()).timeString())
            .arg(ts.GetFrameNumber() + frameOffset);
}

} // end vpUtils
