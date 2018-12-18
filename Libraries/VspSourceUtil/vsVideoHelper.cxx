/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVideoHelper.h"

#include <QHash>

#include <vgVideo.h>

#include <vgVideoSourceRequestor.h>

QTE_IMPLEMENT_D_FUNC(vsVideoHelper)

//-----------------------------------------------------------------------------
class vsVideoHelperPrivate
{
public:
  QHash<QObject*, vtkVgTimeStamp> LastRequest;
};

//-----------------------------------------------------------------------------
vsVideoHelper::vsVideoHelper(QObject* parent)
  : QObject(parent), d_ptr(new vsVideoHelperPrivate)
{
}

//-----------------------------------------------------------------------------
vsVideoHelper::~vsVideoHelper()
{
}

//-----------------------------------------------------------------------------
vgVideoFramePtr vsVideoHelper::updateFrame(
  const vgVideo& video, const vgVideoSeekRequest& request, vgImage& image)
{
  QTE_D(vsVideoHelper);

  QObject* requestor = request.Requestor.data();
  if (!d->LastRequest.contains(requestor))
    {
    d->LastRequest.insert(requestor, vtkVgTimeStamp());
    connect(requestor, SIGNAL(destroyed(QObject*)),
            this, SLOT(cleanupRequestor(QObject*)));
    }

  // Seek to the appropriate frame
  const vgTimeStamp now = request.TimeStamp.GetRawTimeStamp();
  const vgTimeStamp lastTime = d->LastRequest[requestor].GetRawTimeStamp();
  vgVideoFramePtr frame = video.frameAt(now, request.Direction);

  // Check if the frame is valid and has advanced
  if (!frame.isValid() || frame.time() == lastTime ||
      !(image = frame.image()).isValid())
    {
    // Nope; return an invalid frame to indicate failure / nothing to do...
    if (request.RequestId >= 0)
      {
      // ...and if the requestor is expecting a reply, notify them that the
      // request was discarded
      vgVtkVideoFramePtr noFrame;
      request.sendReply(noFrame);
      }
    return vgVideoFramePtr();
    }

  // Update last-time and return the new frame
  d->LastRequest.insert(requestor, frame.time());
  return frame;
}

//-----------------------------------------------------------------------------
void vsVideoHelper::clearLastRequest(vgVideoSourceRequestor* requestor)
{
  QTE_D(vsVideoHelper);
  d->LastRequest.remove(requestor);
}

//-----------------------------------------------------------------------------
vgTimeStamp vsVideoHelper::findTime(
  const vgVideo& video, unsigned int frameNumber, vg::SeekMode roundMode)
{
  vgTimeStamp ts(frameNumber);

  // vgVideo would be perfectly within its rights to not find a matching frame
  // to our time-less time stamp, even if a frame with that frame number
  // exists; so, handle Exact by finding the nearest frame, and accepting it if
  // it has the correct frame number
  if (roundMode == vg::SeekExact)
    {
    vgVideoFramePtr frame = video.frameAt(ts);
    ts = frame.time();
    if (ts.HasFrameNumber() && ts.FrameNumber == frameNumber)
      return ts;
    return vgTimeStamp();
    }

  // Everything else (fuzzy modes) should just work correctly
  return video.frameAt(ts, roundMode).time();
}

//-----------------------------------------------------------------------------
void vsVideoHelper::cleanupRequestor(QObject* requestor)
{
  QTE_D(vsVideoHelper);
  d->LastRequest.remove(requestor);
}
