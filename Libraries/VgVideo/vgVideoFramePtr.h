// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgVideoFramePtr_h
#define __vgVideoFramePtr_h

#include <QSharedPointer>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vgTimeStamp.h>

#include "vgImage.h"

class vgVideoFramePtrPrivate;

class VG_VIDEO_EXPORT vgVideoFramePtr
{
public:
  vgVideoFramePtr();
  vgVideoFramePtr(const vgVideoFramePtr&);
  ~vgVideoFramePtr();

  vgVideoFramePtr& operator=(const vgVideoFramePtr&);

  bool isValid() const;

  vgTimeStamp time() const;
  vgImage image() const;

  vgImage operator*() const;

protected:
  QTE_DECLARE_PRIVATE_SPTR(const vgVideoFramePtr)

  friend class vgKwaVideoClip;
  friend class vgVideoBufferPrivate;

  vgVideoFramePtr(vgVideoFramePtrPrivate*);

private:
  QTE_DECLARE_PRIVATE_CONST(vgVideoFramePtr)
};

#endif
