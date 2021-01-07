// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgVideoPlayerPrivate_h
#define __vgVideoPlayerPrivate_h

#include <qtThread.h>

#include <vgNamespace.h>

#include "vgVideoAnimation.h"
#include "vgVideoPlayer.h"
#include "vgVideoSourceRequestor.h"

class vgVideoPlayerRequestor;

class vgVideoPlayerPrivate : public qtThread
{
  Q_OBJECT

public:
  virtual ~vgVideoPlayerPrivate();

protected:
  QTE_DECLARE_PUBLIC_PTR(vgVideoPlayer)

  vgVideoPlayerPrivate(vgVideoPlayer* q);

  virtual void run();

  void debugPlayback(const char* msg);

  vtkVgTimeStamp findTime(unsigned int frameNumber, vg::SeekMode);
  void requestFrame(vgVideoSeekRequest);

  void updatePlaybackMode(vgVideoPlayer::PlaybackMode, qreal rate);
  void updatePublicPlaybackMode();

protected slots:
  void setDebugArea(qtDebugAreaAccessor);

  void setSource(vgVideoSource*);
  void setStreaming(bool);
  void setFrameRange(vtkVgTimeStamp first, vtkVgTimeStamp last);

  void setPlaybackSpeed(vgVideoPlayer::PlaybackMode, qreal rate);
  void seekToTimestamp(qint64 requestId, vtkVgTimeStamp, vg::SeekMode);

  void setLivePlaybackOffset(double);

  void updateAnimationStatus(QAbstractAnimation::State);

private:
  Q_DISABLE_COPY(vgVideoPlayerPrivate)

  class SuppressStateUpdates;

  QTE_DECLARE_PUBLIC(vgVideoPlayer)
  friend class vgVideoAnimation;
  friend class SuppressStateUpdates;

  // This set of variables are for use in the parent's thread ONLY
  qtDebugAreaAccessor ParentDebugArea;
  vgVideoPlayer::PlaybackMode ParentPlaybackMode;
  qreal ParentPlaybackRate;
  double ParentLivePlaybackOffset;

  // Request callback object can be accessed from either thread
  vgVideoPlayerRequestor* Requestor;
  volatile bool RequestorFlag;

  vgVideoSource* VideoSource;
  vgVideoAnimation* Animation;
  SuppressStateUpdates* StateUpdateSuppressor;
  qtDebugAreaAccessor DebugArea;

  vgVideoPlayer::PlaybackMode PlaybackMode;
  qreal PlaybackRate;

  bool AutoPlay;
  bool AutoPause;
  bool Streaming;

  double LivePlaybackOffset;

  vtkVgTimeStamp FirstFrameTimestamp;
  vtkVgTimeStamp LastFrameTimestamp;
};

#endif
