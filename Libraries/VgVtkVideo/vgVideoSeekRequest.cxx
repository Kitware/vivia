/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
