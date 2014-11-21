/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVideoRequestor.h"

#include <QEventLoop>

#include <vgVideoSourceRequestor.h>
#include <vgVideoSource.h>

enum { SeekId = 1 };

class vsVideoRequestorPrivate : public vgVideoSource {};

//-----------------------------------------------------------------------------
class vsVideoRequestorObject : public vgVideoSourceRequestor
{
public:
  vsVideoRequestorObject(vtkVgVideoFrame& out) : Frame(out) {}

  virtual void update(vgVideoSeekRequest, vgVtkVideoFramePtr);

  vgVideoSource* VideoSource;
  vtkVgVideoFrame& Frame;

  QEventLoop EventLoop;
  bool Finished;
};

typedef QSharedPointer<vsVideoRequestorObject> vsVideoRequestorPtr;

//-----------------------------------------------------------------------------
void vsVideoRequestorObject::update(
  vgVideoSeekRequest seekRequest, vgVtkVideoFramePtr frame)
{
  Q_ASSERT(seekRequest.RequestId == SeekId);
  Q_UNUSED(seekRequest);

  this->Finished = true;
  if (frame)
    {
    this->Frame = *frame;
    this->EventLoop.exit(0);
    }
  else
    {
    this->EventLoop.exit(1);
    }
}

//-----------------------------------------------------------------------------
vsVideoRequestor::vsVideoRequestor(vgVideoSource* source)
  : d_ptr(static_cast<vsVideoRequestorPrivate*>(source))
{
}

//-----------------------------------------------------------------------------
vsVideoRequestor::~vsVideoRequestor()
{
}

//-----------------------------------------------------------------------------
bool vsVideoRequestor::requestFrame(
  vtkVgVideoFrame& out, vtkVgTimeStamp time, vg::SeekMode direction)
{
  return vsVideoRequestor::requestFrame(out, this->d_ptr, time, direction);
}

//-----------------------------------------------------------------------------
bool vsVideoRequestor::requestFrame(
  vtkVgVideoFrame& out, vgVideoSource* source,
  vtkVgTimeStamp time, vg::SeekMode direction)
{
  vsVideoRequestorPtr requestorPtr(new vsVideoRequestorObject(out));
  vsVideoRequestorObject& requestor = *requestorPtr.data();

  // Build request
  vgVideoSeekRequest request;
  request.Requestor = requestorPtr;
  request.RequestId = SeekId;
  request.TimeStamp = time;
  request.Direction = direction;

  // Issue request
  requestor.Finished = false;
  source->requestFrame(request);

  // Wait for reply
  return (requestor.Finished || (requestor.EventLoop.exec() == 0));
}
