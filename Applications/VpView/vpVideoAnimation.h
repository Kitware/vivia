/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVideoAnimation_h
#define __vpVideoAnimation_h

#include <QBasicTimer>
#include <QElapsedTimer>

#include <qtAbstractAnimation.h>

#include <vgVideoSourceRequestor.h>

class vpViewCore;
class vtkVgTimeStamp;

class vpVideoAnimation : public qtAbstractAnimation
{
  Q_OBJECT

public:
  enum PlaybackMode
    {
    PM_Sequential,
    PM_RealTime
    };

public:
  explicit vpVideoAnimation(vpViewCore* viewCore);
  virtual ~vpVideoAnimation();

  void setFrameRange(const vtkVgTimeStamp& first, const vtkVgTimeStamp& last);
  void seek(const vtkVgTimeStamp& ts, vg::SeekMode direction = vg::SeekNearest);

  bool setState(QAbstractAnimation::State);

  void setPlaybackMode(PlaybackMode mode);

  void setFrameInterval(double seconds)
    { this->FrameInterval = seconds; }

  bool sequentialPlayback() { return this->Mode == PM_Sequential; }
  bool realTimePlayback() { return this->Mode == PM_RealTime; }

protected slots:
  void update();

  void updateState(QAbstractAnimation::State newState,
                   QAbstractAnimation::State oldState);

  void updateTimeFromCore(double microseconds);

protected:
  virtual void timerEvent(QTimerEvent* event);

private:
  Q_DISABLE_COPY(vpVideoAnimation)

  void seek(vgVideoSeekRequest);

  virtual double duration() const;
  virtual void updateCurrentTime(double time);

  void updateTimeRange(double start, double end);
  void postUpdate(vgVideoSeekRequest);

  vpViewCore* ViewCore;

  double TimeDuration, TimeOffset, FrameInterval;
  vgVideoSeekRequest NextRequest;
  bool BlockUpdates;

  PlaybackMode Mode;
  QBasicTimer UpdateTimer;
  QElapsedTimer FrameTimer;
};

#endif
