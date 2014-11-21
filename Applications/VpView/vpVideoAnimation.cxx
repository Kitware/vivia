/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVideoAnimation.h"

#include "vpViewCore.h"

#include <qtScopedValueChange.h>

#include <vgCheckArg.h>

#include <QTimerEvent>

//-----------------------------------------------------------------------------
vpVideoAnimation::vpVideoAnimation(vpViewCore* viewCore) :
  qtAbstractAnimation(viewCore),
  ViewCore(viewCore),
  TimeDuration(0.0),
  TimeOffset(0.0),
  FrameInterval(0.5),
  BlockUpdates(false)
{
  connect(this, SIGNAL(stateChanged(QAbstractAnimation::State,
                                    QAbstractAnimation::State)),
          this, SLOT(updateState(QAbstractAnimation::State,
                                 QAbstractAnimation::State)));

  connect(this->ViewCore, SIGNAL(timeChanged(double)),
          this, SLOT(updateTimeFromCore(double)));

  this->setPlaybackMode(PM_Sequential);
}

//-----------------------------------------------------------------------------
vpVideoAnimation::~vpVideoAnimation()
{
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::updateTimeRange(double start, double end)
{
  // Adjust current time for new start time, if it has changed
  if (start != this->TimeOffset)
    {
    double newTime = this->currentTime() + this->TimeOffset - start;
    this->setCurrentTime(qMax(0.0, newTime));
    this->TimeOffset = start;
    }

  this->TimeDuration = end - start;
}

//-----------------------------------------------------------------------------
double vpVideoAnimation::duration() const
{
  return this->TimeDuration;
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::seek(const vtkVgTimeStamp& ts, vg::SeekMode direction)
{
  vgVideoSeekRequest request;
  request.TimeStamp = ts;
  request.Direction = direction;
  this->seek(request);
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::seek(vgVideoSeekRequest request)
{
  // Fix up request direction (use Exact if not specified)
  if (request.Direction < 0)
    {
    request.Direction = vg::SeekExact;
    }

  if (!request.TimeStamp.HasTime())
    {
    // If we don't have a valid time stamp, we cannot do anything with this
    // request...
    return;
    }

  if (this->state() == QAbstractAnimation::Stopped && this->duration() > 0.0)
    {
    qtScopedValueChange<bool> bu(this->BlockUpdates, true);
    this->start();
    this->pause();
    }

  // Bound request time to within the animation's range
  const double time = request.TimeStamp.GetTime();
  const double maxTime = (this->TimeDuration + this->TimeOffset) * 1e6;
  request.TimeStamp.SetTime(qBound(this->TimeOffset * 1e6, time, maxTime));
  this->postUpdate(request);
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::setFrameRange(
  const vtkVgTimeStamp& first, const vtkVgTimeStamp& last)
{
  this->updateTimeRange(first.GetTime() * 1e-6, last.GetTime() * 1e-6);
}

//-----------------------------------------------------------------------------
bool vpVideoAnimation::setState(QAbstractAnimation::State newState)
{
  if (newState != QAbstractAnimation::Stopped && this->duration() <= 0.0)
    {
    // We cannot play or pause an animation with no duration...
    return false;
    }

  switch (newState)
    {
    case QAbstractAnimation::Running:
      if (this->sequentialPlayback())
        {
        // In sequential mode, we don't ever really want the animation to run
        // since we will be updating the position manually .
        if (this->state() == QAbstractAnimation::Stopped)
          {
          this->start();
          this->pause();
          }
        this->UpdateTimer.start(16, this);
        }
      else
        {
        if (this->state() == QAbstractAnimation::Paused)
          {
          this->resume();
          }
        else
          {
          this->start();
          }
        }
      break;
    case QAbstractAnimation::Paused:
      if (this->state() == QAbstractAnimation::Stopped)
        {
        // Must start animation before it can be paused...
        this->start();
        }
      this->pause();
      if (this->sequentialPlayback())
        {
        this->UpdateTimer.stop();
        }
      break;
    default:
      this->stop();
      break;
    }

  // Check if we succeeded
  return this->state() == newState;
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::setPlaybackMode(PlaybackMode mode)
{
  this->Mode = mode;
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::updateCurrentTime(double time)
{
  CHECK_ARG(!this->BlockUpdates);

  if (this->sequentialPlayback())
    {
    this->FrameTimer.start();
    }

  vgVideoSeekRequest request;
  request.TimeStamp = vtkVgTimeStamp((time + this->TimeOffset) * 1e6);
  request.Direction = vg::SeekNearest;
  this->postUpdate(request);
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::postUpdate(vgVideoSeekRequest request)
{
  CHECK_ARG(!this->BlockUpdates);

  // Post update
  if (!this->NextRequest.TimeStamp.IsValid())
    {
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
    }

  // Set next seek
  this->NextRequest = request;
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::update()
{
  CHECK_ARG(this->NextRequest.TimeStamp.IsValid());

  this->ViewCore->seekToFrame(this->NextRequest.TimeStamp,
                              this->NextRequest.Direction);

  // Reset the pending update
  this->NextRequest = vgVideoSeekRequest();
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == this->UpdateTimer.timerId())
    {
    if (!this->FrameTimer.isValid() ||
        this->FrameTimer.hasExpired(this->FrameInterval * 1000 /
                                    fabs(this->rate())))
      {
      if (this->rate() < 0.0)
        {
        this->setCurrentTime(this->currentTime() - this->FrameInterval);
        }
      else
        {
        this->setCurrentTime(this->currentTime() + this->FrameInterval);
        }
      }
    }
  else
    {
    qtAbstractAnimation::timerEvent(event);
    }
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::updateState(QAbstractAnimation::State newState,
                                   QAbstractAnimation::State oldState)
{
  Q_UNUSED(oldState);
  if (this->sequentialPlayback())
    {
    if (newState != QAbstractAnimation::Running)
      {
      this->UpdateTimer.stop();
      }
    }
}

//-----------------------------------------------------------------------------
void vpVideoAnimation::updateTimeFromCore(double microseconds)
{
  qtScopedValueChange<bool> bu(this->BlockUpdates, true);
  this->setCurrentTime((microseconds * 1e-6) - this->TimeOffset);
}
