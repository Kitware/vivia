/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "moc_vsVideoSourcePrivate.cpp"

#include <qtMap.h>

#include "vsLibDebug.h"

QTE_IMPLEMENT_D_FUNC(vsVideoSource)

//BEGIN vsVideoSourcePrivate

//-----------------------------------------------------------------------------
vsVideoSourcePrivate::vsVideoSourcePrivate(vsVideoSource* q)
  : q_ptr(q), Status(vsDataSource::NoSource), Streaming(false)
{
}

//-----------------------------------------------------------------------------
vsVideoSourcePrivate::~vsVideoSourcePrivate()
{
}

//-----------------------------------------------------------------------------
void vsVideoSourcePrivate::run()
{
  qtDebug(vsdVideoSource) << "video source thread executing";
  qtThread::run();
  qtDebug(vsdVideoSource) << "video source thread halting";
}

//-----------------------------------------------------------------------------
void vsVideoSourcePrivate::emitMetadata(
  QList<vtkVgVideoFrameMetaData> metadata)
{
  QTE_Q(vsVideoSource);
  emit q->metadataAvailable(metadata);
}

//-----------------------------------------------------------------------------
void vsVideoSourcePrivate::updateFrameRange(
  vtkVgTimeStamp first, vtkVgTimeStamp last)
{
  QTE_Q(vsVideoSource);
  QMetaObject::invokeMethod(q, "setFrameRange",
                            Q_ARG(vtkVgTimeStamp, first),
                            Q_ARG(vtkVgTimeStamp, last));
}

//-----------------------------------------------------------------------------
void vsVideoSourcePrivate::queueFrameRequest(vgVideoSeekRequest request)
{
  // Set up handler
  if (this->CurrentRequests.isEmpty())
    {
    QMetaObject::invokeMethod(this, "flushFrameRequests",
                              Qt::QueuedConnection);
    }

  vgVideoSeekRequest& currentRequest =
    this->CurrentRequests[request.Requestor.data()];
  if (request.RequestId < 0)
    {
    // If the request does not have a valid ID, copy the one from the previous
    // request, which will either also be invalid (if we had no pending
    // requests with an ID), or else will the ID of the most recent request
    // that DID have an ID... this way, when we reply, we will still indicate
    // the most recent ID'd request that has been satisfied (which would
    // otherwise get lost if we didn't do this!)
    request.RequestId = currentRequest.RequestId;
    }

  // Set next request
  currentRequest.destructiveCopy(request);
}

//-----------------------------------------------------------------------------
void vsVideoSourcePrivate::flushFrameRequests()
{
  // Dispatch queued requests for all requestors...
  qtUtil::mapBound(this->CurrentRequests, this,
                   &vsVideoSourcePrivate::requestFrame);
  // ...then clear the pending requests
  this->CurrentRequests.clear();
}

//END vsVideoSourcePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsVideoSource

//-----------------------------------------------------------------------------
vsVideoSource::vsVideoSource(vsVideoSourcePrivate* d) : d_ptr(d)
{
  if (d->Status != vsDataSource::NoSource)
    this->updateStatus(d->Status);
}

//-----------------------------------------------------------------------------
vsVideoSource::~vsVideoSource()
{
  QTE_D(vsVideoSource);
  qtDebug(vsdVideoSource) << "stopping video source thread";
  d->quit();
  d->wait();
  delete d;
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsVideoSource::status() const
{
  QTE_D_CONST(vsVideoSource);
  return d->Status;
}

//-----------------------------------------------------------------------------
void vsVideoSource::start()
{
  QTE_D(vsVideoSource);

  if (d->Status == vsDataSource::NoSource)
    return;

  vsDataSource::start();
  d->start();
}

//-----------------------------------------------------------------------------
void vsVideoSource::updateStatus(vsDataSource::Status status)
{
  QTE_D(vsVideoSource);
  d->Status = status;
  emit this->statusChanged(status);
}

//-----------------------------------------------------------------------------
vgRange<vtkVgTimeStamp> vsVideoSource::frameRange() const
{
  QTE_D_CONST(vsVideoSource);
  return vgRange<vtkVgTimeStamp>(d->FirstAvailableFrame,
                                 d->LastAvailableFrame);
}

//-----------------------------------------------------------------------------
void vsVideoSource::setFrameRange(
  vtkVgTimeStamp first, vtkVgTimeStamp last)
{
  QTE_D(vsVideoSource);
  d->FirstAvailableFrame = first;
  d->LastAvailableFrame = last;
  emit this->frameRangeAvailable(first, last);
}

//-----------------------------------------------------------------------------
bool vsVideoSource::isStreaming() const
{
  QTE_D_CONST(vsVideoSource);
  return d->Streaming;
}

//-----------------------------------------------------------------------------
void vsVideoSource::setStreaming(bool streaming)
{
  QTE_D(vsVideoSource);
  if (d->Streaming != streaming)
    {
    d->Streaming = streaming;
    emit this->streamingChanged(streaming);
    }
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vsVideoSource::findTime(
  unsigned int frameNumber, vg::SeekMode direction)
{
  QTE_D(vsVideoSource);

  vtkVgTimeStamp result;
  QMetaObject::invokeMethod(d, "findTime", Qt::BlockingQueuedConnection,
                            Q_ARG(vtkVgTimeStamp*, &result),
                            Q_ARG(unsigned int, frameNumber),
                            Q_ARG(vg::SeekMode, direction));
  return result;
}

//-----------------------------------------------------------------------------
void vsVideoSource::requestFrame(vgVideoSeekRequest request)
{
  QTE_D(vsVideoSource);
  QMetaObject::invokeMethod(d, "queueFrameRequest",
                            Q_ARG(vgVideoSeekRequest, request));
}

//-----------------------------------------------------------------------------
void vsVideoSource::clearLastRequest(vgVideoSourceRequestor* requestor)
{
  QTE_D(vsVideoSource);
  QMetaObject::invokeMethod(d, "clearLastRequest",
                            Q_ARG(vgVideoSourceRequestor*, requestor));
}

//END vsVideoSource
