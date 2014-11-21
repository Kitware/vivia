/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgVideoPlayerRequestor_h
#define __vgVideoPlayerRequestor_h

#include "vgVideoSourceRequestor.h"

class vgVideoSource;

// CAUTION: This class is private to vgVideoPlayer; do not use it outside of
//          the vgVtkVideo library

class vgVideoPlayerRequestor : public vgVideoSourceRequestor
{
  Q_OBJECT

public:
  static vgVideoPlayerRequestor* create();
  virtual ~vgVideoPlayerRequestor();

  virtual void requestFrame(vgVideoSource*, vgVideoSeekRequest&);
  virtual void setPlaybackRate(qreal);

  virtual void update(vgVideoSeekRequest, vgVtkVideoFramePtr);

  virtual void release();

signals:
  void frameAvailable(vgVtkVideoFramePtr frame, qint64 seekRequestId);
  void seekRequestDiscarded(qint64 id, vtkVgTimeStamp lastFrameAvailable);

protected:
  vgVideoPlayerRequestor();

  vgVideoSourceRequestorPtr Self;
  vtkVgTimeStamp LastFrameTime;
};

#endif
