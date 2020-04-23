/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "moc_vsViperArchiveSourcePrivate.cpp"

#include <QApplication>
#include <QTimerEvent>

#include <qtMap.h>
#include <qtScopedValueChange.h>
#include <qtStlUtil.h>

#include <vgCheckArg.h>

#include <track_oracle/track_xgtf/file_format_xgtf.h>

#include <vgKwaFrameMetadata.h>
#include <vgKwaVideoClip.h>

#include <vtkVgVideoFrameMetaData.h>

#include <vsTrackInfo.h>
#include <vsTrackSource.h>
#include <vtkVsTrackInfo.h>

#include "vsViperArchiveImportOptionsDialog.h"
#include "vsViperDebug.h"

QTE_IMPLEMENT_D_FUNC(vsViperArchiveSource)

namespace // anonymous
{

//-----------------------------------------------------------------------------
vvTrackState buildState(const QRectF& head, const vtkVgTimeStamp& time)
{
  vvTrackState state;
  state.TimeStamp = time.GetRawTimeStamp();

  // Fill state image point (assume bottom center for location)
  state.ImagePoint.X = head.center().x();
  state.ImagePoint.Y = head.bottom();

  // Fill state image box
  const QRect box = head.toRect();
  state.ImageBox.TopLeft.Y = box.top();
  state.ImageBox.TopLeft.X = box.left();
  state.ImageBox.BottomRight.Y = box.bottom();
  state.ImageBox.BottomRight.X = box.right();

  // Fill state image object
  state.ImageObject.push_back(vvImagePointF(head.left(), head.top()));
  state.ImageObject.push_back(vvImagePointF(head.right(), head.top()));
  state.ImageObject.push_back(vvImagePointF(head.right(), head.bottom()));
  state.ImageObject.push_back(vvImagePointF(head.left(), head.bottom()));

  return state;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vsViperArchiveSourcePrivate::Event::Event() : Object(QUuid() /* FIXME? */)
{
}

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsViperArchiveTrackSource

//-----------------------------------------------------------------------------
class vsViperArchiveTrackSource : public vsTrackSource
{
public:
  virtual ~vsViperArchiveTrackSource() {}

  virtual Status status() const QTE_OVERRIDE;
  virtual QString text() const QTE_OVERRIDE;
  virtual QString toolTip() const QTE_OVERRIDE;

protected:
  QTE_DECLARE_PUBLIC_PTR(vsViperArchiveSourcePrivate)
  friend class vsViperArchiveSource;

  vsViperArchiveTrackSource(vsViperArchiveSourcePrivate* q) : q_ptr(q) {}

private:
  QTE_DECLARE_PUBLIC(vsViperArchiveSourcePrivate)
};

//-----------------------------------------------------------------------------
vsDataSource::Status vsViperArchiveTrackSource::status() const
{
  QTE_Q_CONST(vsViperArchiveSourcePrivate);
  return (q->Status == vsDataSource::InProcessActive
          ? vsDataSource::ArchivedActive : q->Status);
}

//-----------------------------------------------------------------------------
QString vsViperArchiveTrackSource::text() const
{
  QTE_Q_CONST(vsViperArchiveSourcePrivate);
  return q->text();
}

//-----------------------------------------------------------------------------
QString vsViperArchiveTrackSource::toolTip() const
{
  QTE_Q_CONST(vsViperArchiveSourcePrivate);
  return q->toolTip("track", "tracks");
}

//END vsViperArchiveTrackSource

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsViperArchiveSourcePrivate

//-----------------------------------------------------------------------------
vsViperArchiveSourcePrivate::vsViperArchiveSourcePrivate(
  vsViperArchiveSource* q, QUrl archiveUri)
  : q_ptr(q), ArchiveUri(archiveUri),
    StatusTimerId(0), StatusTtl(0), StatusResetNeeded(false),
    TrackSource(new vsViperArchiveTrackSource(this)), ClosureTimerId(0)
{
  this->TrackSource->moveToThread(this->thread());
}

//-----------------------------------------------------------------------------
vsViperArchiveSourcePrivate::~vsViperArchiveSourcePrivate()
{
}

//-----------------------------------------------------------------------------
vsTrackId vsViperArchiveSourcePrivate::mapTrackId(
  uint externalId, const QString& type, vsTrackId& newId)
{
  QHash<uint, vsTrackId>& map = this->ViperTrackIdMap[type];
  if (!map.contains(externalId))
    {
    ViperTrack newTrack;

    // Extract track P/V/O classification
    vsTrackObjectClassifier& toc = newTrack.Type;
    toc.probabilityPerson =
      (type.compare("Person", Qt::CaseInsensitive) ? 0.0 : 1.0);
    toc.probabilityVehicle =
      (type.compare("Vehicle", Qt::CaseInsensitive) ? 0.0 : 1.0);
    toc.probabilityOther =
      (1.0 - (toc.probabilityPerson + toc.probabilityVehicle));

    // Add new track to internal set
    ++newId.SerialNumber;
    this->ViperTracks.insert(newId, newTrack);
    map.insert(externalId, newId);
    }
  return map[externalId];
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::run()
{
  this->updateStatus(vsDataSource::ArchivedActive);

  vidtk::file_format_xgtf format;
  vidtk::track_handle_list_type xgtfTracks;
  const QString archiveFileName = this->ArchiveUri.toLocalFile();

  // Read the XGTF archive
  format.options().promote_pvmoving = false;
  bool result = format.read(stdString(archiveFileName), xgtfTracks);

  // If we are unable to read the tracks, commit suicide
  if (!result)
    {
    this->suicide();
    return;
    }

  // Extract track information and copy to internal storage in a useful form
  vsTrackId nextTrackId(vsTrackInfo::GroundTruthSource, 0, QUuid());
  vidtk::track_xgtf_type oracle;
  for (size_t n = 0, k = xgtfTracks.size(); n < k; ++n)
    {
    const vidtk::track_handle_type& trackHandle = xgtfTracks[n];
    if (trackHandle.is_valid())
      {
      // Set oracle object to track instance referenced by handle
      oracle(trackHandle);

      // Look up corresponding internal track
      const uint externalId = oracle.external_id();
      const QString type = qtString(oracle.type());
      vsTrackId tid = this->mapTrackId(externalId, type, nextTrackId);
      ViperTrack& track = this->ViperTracks[tid];

      // Extract event and add to internal track
      ViperEvent event;
      event.Type = oracle.activity();
      event.Probability = oracle.activity_probability();
      event.StartFrame = oracle.frame_span().first;
      event.EndFrame = oracle.frame_span().second;
      track.Events.append(event);

      // Extract track bounding boxes
      vidtk::frame_handle_list_type frameHandles =
        vidtk::track_oracle::get_frames(trackHandle);
      for (size_t n = 0, k = frameHandles.size(); n < k; ++n)
        {
        const vidtk::frame_handle_type& frameHandle = frameHandles[n];
        if (frameHandle.is_valid())
          {
          // Set oracle object to frame instance referenced by handle
          oracle[frameHandle];

          // Extract bounding box
          const vgl_box_2d<double>& vglBox = oracle.bounding_box();
          const QRectF box(vglBox.min_x(), vglBox.min_y(),
                           vglBox.width(), vglBox.height());

          // Add box to track
          track.Region.insert(oracle.frame_number(), box);
          }
        }
      }
    }

  // Done reading tracks; signal parent (in UI thread) to show conversion
  // options dialog
  QTE_Q(vsViperArchiveSource);
  this->updateStatus(vsDataSource::ArchivedIdle);
  QMetaObject::invokeMethod(q, "getImportOptions");

  qtThread::run();
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::dispatch(
  QUrl metaDataSource, int frameOffset, double frameRate, bool importEvents)
{
  this->updateStatus(vsDataSource::InProcessActive);

  if (importEvents)
    {
    QTE_Q(vsViperArchiveSource);
    emit q->eventGroupExpected(vsEventInfo::Classifier);
    }

  vtkIdType nextEventId = 0;

  // Recreate track and event information with adjusted frame numbers, and in
  // a data structure more conducive to how we will be processing them next
  TrackIterator tIter = this->ViperTracks.begin();
  while (tIter != this->ViperTracks.end())
    {
    vsTrackId tid = tIter.key();
    ViperTrack inTrack = tIter.value(), outTrack;
    outTrack.Type = inTrack.Type;

    // Iterate over track regions
    typedef QMap<uint, QRectF>::const_iterator RegionIterator;
    foreach_iter (RegionIterator, rIter, inTrack.Region)
      {
      // Get and check mapped frame number
      double approxFrame = (rIter.key() - frameOffset) * frameRate;
      qint64 frame = qRound64(approxFrame);
      if (approxFrame >= 0 && fabs(frame - approxFrame) < 0.1)
        {
        // Add frame to output track
        uint fnum = static_cast<uint>(approxFrame);
        outTrack.Region.insert(fnum, rIter.value());
        this->PendingTrackFrames[fnum].append(tid);
        }
      }

    if (!outTrack.Region.isEmpty())
      {
      if (importEvents)
        {
        // Iterate over events
        foreach (ViperEvent viperEvent, inTrack.Events)
          {
          Event event;
          event.Track = tid;
          vtkIdType eventId = ++nextEventId;

          // Compute mapped frame numbers
          for (uint f = viperEvent.StartFrame; f < viperEvent.EndFrame; ++f)
            {
            // Get and check mapped frame number
            double approxFrame = (f - frameOffset) * frameRate;
            qint64 frame = qRound64(approxFrame);
            if (approxFrame >= 0 && fabs(frame - approxFrame) < 0.1)
              {
              uint fnum = static_cast<uint>(approxFrame);
              if (outTrack.Region.contains(fnum))
                {
                // Add corresponding box to output event
                const QRectF& box = outTrack.Region[fnum];
                event.OutstandingFrames.insert(fnum, box);
                this->PendingEventFrames[fnum].append(eventId);
                }
              }
            }

          if (event.OutstandingFrames.empty())
            {
            // Throw out event if it has no frames
            --nextEventId;
            continue;
            }

          // Create event object and fill in what parts we have available
          vtkVgEventBase* eventObject = event.Object.GetVolatilePointer();
          event.Emitted = false;
          eventObject->SetId(eventId);
          eventObject->AddClassifier(viperEvent.Type,
                                     viperEvent.Probability, 0.0);

          // Add event to internal map
          this->Events.insert(eventId, event);
          }
        }

      // Replace track
      this->OpenTracks.insert(tid);
      tIter = this->ViperTracks.insert(tid, outTrack);
      ++tIter;

      // Go ahead and emit track's classification
      emit this->tocAvailable(tid, inTrack.Type);
      }
    else
      {
      // If frame number adjustment produced an empty track, remove it now
      tIter = this->ViperTracks.erase(tIter);
      }
    }

  if (!metaDataSource.isEmpty())
    {
    // If provided an archive meta data source, open it now and use it to get
    // time information for our data
    this->processExternalTimingInfo(metaDataSource);
    }
  else
    {
    QTE_Q(vsViperArchiveSource);

    // Otherwise, we are done until we receive frame meta data
    this->updateStatus(vsDataSource::InProcessIdle);

    // Begin accepting frame meta data
    emit q->readyForInput(q);
    }
}

//-----------------------------------------------------------------------------
QString vsViperArchiveSourcePrivate::text() const
{
  // Get appropriate format
  switch (this->Status)
    {
    case vsDataSource::InProcessActive:
    case vsDataSource::ArchivedActive:
      return "A Archived";
    case vsDataSource::ArchivedIdle:
      return "(I Archived)";
    default:
      return "(none)";
    }
}

//-----------------------------------------------------------------------------
QString vsViperArchiveSourcePrivate::toolTip(
  const QString& sourceTypeSingular, const QString& sourceTypePlural) const
{
  // Get appropriate format
  QString format;
  switch (this->Status)
    {
    case vsDataSource::InProcessActive:
      format = "Converting %1 from %2";
      break;
    case vsDataSource::ArchivedActive:
      format = "Reading %1 from %2";
      break;
    case vsDataSource::ArchivedIdle:
      format = "Using archived %1 from %2";
      break;
    default:
      return QString("(no %1 source)").arg(sourceTypeSingular);
    }

  // Return formatted text
  return format.arg(sourceTypePlural).arg(this->ArchiveUri.toString());
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::updateStatus(
  vsDataSource::Status newStatus)
{
  if (newStatus == vsDataSource::InProcessIdle)
    {
    // Persist status for a short interval
    newStatus = vsDataSource::InProcessActive;
    this->StatusTtl = 2;
    this->StatusResetNeeded = true;
    if (!this->StatusTimerId)
      {
      // If not already running, start a timer to 'clear' the active status
      // after a short interval
      this->StatusTimerId = this->startTimer(150);
      }
    }

  // Report status to parent
  QTE_Q(vsViperArchiveSource);
  QMetaObject::invokeMethod(q, "updateStatus",
                            Q_ARG(vsDataSource::Status, newStatus));
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::timerEvent(QTimerEvent* e)
{
  if (e->timerId() == this->StatusTimerId)
    {
    if ((--this->StatusTtl) <= 0)
      {
      // If our interval for active status has freshly expired, drop back to
      // idle
      if (this->StatusResetNeeded)
        {
        this->updateStatus(vsDataSource::ArchivedIdle);
        this->StatusResetNeeded = false;
        }

      // If nothing has happened for a while, kill the status timer
      if (this->StatusTtl < -10)
        {
        this->killTimer(this->StatusTimerId);
        this->StatusTimerId = 0;
        }
      }
    return;
    }
  else if (e->timerId() == this->ClosureTimerId)
    {
    this->checkClosures();
    return;
    }
  qtThread::timerEvent(e);
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::suicide()
{
  QTE_Q(vsViperArchiveSource);

  // Stop our thread
  this->quit();

  // Notify core we are done
  this->TrackSource->suicide();
  q->suicide(); // CAUTION: we may be deleted when this returns!
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::processExternalTimingInfo(
  const QUrl& source)
{
  this->updateStatus(vsDataSource::InProcessActive);

  // Attempt to open video archive
  // \TODO support other formats?
  vgKwaVideoClip clip(source);

  // Check if that succeeded; if not, die
  vgKwaVideoClip::MetadataMap metaData = clip.metadata();
  if (metaData.isEmpty())
    {
    this->suicide();
    return;
    }

  // Run resolution for each time stamp in the meta data map
  qtUtil::mapBound(metaData.keys(), this,
                   &vsViperArchiveSourcePrivate::resolve);

  // Run closure timer one to clean up anything we were not able to fully
  // resolve
  this->LastTime = (metaData.end() - 1).key();
  this->activateClosureTimer();

  this->updateStatus(vsDataSource::InProcessIdle);
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::injectInput(
  qint64 id, vsDescriptorInputPtr input)
{
  Q_UNUSED(id);

  const vtkVgVideoFrameMetaData* fmd = input->frameMetaData();
  CHECK_ARG(fmd);

  const bool active = this->resolve(fmd->Time);

  // Activate closure timer, if time has advanced
  if (!this->LastTime.IsValid() || this->LastTime < fmd->Time)
    {
    this->LastTime = fmd->Time;
    this->activateClosureTimer();
    }

  if (active)
    {
    // If we did something, reset status to idle
    this->updateStatus(vsDataSource::InProcessIdle);
    }
}

//-----------------------------------------------------------------------------
bool vsViperArchiveSourcePrivate::resolve(vtkVgTimeStamp fmdTimeStamp)
{
  const uint fmdFrame = fmdTimeStamp.GetFrameNumber();
  bool active = false;

  // Check for resolved track states
  if (this->PendingTrackFrames.contains(fmdFrame))
    {
    active = true;
    this->updateStatus(vsDataSource::InProcessActive);

    // Update all tracks that use this frame
    foreach (vsTrackId tid, this->PendingTrackFrames[fmdFrame])
      {
      Q_ASSERT(this->ViperTracks.contains(tid));
      const ViperTrack& track = this->ViperTracks[tid];
      const QRectF& head = track.Region[fmdFrame];

      // Emit track state
      const vvTrackState state = buildState(head, fmdTimeStamp);
      emit this->TrackSource->trackUpdated(tid, state);

      // Check for track closure on this frame
      if ((track.Region.end() - 1).key() == fmdFrame)
        {
        this->OpenTracks.remove(tid);
        emit this->TrackSource->trackClosed(tid);
        }
      }
    this->PendingTrackFrames.remove(fmdFrame);
    }

  // Check for resolved event regions
  if (this->PendingEventFrames.contains(fmdFrame))
    {
    active = true;
    this->updateStatus(vsDataSource::InProcessActive);

    // Update all events that use this frame
    qtUtil::mapBound(this->PendingEventFrames[fmdFrame], this,
                     &vsViperArchiveSourcePrivate::updateEvent, fmdTimeStamp);

    this->PendingEventFrames.remove(fmdFrame);
    }

  return active;
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::updateEvent(
  vtkIdType eventId, vtkVgTimeStamp fmdTimeStamp)
{
  // Sanity check #1: event must exist
  CHECK_ARG(this->Events.contains(eventId));

  Event& event = this->Events[eventId];
  const uint fmdFrame = fmdTimeStamp.GetFrameNumber();

  // Sanity check #2: event has outstanding region at this frame
  CHECK_ARG(event.OutstandingFrames.contains(fmdFrame));

  // Get box at this frame and add to event
  QRectF box = event.OutstandingFrames.take(fmdFrame);
  double points[8] =
    {
    box.left(), box.top(),
    box.right(), box.top(),
    box.right(), box.bottom(),
    box.left(), box.bottom()
    };
  vtkVgEventBase* eventObject = event.Object.GetVolatilePointer();
  eventObject->AddRegion(fmdTimeStamp, 4, points);

  // Create or update track info
  vtkVsTrackInfo* trackInfo;
  if (eventObject->GetNumberOfTracks())
    {
    trackInfo = vtkVsTrackInfo::SafeDownCast(eventObject->GetTrackInfo(0));
    Q_ASSERT(trackInfo);
    trackInfo->StartFrame = qMin(trackInfo->StartFrame, fmdTimeStamp);
    trackInfo->EndFrame = qMax(trackInfo->EndFrame, fmdTimeStamp);
    }
  else
    {
    trackInfo = new vtkVsTrackInfo(event.Track, fmdTimeStamp, fmdTimeStamp);
    eventObject->AddTrack(trackInfo);
    }

  // Update event time range
  eventObject->SetStartFrame(trackInfo->StartFrame);
  eventObject->SetEndFrame(trackInfo->EndFrame);

  // Check if event should be emitted (or re-emitted)
  if (event.OutstandingFrames.isEmpty())
    {
    // All regions have now been resolved; emit event immediately and remove
    // from overdue queue, if present
    event.Emitted = true;
    this->OverdueEvents.remove(eventId);
    emit this->eventAvailable(event.Object);
    }
  else if (event.Emitted)
    {
    qtDebug(vsdViperOverdueEvents)
      << "scheduling event" << eventId << "for re-emission";
    // We have already emitted the event, so should update it with the new
    // information, but wait a while to see if additional new regions are
    // resolved, so that we aren't re-emitting many times in quick succession
    this->OverdueEvents.insert(eventId, 2);
    this->activateClosureTimer();
    }
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::checkClosures()
{
  const uint lastFrame = this->LastTime.GetFrameNumber();

  // Check for track closures
  foreach (const vsTrackId& tid, this->OpenTracks)
    {
    Q_ASSERT(this->ViperTracks.contains(tid));
    ViperTrack& track = this->ViperTracks[tid];

    if ((track.Region.end() - 1).key() <= lastFrame)
      {
      // If we've seen past the end of the track, close it
      this->OpenTracks.remove(tid);
      emit this->TrackSource->trackClosed(tid);
      }
    }

  // Check for "expired" overdue events
  foreach (vtkIdType eventId, this->OverdueEvents.keys())
    {
    if (--this->OverdueEvents[eventId] <= 0)
      {
      Q_ASSERT(this->Events.contains(eventId));
      Event& event = this->Events[eventId];
      qtDebug(vsdViperOverdueEvents)
        << "event" << eventId << "overdue TTL expired; (re-)emitting";

      // Overdue TTL has expired; remove it from the queue and emit the event
      // in whatever is its current state
      event.Emitted = true;
      this->OverdueEvents.remove(eventId);
      emit this->eventAvailable(event.Object);
      }
    }

  // Check for newly overdue events
  typedef QHash<vtkIdType, Event>::const_iterator EventIterator;
  foreach_iter (EventIterator, iter, this->Events)
    {
    const Event& event = iter.value();
    if (!event.Emitted && !this->OverdueEvents.contains(iter.key()))
      {
      Q_ASSERT(event.OutstandingFrames.count());
      // Check if event is "overdue" to be emitted, i.e. we have seen past its
      // last frame
      if (lastFrame > (event.OutstandingFrames.end() - 1).key())
        {
        // Yes; add to overdue list to emit after a little while if the
        // remaining frames remain unresolved
        qtDebug(vsdViperOverdueEvents)
          << "event" << iter.key() << "has outstanding frame(s)"
          << event.OutstandingFrames.keys() << "and is now overdue";
        this->OverdueEvents.insert(iter.key(), 4);
        }
      }
    }

  // Check if timer should be killed
  if (this->OpenTracks.isEmpty() && this->OverdueEvents.isEmpty())
    {
    qtDebug(vsdViperOverdueEvents) << "killing closure timer";
    this->killTimer(this->ClosureTimerId);
    this->ClosureTimerId = 0;
    }
}

//-----------------------------------------------------------------------------
void vsViperArchiveSourcePrivate::activateClosureTimer()
{
  if (!this->ClosureTimerId)
    {
    // Check if we have anything to do; if there are no unresolved frames, and
    // no events waiting to be re-emitted, then there is no reason to run the
    // closure timer
    if (!this->PendingTrackFrames.isEmpty() ||
        !this->PendingEventFrames.isEmpty() ||
        !this->OverdueEvents.isEmpty())
      {
      // Start timer to check for track closures and overdue events, i.e.
      // things that depend on the most recent time we have seen that we don't
      // need (or necessarily want) to do every frame
      qtDebug(vsdViperOverdueEvents) << "starting closure timer";
      this->ClosureTimerId = this->startTimer(150);
      }
    }
}

//END vsViperArchiveSourcePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsViperArchiveSource

//-----------------------------------------------------------------------------
vsViperArchiveSource::vsViperArchiveSource(const QUrl& archiveUri) :
  d_ptr(new vsViperArchiveSourcePrivate(this, archiveUri))
{
  QTE_D(vsViperArchiveSource);

  connect(d, SIGNAL(eventAvailable(vsEvent)),
          this, SLOT(emitEvent(vsEvent)));
  connect(d, SIGNAL(eventRevoked(vtkIdType)),
          this, SLOT(revokeEvent(vtkIdType)));
  connect(d, SIGNAL(tocAvailable(vsTrackId, vsTrackObjectClassifier)),
          this, SIGNAL(tocAvailable(vsTrackId, vsTrackObjectClassifier)));

  d->Status = vsDataSource::ArchivedIdle;
}

//-----------------------------------------------------------------------------
vsViperArchiveSource::~vsViperArchiveSource()
{
  QTE_D(vsViperArchiveSource);

  // Don't emit our own destroyed() when we are going away anyway
  qtScopedBlockSignals bs(this);

  // Shut down thread and track source
  d->suicide();
  d->wait();
}

//-----------------------------------------------------------------------------
void vsViperArchiveSource::start()
{
  QTE_D(vsViperArchiveSource);

  d->start();
  vsDataSource::start();
}

//-----------------------------------------------------------------------------
vsDescriptorInput::Types vsViperArchiveSource::inputAccepted() const
{
  return vsDescriptorInput::FrameMetaData | vsDescriptorSource::inputAccepted();
}

//-----------------------------------------------------------------------------
void vsViperArchiveSource::injectInput(qint64 id, vsDescriptorInputPtr input)
{
  QTE_D(vsViperArchiveSource);

  QMetaObject::invokeMethod(d, "injectInput",
                            Q_ARG(qint64, id),
                            Q_ARG(vsDescriptorInputPtr, input));

  vsDescriptorSource::injectInput(id, input);
}

//-----------------------------------------------------------------------------
void vsViperArchiveSource::updateStatus(vsDataSource::Status newStatus)
{
  QTE_D(vsViperArchiveSource);

  if (newStatus != d->Status)
    {
    d->Status = newStatus;
    emit this->statusChanged(newStatus);
    emit d->TrackSource->statusChanged(newStatus);
    }
}

//-----------------------------------------------------------------------------
void vsViperArchiveSource::getImportOptions()
{
  QTE_D(vsViperArchiveSource);

  // Create and set up dialog
  vsViperArchiveImportOptionsDialog dlg(qApp->activeWindow());

  // Get options from user
  if (dlg.exec() != QDialog::Accepted)
    {
    d->suicide();
    return;
    }

  // Signal thread to continue processing
  QMetaObject::invokeMethod(d, "dispatch",
                            Q_ARG(QUrl, dlg.metaDataSource()),
                            Q_ARG(int, dlg.frameOffset()),
                            Q_ARG(double, dlg.frameRate()),
                            Q_ARG(bool, dlg.importEvents()));
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsViperArchiveSource::status() const
{
  QTE_D_CONST(vsViperArchiveSource);
  return (d->Status == vsDataSource::InProcessActive
          ? vsDataSource::ArchivedActive : d->Status);
}

//-----------------------------------------------------------------------------
QString vsViperArchiveSource::text() const
{
  QTE_D_CONST(vsViperArchiveSource);
  return d->text();
}

//-----------------------------------------------------------------------------
QString vsViperArchiveSource::toolTip() const
{
  QTE_D_CONST(vsViperArchiveSource);
  return d->toolTip("descriptor", "descriptors");
}

//-----------------------------------------------------------------------------
vsTrackSourcePtr vsViperArchiveSource::trackSource()
{
  QTE_D(vsViperArchiveSource);
  return d->TrackSource;
}

//END vsViperArchiveSource
