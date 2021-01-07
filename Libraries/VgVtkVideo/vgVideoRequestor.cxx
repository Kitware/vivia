// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgVideoRequestor.h"

#include "vgVideoSource.h"
#include "vgVideoSourceRequestor.h"

#include <QEventLoop>

enum { SeekId = 1 };

class vgVideoRequestorPrivate : public vgVideoSource {};

//-----------------------------------------------------------------------------
class vgVideoRequestorObject : public vgVideoSourceRequestor
{
public:
  vgVideoRequestorObject(vtkVgVideoFrame& out) : Frame(out) {}

  virtual void update(vgVideoSeekRequest, vgVtkVideoFramePtr);

  vgVideoSource* VideoSource;
  vtkVgVideoFrame& Frame;

  QEventLoop EventLoop;
  bool Finished;
};

typedef QSharedPointer<vgVideoRequestorObject> vgVideoRequestorPtr;

//-----------------------------------------------------------------------------
void vgVideoRequestorObject::update(
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
vgVideoRequestor::vgVideoRequestor(vgVideoSource* source)
  : d_ptr(static_cast<vgVideoRequestorPrivate*>(source))
{
}

//-----------------------------------------------------------------------------
vgVideoRequestor::~vgVideoRequestor()
{
}

//-----------------------------------------------------------------------------
bool vgVideoRequestor::requestFrame(
  vtkVgVideoFrame& out, vtkVgTimeStamp time, vg::SeekMode direction)
{
  return vgVideoRequestor::requestFrame(out, this->d_ptr, time, direction);
}

//-----------------------------------------------------------------------------
bool vgVideoRequestor::requestFrame(
  vtkVgVideoFrame& out, vgVideoSource* source,
  vtkVgTimeStamp time, vg::SeekMode direction)
{
  vgVideoRequestorPtr requestorPtr(new vgVideoRequestorObject(out));
  vgVideoRequestorObject& requestor = *requestorPtr.data();

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
