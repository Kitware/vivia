/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgVideo_h
#define __vgVideo_h

#include <QMap>

#include <qtGlobal.h>

#include <vgExport.h>
#include <vgNamespace.h>
#include <vgRange.h>
#include <vgTimeStamp.h>

#include <vgTimeMap.h>

#include "vgVideoFramePtr.h"

class vgVideoPrivate;

class VG_VIDEO_EXPORT vgVideo
{
public:
  typedef vgTimeMap<vgVideoFramePtr> FrameMap;

  virtual ~vgVideo();

  // Stream accessors
  vgVideoFramePtr currentFrame() const;
  vgTimeStamp currentTimeStamp() const;
  vgImage currentImage() const;

  // Seek operators
  vgTimeStamp advance();
  vgTimeStamp recede();

  void rewind();
  vgTimeStamp seek(vgTimeStamp pos,
                   vg::SeekMode direction = vg::SeekNearest);

  // Random accessors
  vgVideoFramePtr frameAt(vgTimeStamp pos,
                          vg::SeekMode direction = vg::SeekNearest) const;
  vgImage imageAt(vgTimeStamp pos,
                  vg::SeekMode direction = vg::SeekNearest) const;

  FrameMap frames() const;
  vgRange<vgTimeStamp> timeRange() const;
  vgTimeStamp firstTime() const;
  vgTimeStamp lastTime() const;

  int frameCount() const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgVideo)

  explicit vgVideo(vgVideoPrivate*);

  FrameMap::const_iterator iterAt(vgTimeStamp pos,
                                  vg::SeekMode direction) const;

private:
  QTE_DECLARE_PRIVATE(vgVideo)
  Q_DISABLE_COPY(vgVideo)
};

#endif
