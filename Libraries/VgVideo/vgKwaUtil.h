// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgKwaUtil_h
#define __vgKwaUtil_h

#include <vgExport.h>

class QString;

namespace vgKwaUtil
{
  VG_VIDEO_EXPORT bool resolvePath(QString& name, const QString& relativeTo,
                                   const char* what);
}

#endif
