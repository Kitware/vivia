// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
