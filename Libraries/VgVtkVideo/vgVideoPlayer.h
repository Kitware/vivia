/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgVideoPlayer_h
#define __vgVideoPlayer_h

#include <QObject>

#include <qtDebugArea.h>
#include <qtGlobal.h>

#include <vgExport.h>

#include <vgNamespace.h>

#include "vgVtkVideoFrame.h"

class vgVideoSource;

class vgVideoPlayerPrivate;

class VG_VTKVIDEO_EXPORT vgVideoPlayer : public QObject
{
  Q_OBJECT

public:
  enum PlaybackMode
    {
    Stopped,
    Paused,
    Playing,
    Buffering,
    Live
    };

  vgVideoPlayer(QObject* parent = 0);
  virtual ~vgVideoPlayer();

  void setDebugArea(qtDebugAreaAccessor);
  void setVideoSource(vgVideoSource*);

  vgVideoPlayer::PlaybackMode playbackMode() const;
  qreal playbackSpeed() const;

  double livePlaybackOffset() const;

signals:
  void frameAvailable(vtkVgVideoFrame frame, qint64 seekRequestId);
  void seekRequestDiscarded(qint64 id, vtkVgTimeStamp lastFrameAvailable);

  void playbackSpeedChanged(vgVideoPlayer::PlaybackMode, qreal rate);

public slots:
  void setSourceStreaming(bool);
  void setSourceFrameRange(vtkVgTimeStamp first, vtkVgTimeStamp last);

  void setPlaybackSpeed(vgVideoPlayer::PlaybackMode, qreal rate = 1.0);
  void seekToTimestamp(qint64 requestId, vtkVgTimeStamp,
                       vg::SeekMode = vg::SeekNearest);
  void setLivePlaybackOffset(double);

protected slots:
  void updatePlaybackMode(vgVideoPlayer::PlaybackMode, qreal rate);
  void emitFrameAvailable(vgVtkVideoFramePtr frame, qint64 seekRequestId);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgVideoPlayer)
  friend class vgVideoPlayerRequestor;

private:
  QTE_DECLARE_PRIVATE(vgVideoPlayer)
  Q_DISABLE_COPY(vgVideoPlayer)
};

#endif
