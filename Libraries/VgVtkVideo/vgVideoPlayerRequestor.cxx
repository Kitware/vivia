// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgVideoPlayerRequestor.h"

#include <vgCheckArg.h>

#include "vgVideoSource.h"

//-----------------------------------------------------------------------------
vgVideoPlayerRequestor* vgVideoPlayerRequestor::create()
{
  return new vgVideoPlayerRequestor;
}

//-----------------------------------------------------------------------------
vgVideoPlayerRequestor::vgVideoPlayerRequestor()
{
  this->Self = vgVideoSourceRequestorPtr(this);
}

//-----------------------------------------------------------------------------
vgVideoPlayerRequestor::~vgVideoPlayerRequestor()
{
  Q_ASSERT(this->Self.isNull()); // Must be released!
}

//-----------------------------------------------------------------------------
void vgVideoPlayerRequestor::release()
{
  this->disconnect();

  // Remove ourselves from all event loops... this allows us (specifically, our
  // base QObject) to be deleted from any thread, in case there are requests
  // still hanging on to pointers to us
  this->moveToThread(0);

  // Release our internal pointer reference... this may cause our destructor to
  // run immediately; if not, remaining references will be held by requestors
  // and can be released asynchronously
  // CAUTION: we may be deleted when this returns!
  this->Self.clear();
}

//-----------------------------------------------------------------------------
void vgVideoPlayerRequestor::requestFrame(
  vgVideoSource* source, vgVideoSeekRequest& request)
{
  CHECK_ARG(source);

  request.Requestor = this->Self;
  source->requestFrame(request);
}

//-----------------------------------------------------------------------------
void vgVideoPlayerRequestor::setPlaybackRate(qreal)
{
  // Not implemented in base class
}

//-----------------------------------------------------------------------------
void vgVideoPlayerRequestor::update(
  vgVideoSeekRequest seekRequest, vgVtkVideoFramePtr frame)
{
  if (frame)
    {
    this->LastFrameTime = frame->MetaData.Time;
    emit this->frameAvailable(frame, seekRequest.RequestId);
    }
  else
    {
    emit this->seekRequestDiscarded(seekRequest.RequestId,
                                    this->LastFrameTime);
    }
}
