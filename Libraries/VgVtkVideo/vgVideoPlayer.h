// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
