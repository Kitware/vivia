/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
