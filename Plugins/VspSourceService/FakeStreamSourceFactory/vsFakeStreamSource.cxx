// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "moc_vsFakeStreamSourcePrivate.cpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTimer>

#include <qtRand.h>

#include <vsDescriptorSource.h>
#include <vsSourceService.h>
#include <vsTrackSource.h>

#include <vsAdapt.h>
#include <vsVideoArchive.h>

#include "vsFakeStreamControl.h"

QTE_IMPLEMENT_D_FUNC(vsFakeStreamSource)

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsFakeStreamSourcePrivate

//-----------------------------------------------------------------------------
vsFakeStreamSourcePrivate::vsFakeStreamSourcePrivate(
  vsFakeStreamSource* q, const QUrl& streamUri)
  : vsStreamSourcePrivate(q, streamUri),
    Project(streamUri, QRegExp(" "), QRegExp("\n")), NextEventId(0),
    StreamRate(0.0), StreamJitter(0.25), StreamMaxBurstTime(5.0)
{
}

//-----------------------------------------------------------------------------
vsFakeStreamSourcePrivate::~vsFakeStreamSourcePrivate()
{
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::run()
{
  this->updateStatus(vsDataSource::StreamingPending);

  QTE_Q(vsFakeStreamSource);
  connect(this, SIGNAL(trackUpdated(vsTrackId, vvTrackState)),
          q->trackSource().data(),
          SIGNAL(trackUpdated(vsTrackId, vvTrackState)));
  connect(this, SIGNAL(trackClosed(vsTrackId)),
          q->trackSource().data(),
          SIGNAL(trackClosed(vsTrackId)));
  connect(this, SIGNAL(tocAvailable(vsTrackId, vsTrackObjectClassifier)),
          q->descriptorSource().data(),
          SIGNAL(tocAvailable(vsTrackId, vsTrackObjectClassifier)));
  connect(this, SIGNAL(eventAvailable(vsEvent)),
          q->descriptorSource().data(),
          SLOT(emitEvent(vsEvent)));
  connect(this, SIGNAL(eventRevoked(vtkIdType)),
          q->descriptorSource().data(),
          SLOT(revokeEvent(vtkIdType)));

  // Start processing first line of project
  if (this->Project.isValid())
    {
    this->processProjectLine();
    }
  else
    {
    qWarning() << "error reading project file" << this->StreamUri;
    }

  // Hand off to event loop
  vsStreamSourcePrivate::run();
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::findTime(
  vtkVgTimeStamp* result, unsigned int frameNumber, vg::SeekMode roundMode)
{
  if (this->VideoArchive)
    {
    *result = this->VideoArchive->findTime(frameNumber, roundMode);
    }
  else
    {
    *result = vtkVgTimeStamp();
    }
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::requestFrame(
  const vgVideoSeekRequest& request)
{
  if (this->VideoArchive)
    {
    // Hand request off to internal archive
    this->VideoArchive->requestFrame(request);
    }
  else
    {
    // If we have no archive, all we can do is discard the request...
    vgVtkVideoFramePtr noFrame;
    request.sendReply(noFrame);
    }
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::clearLastRequest(
  vgVideoSourceRequestor* requestor)
{
  this->VideoArchive->clearLastRequest(requestor);
}

//-----------------------------------------------------------------------------
QUrl vsFakeStreamSourcePrivate::relativeUri(QString path) const
{
  QFileInfo fi(this->StreamUri.toLocalFile());
  fi = QFileInfo(QDir(fi.canonicalPath()), path);
  return QUrl::fromLocalFile(fi.canonicalFilePath());
}

//-----------------------------------------------------------------------------
#define METADATA(_type_, _name_) \
  _type_(_name_(QList<vtkVgVideoFrameMetaData>))
void vsFakeStreamSourcePrivate::processProjectLine()
{
  if (Project.isEndOfFile())
    {
    // Done processing project
    if (this->CurrentDataSources.isEmpty())
      {
      // Not waiting on any data to queue; begin emitting frames
      this->updateStatus(vsDataSource::StreamingActive);
      this->releaseFrame();
      }
    return;
    }

  // Read next line of project file
  QString type;
  QString file;
  if (this->Project.readString(type, 0) &&
      this->Project.readString(file, 1))
    {
    QUrl uri = this->relativeUri(file);
    QRegExp vre("v(ideo)?", Qt::CaseInsensitive);
    QRegExp tre("t(racks?)?", Qt::CaseInsensitive);
    QRegExp dre("d(escriptors?)?", Qt::CaseInsensitive);

    // What type of data is being requested?
    if (vre.exactMatch(type))
      {
      // Video archive... do we have one already?
      if (this->VideoArchive)
        {
        // Ignore all but first one to load successfully
        qWarning() << "project requested video from" << file
                   << "but video is already loaded from"
                   << this->VideoArchive->uri().toLocalFile();
        qWarning() << "the previously loaded video will be used";
        }
      else
        {
        // No video archive, try to load it
        this->VideoArchive.reset(new vsVideoArchive(uri));
        if (this->VideoArchive->okay())
          {
          // Load okay, connect the archive and initialize it
          connect(this->VideoArchive.data(),
                  METADATA(SIGNAL, metadataAvailable),
                  this, METADATA(SLOT, setAvailableFrames));
          this->VideoArchive->initialize();
          }
        else
          {
          // Load failed; reset so we will try again if we see another
          qWarning() << "failed to load video archive from" << file;
          this->VideoArchive.reset();
          }
        }
      }
    else if (tre.exactMatch(type))
      {
      // Track archive
      vsSimpleSourceFactoryPtr factory =
        vsSourceService::createArchiveSource(vs::ArchiveTrackSource, uri);
      this->addArchiveSourceFactory(factory);
      }
    else if (dre.exactMatch(type))
      {
      // Descriptor archive
      vsSimpleSourceFactoryPtr factory =
        vsSourceService::createArchiveSource(vs::ArchiveDescriptorSource, uri);
      this->addArchiveSourceFactory(factory);
      }
    else
      {
      qWarning() << "unknown resource type," << type;
      }
    }
  else
    {
    qWarning() << "error reading project file at line"
               << this->Project.currentRecord();
    }

  // Continue to the next line of the project
  this->Project.nextRecord();
  this->processProjectLine();
}

//-----------------------------------------------------------------------------
#define REDIRECT(_src, _signal, _slot, _params) \
  connect(_src.data(), SIGNAL(_signal _params), \
          this, SLOT(_slot _params))
void vsFakeStreamSourcePrivate::addArchiveSourceFactory(
  vsSimpleSourceFactoryPtr& factory)
{
  if (factory)
    {
    foreach (vsTrackSourcePtr ts, factory->trackSources())
      {
      REDIRECT(ts, trackUpdated, queueTrackUpdate,
               (vsTrackId, vvTrackState));
      REDIRECT(ts, trackUpdated, queueTrackUpdate,
               (vsTrackId, QList<vvTrackState>));
      connect(ts.data(), SIGNAL(statusChanged(vsDataSource::Status)),
              this, SLOT(sourceStatusChanged()));
      this->CurrentDataSources.append(ts);
      ts->start();
      }
    foreach (vsDescriptorSourcePtr ds, factory->descriptorSources())
      {
      REDIRECT(ds, tocAvailable, queueTrackClassifier,
               (vsTrackId, vsTrackObjectClassifier));
      REDIRECT(ds, eventAvailable, queueEvent,
               (vsDescriptorSource*, vsEvent));
      connect(ds.data(), SIGNAL(statusChanged(vsDataSource::Status)),
              this, SLOT(sourceStatusChanged()));
      this->CurrentDataSources.append(ds);
      ds->start();
      }
    }
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::sourceStatusChanged()
{
  QList<QSharedPointer<vsDataSource> >::iterator iter =
    this->CurrentDataSources.begin();
  while (iter != this->CurrentDataSources.end())
    {
    // If source is done providing data...
    if ((*iter)->status() == vsDataSource::ArchivedIdle)
      {
      // ...then release it
      iter = this->CurrentDataSources.erase(iter);
      continue;
      }
    ++iter;
    }

  // When all sources are done, begin emitting frames
  if (this->CurrentDataSources.isEmpty())
    {
    this->updateStatus(vsDataSource::StreamingActive);
    this->releaseFrame();
    }
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::queueTrackUpdate(
  vsTrackId id, vvTrackState state)
{
  this->TrackUpdates[id].insert(state.TimeStamp, state);
  this->flushData(this->LastFrameReleased);
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::queueTrackUpdate(
  vsTrackId id, QList<vvTrackState> states)
{
  foreach (const vvTrackState& state, states)
    {
    this->TrackUpdates[id].insert(state.TimeStamp, state);
    }
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::queueTrackClassifier(
  vsTrackId id, vsTrackObjectClassifier toc)
{
  this->Tocs.insert(id, toc);
  this->flushData(this->LastFrameReleased);
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::queueEvent(
  vsDescriptorSource* source, vsEvent event)
{
  this->notifyClassifiersAvailable(this->Events.isEmpty());

  Q_UNUSED(source);
  event->SetId(this->NextEventId++);
  this->Events.insert(event->GetEndFrame(), event);

  this->flushData(this->LastFrameReleased);
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::releaseFrame()
{
  // Determine what frame we will release
  QMap<vtkVgTimeStamp, vtkVgVideoFrameMetaData>::const_iterator iter;
  if (!this->LastFrameReleased.IsValid())
    {
    // Not started yet; use first frame
    iter = this->VideoFrames.begin();
    this->NextFrame = iter.key();
    }
  else if (this->NextFrame.IsValid())
    {
    // Find next frame
    iter = this->VideoFrames.lowerBound(this->NextFrame);
    }
  else
    {
    // No valid next frame; don't release anything... this can happen when the
    // user requests a flush to end
    return;
    }

  if ((iter + 1) == this->VideoFrames.end())
    {
    // Next frame is the end; make sure everything is flushed, and we are done
    this->flush();
    return;
    }

  // Emit next frame set
  this->flush(this->NextFrame);

  // Determine the next time we will release
  const double skew = 2.0 * this->StreamJitter * (qtRandD() - 0.5);
  const double rate = pow(2.0, skew - this->StreamRate);
  double burst = 0.0;
  if (qtRandD() < this->StreamJitter)
    {
    burst = pow(qtRandD(), 1.7) * this->StreamMaxBurstTime;
    burst *= 1e6; // s -> ns
    }

  const double nextTime = this->NextFrame.GetTime() + burst;

  // Find next frame
  QMap<vtkVgTimeStamp, vtkVgVideoFrameMetaData>::const_iterator next = ++iter;
  do
    {
    iter = next;
    ++next;
    }
  while (next != this->VideoFrames.end() && next.key().GetTime() <= nextTime);

  // Get time until next frame
  const double delta = iter.key().GetTime() - this->NextFrame.GetTime();
  const double delay = 1e-3 * delta * rate; // convert ns -> ms

  // Schedule next frame for release
  this->NextFrame = iter.key();
  QTimer::singleShot(qRound(delay), this, SLOT(releaseFrame()));
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::setAvailableFrames(
  QList<vtkVgVideoFrameMetaData> md)
{
  foreach (const vtkVgVideoFrameMetaData& i, md)
    {
    this->VideoFrames.insert(i.Time, i);
    }
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::flush(vtkVgTimeStamp next)
{
  if (!next.IsValid())
    {
    // Flush everything
    this->NextFrame.Reset();
    this->flush(vtkVgTimeStamp(true));
    this->updateStatus(vsDataSource::StreamingStopped);
    emit this->streamingFinished(); // close control dialog
    return;
    }

  // Build list of new metadata to be released
  QList<vtkVgVideoFrameMetaData> metadata;
  QMap<vtkVgTimeStamp, vtkVgVideoFrameMetaData>::const_iterator mditer;
  if (this->LastFrameReleased.IsValid())
    {
    // Start after last released metadata
    mditer = this->VideoFrames.upperBound(this->LastFrameReleased);
    }
  else
    {
    // Haven't released anything yet; start at beginning
    mditer = this->VideoFrames.begin();
    }

  while (mditer != this->VideoFrames.end() && mditer.key() <= next)
    {
    metadata.append(mditer.value());
    ++mditer;
    }

  if (metadata.count())
    {
    // Emit new metadata
    this->emitMetadata(metadata);
    }

  // If asked to flush everything, cap claimed available range to actual range
  vtkVgTimeStamp last = next;
  if (this->VideoFrames.upperBound(next) == this->VideoFrames.end())
    {
    last = (--this->VideoFrames.end()).key();
    }

  // Emit frame availability
  vtkVgTimeStamp first = this->VideoFrames.begin().key();
  this->updateFrameRange(first, last);
  this->LastFrameReleased = last;

  // Emit track updates and events
  this->flushData(last);
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::flushData(vtkVgTimeStamp next)
{
  // Emit track updates
  foreach (vsTrackId tid, this->TrackUpdates.keys())
    {
    TrackStateMap& map = this->TrackUpdates[tid];
    TrackStateMap::iterator iter = map.begin();
    while (iter != map.end() && iter.key() <= next)
      {
      emit this->trackUpdated(tid, iter.value());
      iter = map.erase(iter);
      if (this->Tocs.contains(tid))
        {
        emit this->tocAvailable(tid, this->Tocs[tid]);
        this->Tocs.remove(tid);
        }
      }

    // If there are no remaining states...
    if (map.isEmpty())
      {
      // ...emit closure and remove from the updates hash
      emit this->trackClosed(tid);
      this->TrackUpdates.remove(tid);
      }
    }

  // Emit events
  QMap<vtkVgTimeStamp, vsEvent>::iterator eiter = this->Events.begin();
  while (eiter != this->Events.end() && eiter.key() <= next)
    {
    emit this->eventAvailable(eiter.value());
    eiter = this->Events.erase(eiter);
    }
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::setStreamRate(double newRate)
{
  this->StreamRate = newRate;
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::setStreamJitter(double newJitter)
{
  this->StreamJitter = newJitter;
}

//-----------------------------------------------------------------------------
void vsFakeStreamSourcePrivate::setStreamMaxBurstTime(double newTime)
{
  this->StreamMaxBurstTime = newTime;
}

//-----------------------------------------------------------------------------
QString vsFakeStreamSourcePrivate::text(QString format) const
{
  QFileInfo fi(this->StreamUri.toLocalFile());
  return format.arg(fi.completeBaseName());
}

//END vsFakeStreamSourcePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsFakeStreamSource

//-----------------------------------------------------------------------------
vsFakeStreamSource::vsFakeStreamSource(const QUrl& streamUri) :
  vsStreamSource(new vsFakeStreamSourcePrivate(this, streamUri))
{
  QTE_D(vsFakeStreamSource);

  vsFakeStreamControl* c = new vsFakeStreamControl;
  connect(c, SIGNAL(rateChanged(double)),
          d, SLOT(setStreamRate(double)));
  connect(c, SIGNAL(jitterChanged(double)),
          d, SLOT(setStreamJitter(double)));
  connect(c, SIGNAL(maxBurstTimeChanged(double)),
          d, SLOT(setStreamMaxBurstTime(double)));
  connect(c, SIGNAL(flushRequested()), d, SLOT(flush()));
  connect(c, SIGNAL(flushRequested()), d, SLOT(flush()));
  connect(c, SIGNAL(flushRequested()), d, SLOT(flush()));
  connect(d, SIGNAL(streamingFinished()), c, SLOT(close()));
  connect(qApp, SIGNAL(lastViewClosed()), c, SLOT(close()));
  connect(this, SIGNAL(destroyed()), c, SLOT(deleteLater()));
  c->show();
}

//-----------------------------------------------------------------------------
vsFakeStreamSource::~vsFakeStreamSource()
{
}

//END vsFakeStreamSource
