/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgVideoFramePtrPrivate_h
#define __vgVideoFramePtrPrivate_h

#include "vgImage.h"
#include "vgVideoFramePtr.h"

class vgVideoFramePtrPrivate
{
public:
  vgVideoFramePtrPrivate() {}
  virtual ~vgVideoFramePtrPrivate() {}

  virtual vgImage data() const = 0;

  vgTimeStamp time;

private:
  Q_DISABLE_COPY(vgVideoFramePtrPrivate)
};

#endif
