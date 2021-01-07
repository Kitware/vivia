// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgVideoPlayerBufferedRequestor_h
#define __vgVideoPlayerBufferedRequestor_h

#include <vgTimeMap.h>

#include "vgVideoPlayerRequestor.h"

// CAUTION: This class is private to vgVideoPlayer; do not use it outside of
//          the vgVtkVideo library

class vgVideoPlayerBufferRequestor;

class vgVideoPlayerBufferedRequestor : public vgVideoPlayerRequestor
{
  Q_OBJECT

public:
  static vgVideoPlayerBufferedRequestor* create();
  virtual ~vgVideoPlayerBufferedRequestor();

  void setBufferLimit(double maxTime, int maxFrames, qreal maxRate);

  virtual void requestFrame(vgVideoSource*, vgVideoSeekRequest&);
  virtual void setPlaybackRate(qreal);

  virtual void update(vgVideoSeekRequest, vgVtkVideoFramePtr);

  virtual void release();

protected:
  friend class vgVideoPlayerBufferRequestor;

  typedef vtkVgVideoFrame* FramePointer;
  typedef vgTimeMap<FramePointer> FrameBuffer;
  typedef FrameBuffer::iterator FrameBufferIterator;

  vgVideoPlayerBufferedRequestor();

  void setVideoSource(vgVideoSource* source);
  void releaseBufferRequestor();

  void addFrameToBuffer(const vgVideoSeekRequest&, vgVtkVideoFramePtr frame);

  void issueBufferRequest();
  void fillBuffer();
  void pruneBuffer(FrameBufferIterator iter, const vgTimeStamp& ref);
  void clearBuffer(bool keepLast = false);

  bool isRequestWithinBuffer(const vgTimeStamp&, vg::SeekMode);
  bool isTimeStampBeforeBufferHead(const vgTimeStamp&);

  vgVideoSource* VideoSource;

  bool SourceLastFlushed;

  FrameBuffer Buffer;
  vgVideoPlayerBufferRequestor* BufferRequestor;
  vtkVgTimeStamp PendingBufferRequest;
  bool BufferingEnabled;

  vg::SeekMode BufferFillDirection;
  vgTimeStamp BufferHead;
  vgTimeStamp BufferTail;

  qreal BufferRateLimit;
  double BufferTimeLimit;
  int BufferFrameLimit;
};

#endif
