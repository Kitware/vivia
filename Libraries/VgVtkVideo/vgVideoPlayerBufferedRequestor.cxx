/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgVideoPlayerBufferedRequestor.h"

#include <qtGlobal.h>
#include <qtMath.h>

#include <vgCheckArg.h>

#include "vgVideoSource.h"

//-----------------------------------------------------------------------------
class vgVideoPlayerBufferRequestor : public vgVideoPlayerRequestor
{
public:
  static vgVideoPlayerBufferRequestor*
  create(vgVideoPlayerBufferedRequestor* q)
    { return new vgVideoPlayerBufferRequestor(q); }

  virtual void update(vgVideoSeekRequest request, vgVtkVideoFramePtr frame)
    {
    CHECK_ARG(frame);
    QTE_Q(vgVideoPlayerBufferedRequestor);
    q->addFrameToBuffer(request, frame);
    }

protected:
  QTE_DECLARE_PUBLIC_PTR(vgVideoPlayerBufferedRequestor);

  vgVideoPlayerBufferRequestor(vgVideoPlayerBufferedRequestor* q) :
    q_ptr(q)
    {}

private:
  QTE_DECLARE_PUBLIC(vgVideoPlayerBufferedRequestor);
};

//-----------------------------------------------------------------------------
vgVideoPlayerBufferedRequestor* vgVideoPlayerBufferedRequestor::create()
{
  return new vgVideoPlayerBufferedRequestor;
}

//-----------------------------------------------------------------------------
vgVideoPlayerBufferedRequestor::vgVideoPlayerBufferedRequestor() :
  VideoSource(0),
  SourceLastFlushed(true),
  BufferRequestor(0),
  BufferingEnabled(false),
  BufferFillDirection(vg::SeekExact)
{
}

