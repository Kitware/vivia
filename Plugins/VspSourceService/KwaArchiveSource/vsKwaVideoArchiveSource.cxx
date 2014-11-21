/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsKwaVideoArchiveSource.h"
#include "vsKwaVideoArchiveSourcePrivate.h"

#include <QTimerEvent>

#include <qtUtil.h>

QTE_IMPLEMENT_D_FUNC(vsKwaVideoArchiveSource)

//-----------------------------------------------------------------------------
void vsKwaVideoArchiveSourcePrivate::requestFrame(
  const vgVideoSeekRequest& request)
{
  this->Archive.requestFrame(request);
}

//-----------------------------------------------------------------------------
void vsKwaVideoArchiveSourcePrivate::clearLastRequest(
  vgVideoSourceRequestor* requestor)
{
  this->Archive.clearLastRequest(requestor);
}

//-----------------------------------------------------------------------------
void vsKwaVideoArchiveSourcePrivate::findTime(
  vtkVgTimeStamp* result, unsigned int frameNumber, vg::SeekMode roundMode)
{
  *result = this->Archive.findTime(frameNumber, roundMode);
}

//-----------------------------------------------------------------------------
vsKwaVideoArchiveSourcePrivate::vsKwaVideoArchiveSourcePrivate(
  vsVideoSource* q, QUrl archiveUri) :
  vsVideoSourcePrivate(q), Archive(archiveUri),
  StatusTimerId(0), StatusTtl(0)
{
  if (this->Archive.okay())
    {
    this->Status = vsDataSource::ArchivedIdle;
    this->Archive.moveToThread(this->thread());
    }
}

//-----------------------------------------------------------------------------
vsKwaVideoArchiveSourcePrivate::~vsKwaVideoArchiveSourcePrivate()
{
}

//-----------------------------------------------------------------------------
void vsKwaVideoArchiveSourcePrivate::run()
{
  // Connect archive signals
  connect(&this->Archive,
          SIGNAL(frameRangeAvailable(vtkVgTimeStamp, vtkVgTimeStamp)),
          this, SLOT(updateFrameRange(vtkVgTimeStamp, vtkVgTimeStamp)));
  connect(&this->Archive,
          SIGNAL(metadataAvailable(QList<vtkVgVideoFrameMetaData>)),
          this, SLOT(emitMetadata(QList<vtkVgVideoFrameMetaData>)));

  // Get homographies for the whole clip; needed to load tracks/events correctly
  this->Archive.initialize();

  // Hand off to event loop
  vsVideoSourcePrivate::run();
}

//-----------------------------------------------------------------------------
vsKwaVideoArchiveSource::vsKwaVideoArchiveSource(QUrl archiveUri) :
  vsVideoSource(new vsKwaVideoArchiveSourcePrivate(this, archiveUri))
{
}

//-----------------------------------------------------------------------------
vsKwaVideoArchiveSource::~vsKwaVideoArchiveSource()
{
}

//-----------------------------------------------------------------------------
QString vsKwaVideoArchiveSource::text() const
{
  QTE_D_CONST(vsKwaVideoArchiveSource);

  switch (d->Status)
    {
    case vsDataSource::ArchivedActive:
      return "A Archived";
    case vsDataSource::ArchivedIdle:
      return "(I Archived)";
    default:
      return "(none)";
    }
}

//-----------------------------------------------------------------------------
QString vsKwaVideoArchiveSource::toolTip() const
{
  QTE_D_CONST(vsKwaVideoArchiveSource);

  QString uriStr = d->Archive.uri().toString();
  switch (d->Status)
    {
    case vsDataSource::ArchivedActive:
      return "Playing archived video from " + uriStr;
    case vsDataSource::ArchivedIdle:
      return "Using archived video (idle) from " + uriStr;
    default:
      return "(no video source)";
    }
}

//-----------------------------------------------------------------------------
void vsKwaVideoArchiveSource::requestFrame(vgVideoSeekRequest request)
{
  if (QThread::currentThread() != this->thread())
    {
    QMetaObject::invokeMethod(this, "requestFrame",
                              Q_ARG(vgVideoSeekRequest, request));
    return;
    }

  QTE_D(vsKwaVideoArchiveSource);

  if (d->Status != vsDataSource::ArchivedActive)
    {
    // If not already active, change status to active
    this->updateStatus(vsDataSource::ArchivedActive);
    }

  if (d->StatusTimerId == 0)
    {
    // If not already running, start a timer to 'clear' the active status after
    // a short interval
    d->StatusTimerId = this->startTimer(150);
    }

  // Persist status for a short interval
  d->StatusTtl = 2;

  // Pass the actual request along to the superclass
  vsVideoSource::requestFrame(request);
}

//-----------------------------------------------------------------------------
void vsKwaVideoArchiveSource::timerEvent(QTimerEvent* e)
{
  QTE_D(vsKwaVideoArchiveSource);
  if (e->timerId() == d->StatusTimerId)
    {
    if ((--d->StatusTtl) <= 0)
      {
      // If our interval for active status has expired, drop back to idle and
      // kill the timer
      this->updateStatus(vsDataSource::ArchivedIdle);
      this->killTimer(e->timerId());
      d->StatusTimerId = 0;
      }
    return;
    }
  vsVideoSource::timerEvent(e);
}
