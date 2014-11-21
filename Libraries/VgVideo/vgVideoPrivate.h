/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgVideoPrivate_h
#define __vgVideoPrivate_h

#include "vgVideo.h"

class vgVideoPrivate
{
public:
  vgVideoPrivate();
  virtual ~vgVideoPrivate();

  vgVideo::FrameMap frames;
  vgVideo::FrameMap::const_iterator pos;
};

#endif