//-----------------------------------------------------------------------------
vgVideoPlayerBufferedRequestor::~vgVideoPlayerBufferedRequestor()
{
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::release()
{
  this->VideoSource = 0;
  this->clearBuffer();

  this->releaseBufferRequestor();
  vgVideoPlayerRequestor::release();
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::releaseBufferRequestor()
{
  if (this->BufferRequestor)
    {
    this->BufferRequestor->release();
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::setBufferLimit(
  double maxTime, int maxFrames, qreal maxRate)
{
  this->BufferFrameLimit = maxFrames;
  this->BufferTimeLimit = maxTime * 1e6;
  this->BufferRateLimit = maxRate;
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::setVideoSource(vgVideoSource* source)
{
  // Throw out any buffer from the previous source (if any)
  this->clearBuffer();
  this->VideoSource = source;
  this->SourceLastFlushed = true,

  // If we have a requestor for the old source, release it so we won't get
  // any more frames from it
  this->releaseBufferRequestor();

  // Create a new requestor for the new source
  this->BufferRequestor = vgVideoPlayerBufferRequestor::create(this);
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::requestFrame(
  vgVideoSource* source, vgVideoSeekRequest& request)
{
  Q_ASSERT(request.TimeStamp.IsValid());

  const vgTimeStamp ts = request.TimeStamp.GetRawTimeStamp();

  // Check if our source has changed
  if (source != this->VideoSource)
    {
    // If so, reset to use the new source
    this->setVideoSource(source);
    }

  // Does the request lie within the range we have buffered?
  while (this->isRequestWithinBuffer(ts, request.Direction))
    {
    // Use buffer to satisfy request
    FrameBufferIterator iter = this->Buffer.find(ts, request.Direction);
    if (iter != this->Buffer.end())
      {
      // Did we fill the request?
      if (iter.value())
        {
        // Yes; remove the frame from the buffer...
        vgVtkVideoFramePtr frame(*iter);
        *iter = 0;

        // ...purge earlier entries from the buffer...
        this->pruneBuffer(iter, ts);

        // ...notify source to discard its notion of the last frame we saw...
        if (!this->SourceLastFlushed)
          {
          source->clearLastRequest(this);
          this->SourceLastFlushed = true;
          }

        // ...and hand the frame off to the caller (will also start or resume
        // filling the buffer, if not full)
        this->update(request, frame);
        }
      else
        {
        // No... did we give back the same frame as before?
        if (vtkVgTimeStamp(iter.key()) == this->LastFrameTime)
          {
          // Yes; does caller care?
          if (request.RequestId >= 0)
            {
            // Yes; notify caller of discarded request
            emit this->seekRequestDiscarded(request.RequestId,
                                            this->LastFrameTime);
            }
          }
        else
          {
          // No; this can happen if we were between frames leaning toward
          // a frame 'later' in the buffer, and a seek (while playing) makes
          // a request that resolves to the 'earlier' frame... in this case,
          // since the frames are non-persistent (i.e. we no longer have the
          // earlier frame to give back), punt on using the buffer
          break;
          }
        }
      }
    // The find() should only fail if the seek mode was Exact, and we have no
    // match... if so, do we need to notify the caller?
    else if (request.RequestId >= 0)
      {
      // Yes; notify caller of discarded request
      emit this->seekRequestDiscarded(request.RequestId, this->LastFrameTime);
      }

    // Use of buffer was successful
    return;
    }

  // Check for seek requests 'before' the buffer head
  if (!this->Buffer.isEmpty() && this->isTimeStampBeforeBufferHead(ts))
    {
    // Any such request means we must throw out the buffer, because the reply
    // we get will otherwise violate its continuity property
    this->clearBuffer();
    }

  // Hand off to source to satisfy request
  vgVideoPlayerRequestor::requestFrame(source, request);
  this->SourceLastFlushed = false;
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::setPlaybackRate(qreal rate)
{
  if (!qFuzzyIsNull(rate))
    {
    const vg::SeekMode newDirection =
      (rate > 0.0 ? vg::SeekNext : vg::SeekPrevious);
    if (this->BufferFillDirection != newDirection)
      {
      this->BufferFillDirection = newDirection;
      this->clearBuffer(true);
      }
    }

  // Disable buffering if playing back "significantly" faster than real time
  // (with somewhat arbitrary definition of "significantly")... don't bother to
  // clear the buffer, as we might as well use anything we have until we run
  // out and it is flushed naturally
  this->BufferingEnabled = (fabs(rate) < this->BufferRateLimit);
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::update(
  vgVideoSeekRequest seekRequest, vgVtkVideoFramePtr frame)
{
  if (frame)
    {
    const vgTimeStamp ts = frame->MetaData.Time.GetRawTimeStamp();

    if (this->isTimeStampBeforeBufferHead(ts))
      {
      this->clearBuffer();
      this->Buffer.insert(ts, 0);
      this->BufferHead = this->BufferTail = ts;
      }
    else
      {
      FrameBufferIterator iter = this->Buffer.find(ts);

      if (iter == this->Buffer.end())
        {
        // Add placeholder to buffer if not found...
        iter = this->Buffer.insert(ts, 0);
        }
      else
        {
        // ...otherwise replace old frame with placeholder
        delete *iter;
        *iter = 0;
        }

      // Purge earlier entries from the buffer
      this->pruneBuffer(iter, seekRequest.TimeStamp.GetRawTimeStamp());
      if (this->Buffer.count() < 2)
        {
        // If our new entry is the only one left, it must now be the tail
        this->BufferTail = ts;
        }
      }
    }

  // Pass frame along to caller
  vgVideoPlayerRequestor::update(seekRequest, frame);

  // Start filling buffer
  this->fillBuffer();
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::addFrameToBuffer(
  const vgVideoSeekRequest& request, vgVtkVideoFramePtr frame)
{
  // Don't add frame it it would violate continuity property
  CHECK_ARG(request.TimeStamp == this->BufferTail);
  CHECK_ARG(request.Direction == this->BufferFillDirection);

  // Add frame to buffer
  const vgTimeStamp ts = frame->MetaData.Time.GetRawTimeStamp();
  this->Buffer.insert(ts, frame.take());
  this->BufferTail = ts;

  // Continue filling buffer until full
  this->fillBuffer();
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::fillBuffer()
{
  // Don't fill if buffering is disabled
  CHECK_ARG(this->BufferingEnabled);
  CHECK_ARG(this->BufferFillDirection != vg::SeekExact);

  // Don't fill if full based on frame count
  CHECK_ARG(this->Buffer.count() < this->BufferFrameLimit);

  if (this->BufferHead.HasTime() && this->BufferTail.HasTime())
    {
    const double bufferTimeSize =
      fabs(this->BufferHead.Time - this->BufferTail.Time);
    // Don't fill if full based on time range
    CHECK_ARG(bufferTimeSize < this->BufferTimeLimit);
    }

  // Request next (in playback direction) frame for buffer
  this->PendingBufferRequest = this->BufferTail;
  this->issueBufferRequest();
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::issueBufferRequest()
{
  vgVideoSeekRequest request;
  request.TimeStamp = this->PendingBufferRequest;
  request.Direction = this->BufferFillDirection;
  this->BufferRequestor->requestFrame(this->VideoSource, request);
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::pruneBuffer(
  FrameBufferIterator iter, const vgTimeStamp& ref)
{
  if (this->BufferFillDirection == vg::SeekNext)
    {
    if (iter != this->Buffer.begin())
      {
      // If buffer seek landed us on the later of the first two entries...
      if (ref < iter.key())
        {
        // ...then go back one, so that if we seek again between entries we can
        // still use the buffer
        --iter;
        }

      // Update head
      this->BufferHead = iter.key();

      // Remove all entries before updated iterator
      const vgTimeStamp keep = iter.key();
      iter = this->Buffer.begin();
      while (iter != this->Buffer.end() && iter.key() != keep)
        {
        // Erase entry
        delete *iter;
        iter = this->Buffer.erase(iter);
        }
      }
    }
  else if (this->BufferFillDirection == vg::SeekPrevious)
    {
    if ((iter + 1) != this->Buffer.end())
      {
      // If buffer seek landed us on the later of the first two entries...
      if (ref > iter.key())
        {
        // ...then go forward one, so that if we seek again between entries we
        // can still use the buffer
        ++iter;
        }

      // Update head
      this->BufferHead = iter.key();

      // Remove all entries after updated iterator
      ++iter;
      while (iter != this->Buffer.end())
        {
        // Erase entry
        delete *iter;
        iter = this->Buffer.erase(iter);
        }
      }
    }
  else
    {
    // Remove everything but specified entry
    const vgTimeStamp keep = iter.key();
    iter = this->Buffer.begin();
    while (iter != this->Buffer.end())
      {
      if (iter.key() == keep)
        {
        // Skip specified entry
        ++iter;
        }
      else
        {
        // Erase other entries
        delete *iter;
        iter = this->Buffer.erase(iter);
        }
      }
    // Update head
    this->BufferHead = keep;
    }
}

//-----------------------------------------------------------------------------
void vgVideoPlayerBufferedRequestor::clearBuffer(bool keepLast)
{
  // Clear the buffer
  qDeleteAll(this->Buffer);
  this->Buffer.clear();

  if (keepLast && this->LastFrameTime.IsValid())
    {
    // Add placeholder for the last frame seen
    const vgTimeStamp ts = this->LastFrameTime.GetRawTimeStamp();
    this->Buffer.insert(ts, 0);
    this->BufferHead = this->BufferTail = ts;
    }

  // If we have outstanding buffer requests...
  if (this->VideoSource && this->PendingBufferRequest.IsValid())
    {
    // ...clear our pending request and make an invalid request (which will
    // either be a no-op, or will purge any outstanding buffer requests,
    // depending on the source's processing order)
    this->PendingBufferRequest.Reset();
    this->issueBufferRequest();
    }
}

//-----------------------------------------------------------------------------
bool vgVideoPlayerBufferedRequestor::isRequestWithinBuffer(
  const vgTimeStamp& ts, vg::SeekMode direction)
{
  if (!this->Buffer.isEmpty())
    {
    const vgTimeStamp lower = this->Buffer.begin().key();
    const vgTimeStamp upper = (this->Buffer.end() - 1).key();
    if (ts == lower)
      {
      // If ts == lower, the request is within the buffer unless the seek
      // direction == SeekPrevious
      return direction != vg::SeekPrevious;
      }
    else if (ts == upper)
      {
      // Similar for upper
      return direction != vg::SeekNext;
      }
    else
      {
      // For all other modes, lower <= ts <= upper will suffice
      return lower <= ts && ts <= upper;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vgVideoPlayerBufferedRequestor::isTimeStampBeforeBufferHead(
  const vgTimeStamp& ts)
{
  return
    (this->BufferFillDirection == vg::SeekNext ? ts < this->BufferHead :
     this->BufferFillDirection == vg::SeekPrevious ? ts > this->BufferHead :
     true);
}
