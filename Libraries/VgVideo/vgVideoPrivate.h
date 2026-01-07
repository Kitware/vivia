// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
