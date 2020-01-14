/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgKwaUtilPrivate_h
#define __vgKwaUtilPrivate_h

#include "vgKwaUtil.h"

#include "vgImage.h"
#include "vgIStream.h"

#include <vgTimeStamp.h>

#include <QPair>

class vsl_b_istream;

namespace vgKwaUtil
{
  VG_VIDEO_EXPORT QPair<long long, vgImage> readFrame(
    vgIStream* store, vsl_b_istream* stream, int version, long long offset,
    const char* warningPrefix, const vgTimeStamp& expectedTime = {});
}

#endif
