// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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

  bool insert(const vgTimeStamp& pos, const vgImage& image);
  bool insert(const vgTimeStamp& pos, const QByteArray& imageData,
              const QByteArray& imageFormat);

private:
  QTE_DECLARE_PRIVATE(vgVideoBuffer)
  Q_DISABLE_COPY(vgVideoBuffer)
};

#endif
