/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
