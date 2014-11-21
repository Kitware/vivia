/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpUtils_h
#define __vpUtils_h

class QString;

class vtkVgTimeStamp;

namespace vpUtils
{
QString GetTimeAndFrameNumberString(const vtkVgTimeStamp& ts, int frameOffset);
};

#endif // __vpUtils_h
