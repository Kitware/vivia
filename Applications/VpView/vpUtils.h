// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpUtils_h
#define __vpUtils_h

class QString;

class vtkVgTimeStamp;

namespace vpUtils
{
QString GetTimeAndFrameNumberString(const vtkVgTimeStamp& ts, int frameOffset);
};

#endif // __vpUtils_h
