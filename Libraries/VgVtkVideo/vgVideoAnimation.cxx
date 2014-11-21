/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgVideoAnimation.h"

#include <qtScopedValueChange.h>

#include <vgCheckArg.h>

#include "vgVideoPlayerPrivate.h"

//-----------------------------------------------------------------------------
vgVideoAnimation::vgVideoAnimation(vgVideoPlayerPrivate* q) :
  qtAbstractAnimation(q),
  q_ptr(q),
  TimeDuration(0.0),
  TimeOffset(0.0),
  BlockUpdates(false)
{
}

//-----------------------------------------------------------------------------
vgVideoAnimation::~vgVideoAnimation()
{
}

//-----------------------------------------------------------------------------
void vgVideoAnimation::updateTimeRange(double start, double end)
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
double vgVideoAnimation::duration() const
{
  return this->TimeDuration;
}

//-----------------------------------------------------------------------------
void vgVideoAnimation::seek(vgVideoSeekRequest request)
{
  // Fix up request direction (use Exact if not specified)
  if (request.Direction < 0)
    {
    request.Direction = vg::SeekExact;
    }

  // Find time for frame, if not provided
  if (!request.TimeStamp.HasTime() && request.TimeStamp.HasFrameNumber())
    {
    QTE_Q(vgVideoPlayerPrivate);
    request.TimeStamp =
      q->findTime(request.TimeStamp.GetFrameNumber(), request.Direction);
    }

  if (!request.TimeStamp.IsValid())
    {
    // If we don't have a valid time stamp, we cannot do anything with this
    // request...
    return;
    }

  if (this->state() == QAbstractAnimation::Stopped && this->duration() > 0.0)
    {
    this->start();
    this->pause();
    }

  // Bound request time to within the animation's range
  const double time = request.TimeStamp.GetTime();
  const double maxTime = (this->TimeDuration + this->TimeOffset) * 1e6;
  request.TimeStamp.SetTime(qBound(this->TimeOffset * 1e6, time, maxTime));
  this->postUpdate(request);

  qtScopedValueChange<bool> bu(this->BlockUpdates, true);
  this->setCurrentTime((time * 1e-6) - this->TimeOffset);
}

//-----------------------------------------------------------------------------
void vgVideoAnimation::seek(const vtkVgTimeStamp& ts, vg::SeekMode direction)
{
  vgVideoSeekRequest request;
  request.TimeStamp = ts;
  request.Direction = direction;
  this->seek(request);
}

//-----------------------------------------------------------------------------
void vgVideoAnimation::setFrameRange(
  const vtkVgTimeStamp& first, const vtkVgTimeStamp& last)
{
  this->updateTimeRange(first.GetTime() * 1e-6, last.GetTime() * 1e-6);
}

//-----------------------------------------------------------------------------
bool vgVideoAnimation::setState(QAbstractAnimation::State newState)
{
  if (this->state() != newState)
    {
    if (newState != QAbstractAnimation::Stopped && this->duration() <= 0.0)
      {
      // We cannot play or pause an animation with no duration...
      return false;
      }
    switch (newState)
      {
      case QAbstractAnimation::Running:
        if (this->state() == QAbstractAnimation::Paused)
          {
          this->resume();
          }
        else
          {
          this->start();
          }
        break;
      case QAbstractAnimation::Paused:
        if (this->state() == QAbstractAnimation::Stopped)
          {
          // Must start animation before it can be paused...
          this->start();
          }
        this->pause();
        break;
      default:
        this->stop();
        break;
      }

    // Check if we succeeded
    return this->state() == newState;
    }

  // No change was needed
  return true;
}

//-----------------------------------------------------------------------------
void vgVideoAnimation::updateCurrentTime(double time)
{
  vgVideoSeekRequest request;

  request.TimeStamp = vgTimeStamp::fromTime((time + this->TimeOffset) * 1e6);
  request.Direction = vg::SeekNearest;

  this->postUpdate(request);
}

//-----------------------------------------------------------------------------
void vgVideoAnimation::postUpdate(vgVideoSeekRequest request)
{
  CHECK_ARG(!this->BlockUpdates);

  // Post update
  if (!this->NextRequest.TimeStamp.IsValid())
    {
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
    }

  if (request.RequestId < 0)
    {
    // If the request does not have a valid ID, copy the one from the previous
    // request, which will either also be invalid (if we had no pending
    // requests with an ID), or else will the ID of the most recent request
    // that DID have an ID... this way, when we reply, we will still indicate
    // the most recent ID'd request that has been satisfied (which would
    // otherwise get lost if we didn't do this!)
    request.RequestId = this->NextRequest.RequestId;
    }

  // Set next seek
  this->NextRequest = request;
}

//-----------------------------------------------------------------------------
void vgVideoAnimation::update()
{
  CHECK_ARG(this->NextRequest.TimeStamp.IsValid());

  // Pass off to implementation handler
  QTE_Q(vgVideoPlayerPrivate);
  q->requestFrame(this->NextRequest);

  // Reset the pending update
  this->NextRequest = vgVideoSeekRequest();
}
