/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
