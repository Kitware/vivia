/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgVideoBuffer_h
#define __vgVideoBuffer_h

#include <vgExport.h>

#include "vgVideo.h"

class vgVideoBufferPrivate;

class VG_VIDEO_EXPORT vgVideoBuffer : public vgVideo
{
public:
  explicit vgVideoBuffer(const char* compressionFormat = 0);
  virtual ~vgVideoBuffer();

  bool insert(vgTimeStamp pos, vgImage image);

private:
  QTE_DECLARE_PRIVATE(vgVideoBuffer)
  Q_DISABLE_COPY(vgVideoBuffer)
};

#endif
