// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgVideoSourceRequestor.h"

//-----------------------------------------------------------------------------
vgVideoSeekRequest::vgVideoSeekRequest() :
  RequestId(-1),
  Direction(vg::SeekUnspecified)
{
}

//-----------------------------------------------------------------------------
vgVideoSeekRequest::~vgVideoSeekRequest()
{
}

//-----------------------------------------------------------------------------
void vgVideoSeekRequest::copyWithoutRequestor(const vgVideoSeekRequest& other)
{
  this->RequestId = other.RequestId;
  this->TimeStamp = other.TimeStamp;
  this->Direction = other.Direction;
}

//-----------------------------------------------------------------------------
void vgVideoSeekRequest::destructiveCopy(vgVideoSeekRequest& other)
{
  this->copyWithoutRequestor(other);
  this->Requestor.swap(other.Requestor);
}

//-----------------------------------------------------------------------------
void vgVideoSeekRequest::sendReply(vgVtkVideoFramePtr frame) const
{
  vgVideoSeekRequest strippedRequest;
  strippedRequest.copyWithoutRequestor(*this);
  QMetaObject::invokeMethod(this->Requestor.data(), "update",
                            Q_ARG(vgVideoSeekRequest, strippedRequest),
                            Q_ARG(vgVtkVideoFramePtr, frame));
}
