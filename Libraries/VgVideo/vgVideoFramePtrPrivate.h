// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
