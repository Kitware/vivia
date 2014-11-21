/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsCorePrivate.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include <qtMap.h>
#include <qtStlUtil.h>

#include <vtkPoints.h>

#include <vtkVgEvent.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgScalars.h>
#include <vtkVgTrack.h>
#include <vtkVgUtil.h>

#include <vtkVgEventModel.h>
#include <vtkVgEventModelCollection.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackModelCollection.h>

#include <vgDebug.h>

#include <vsDescriptorSource.h>
#include <vsTrackSource.h>
#include <vsTrackState.h>
#include <vsVideoSource.h>
#include <vtkVsTrackInfo.h>

#include "vsAlertEditor.h"
#include "vsTrackInfo.h"
#include "vsTripwireDescriptor.h"

//-----------------------------------------------------------------------------
vsCorePrivate::vsCorePrivate(vsCore* q)
  : CollectedDescriptorInputs(vsDescriptorInput::NoType),
    NextInputId(0),
    NextTrackId(0),
    NextEventId(0),
    NextRawEventId(0),
    NextContourId(1),
    NextAlertType(vsEventInfo::QueryAlert),
    NextUserType(vsEventInfo::UserType),
    GroundTruthDataPresent(false),
    PersistentAlertsLoaded(false),
    FollowedTrackId(-1),
    q_ptr(q)
{
  this->EventModel->GetInternalEventModel()->SetTrackModel(
    this->TrackModel->GetInternalTrackModel());
  this->GroundTruthEventModel->GetInternalEventModel()->SetTrackModel(
    this->GroundTruthTrackModel->GetInternalTrackModel());
}

//-----------------------------------------------------------------------------
vsCorePrivate::~vsCorePrivate()
{
  QTE_Q(vsCore);

  qDeleteAll(this->Descriptors);

  // Explicitly release all sources... this prevents us from processing their
  // destroyed() events after we have been deleted
  if (this->VideoSource)
    {
    q->disconnect(this->VideoSource.data());
    this->VideoSource.clear();
    }
  foreach (vsTrackSourcePtr ts, this->TrackSources)
    {
    q->disconnect(ts.data());
    }
  this->TrackSources.clear();
  foreach (vsDescriptorSourcePtr ds, this->DescriptorSources)
    {
    q->disconnect(ds.data());
    }
  this->DescriptorSources.clear();
}

//-----------------------------------------------------------------------------
void vsCorePrivate::registerEventType(const vsEventInfo& ei, bool isManualType)
{
  QTE_Q(vsCore);

  // Check if type is already registered
  int index = this->EventTypeRegistry->GetTypeIndex(ei.type);
  if (index >= 0)
    {
    // If so, updating the type information
    this->EventTypeRegistry->SetType(index, ei.toVgEventType());
    if (isManualType)
      {
      const int index = this->ManualEventTypesMap[ei.type];
      this->ManualEventTypes[index] = ei;
      emit q->manualEventTypesUpdated(this->ManualEventTypes);
      }
    }
  else
    {
    // Otherwise, add it
    this->EventTypeRegistry->AddType(ei.toVgEventType());
    if (isManualType)
      {
      this->ManualEventTypesMap.insert(ei.type, this->ManualEventTypes.count());
      this->ManualEventTypes.append(ei);
      emit q->manualEventTypesUpdated(this->ManualEventTypes);
      }
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::loadPersistentAlerts()
{
  QTE_Q(vsCore);

  QSettings settings;
  QStringList filePaths;
  QStringList paths = settings.value("PersistentAlerts").toStringList();

  vsAlertEditor editor;

  foreach (QString path, paths)
    {
    this->findAndStorePossibleAlerts(path, filePaths);
    }

  foreach (QString filePath, filePaths)
    {
    if (editor.loadAlert(filePath))
      {
      q->addAlert(editor.alert());
      }
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::findAndStorePossibleAlerts(
  QString path, QStringList& filePaths)
{
  QFileInfo info(path);
  if (info.isDir())
    {
    QDir dir(path);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks
                  | QDir::NoDot | QDir::NoDotDot);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
      {
      QFileInfo fileInfo = list.at(i);
      if (fileInfo.isDir())
        {
        findAndStorePossibleAlerts(fileInfo.canonicalFilePath(), filePaths);
        }
      else
        {
        filePaths.push_back(fileInfo.canonicalFilePath());
        }
      }
    }
  else
    {
    filePaths.push_back(path);
    }
}

//-----------------------------------------------------------------------------
#define CONNECT_SOURCE(_signal, _otype, _oname, _params) \
  q->connect(sourcePtr, SIGNAL(_signal _params), q, _otype(_oname _params))

//-----------------------------------------------------------------------------
void vsCorePrivate::setVideoSource(vsVideoSourcePtr source)
{
  QTE_Q(vsCore);

  // Swap our internal source reference with the new source, so that the old
  // source (which may be held as a bare pointer by anyone listening to
  // vsCore::videoSourceChanged) is not destroyed until said listeners have a
  // chance to switch over to the new source
  this->VideoSource.swap(source);
  vsVideoSource* sourcePtr = this->VideoSource.data();

  // Clear all existing tracks, events, contours, etc.
  // \TODO
  this->NextTrackId = 0;
  this->NextEventId = 0;
  this->NextRawEventId = 0;
  this->LastHomographyTimestamp.Reset();

  CONNECT_SOURCE(statusChanged, SIGNAL, videoSourceStatusChanged,
                 (vsDataSource::Status));
  CONNECT_SOURCE(frameRangeAvailable, SIGNAL, videoTimeRangeAvailableUpdated,
                 (vtkVgTimeStamp, vtkVgTimeStamp));
  CONNECT_SOURCE(metadataAvailable, SLOT, addMetadata,
                 (QList<vtkVgVideoFrameMetaData>));

  emit q->videoSourceChanged(sourcePtr);

  sourcePtr->start();
}

//-----------------------------------------------------------------------------
void vsCorePrivate::addTrackSource(vsTrackSourcePtr source)
{
  QTE_Q(vsCore);

  this->TrackSources.append(source);

  vsTrackSource* sourcePtr = source.data();
  CONNECT_SOURCE(trackUpdated, SLOT, updateTrack,
                 (vvTrackId, vvTrackState));
  CONNECT_SOURCE(trackUpdated, SLOT, updateTrack,
                 (vvTrackId, QList<vvTrackState>));
  CONNECT_SOURCE(trackDataUpdated, SLOT, updateTrackData,
                 (vvTrackId, vsTrackData));
  CONNECT_SOURCE(trackClosed, SLOT, closeTrack,
                 (vvTrackId));
  CONNECT_SOURCE(statusChanged, SLOT, updateTrackSourceStatus,
                 (vsDataSource::Status));
  CONNECT_SOURCE(destroyed, SLOT, unregisterTrackSource,
                 (vsTrackSource*));

  source->start();
}

//-----------------------------------------------------------------------------
void vsCorePrivate::addDescriptorSource(vsDescriptorSourcePtr source)
{
  QTE_Q(vsCore);

  this->DescriptorSources.append(source);

  vsDescriptorSource* sourcePtr = source.data();
  CONNECT_SOURCE(readyForInput, SLOT, connectDescriptorInputs,
                 (vsDescriptorSource*));
  CONNECT_SOURCE(eventGroupExpected, SLOT, expectEventGroup,
                 (vsEventInfo::Group));
  CONNECT_SOURCE(eventTypeAvailable, SLOT, addEventType,
                 (vsDescriptorSource*, vsEventInfo, double));
  CONNECT_SOURCE(descriptorAvailable, SLOT, addDescriptor,
                 (vsDescriptorSource*, vsDescriptor));
  CONNECT_SOURCE(descriptorsAvailable, SLOT, addDescriptors,
                 (vsDescriptorSource*, vsDescriptorList));
  CONNECT_SOURCE(eventAvailable, SLOT, addEvent,
                 (vsDescriptorSource*, vsEvent));
  CONNECT_SOURCE(eventRevoked, SLOT, removeEvent,
                 (vsDescriptorSource*, vtkIdType));
  CONNECT_SOURCE(tocAvailable, SLOT, setTrackClassification,
                 (vvTrackId, vsTrackObjectClassifier));
  CONNECT_SOURCE(statusChanged, SLOT, updateDescriptorSourceStatus,
                 (vsDataSource::Status));
  CONNECT_SOURCE(destroyed, SLOT, unregisterDescriptorSource,
                 (vsDescriptorSource*));

  source->start();
}

//-----------------------------------------------------------------------------
void vsCorePrivate::connectDescriptorInputs(vsDescriptorSource* source)
{
  // Connect descriptor inputs to core descriptor-input outputs
  vsDescriptorInput::Types inputs = source->inputAccepted();
  this->connectInput(
    source, inputs, vsDescriptorInput::FrameMetaData,
    SIGNAL(metaDataAvailable(qint64, vsDescriptorInputPtr)));
  this->connectInput(
    source, inputs, vsDescriptorInput::TrackUpdate,
    SIGNAL(trackUpdated(qint64, vsDescriptorInputPtr)));
  this->connectInput(
    source, inputs, vsDescriptorInput::TrackClosure,
    SIGNAL(trackClosed(qint64, vsDescriptorInputPtr)));
  this->connectInput(
    source, inputs, vsDescriptorInput::TrackObjectClassifier,
    SIGNAL(tocAvailable(qint64, vsDescriptorInputPtr)));
  this->connectInput(
    source, inputs, vsDescriptorInput::Event,
    SIGNAL(eventAvailable(qint64, vsDescriptorInputPtr)),
    SIGNAL(eventRevoked(qint64, bool)));
  this->connectInput(
    source, inputs, vsDescriptorInput::EventRating,
    SIGNAL(eventRatingAvailable(qint64, vsDescriptorInputPtr)));
  this->connectInput(
    source, inputs, vsDescriptorInput::EventNote,
    SIGNAL(eventNoteAvailable(qint64, vsDescriptorInputPtr)));
  this->connectInput(
    source, inputs, vsDescriptorInput::Contour,
    SIGNAL(contourAvailable(qint64, vsDescriptorInputPtr)),
    SIGNAL(contourRevoked(qint64, bool)));
  this->connectInput(
    source, inputs, vsDescriptorInput::Query,
    SIGNAL(queryAvailable(qint64, vsDescriptorInputPtr)),
    SIGNAL(queryRevoked(qint64, bool)));

  // Update merged accepted descriptor inputs
  this->DescriptorInputs[source] = inputs;
  this->updateDescriptorInputs();

  // Re-emit historic inputs
  foreach_iter (InputMap::const_iterator, iter, this->InputHistory)
    {
    if (inputs.testFlag(iter.value()->type()))
      {
      QMetaObject::invokeMethod(source, "injectInput",
                                Q_ARG(qint64, iter.key()),
                                Q_ARG(vsDescriptorInputPtr, iter.value()));
      }
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::connectInput(
  vsDescriptorSource* source,
  vsDescriptorInput::Types mask,
  vsDescriptorInput::Type type,
  const char* injectSignal,
  const char* revokeSignal)
{
  if (mask.testFlag(type))
    {
    QTE_Q(vsCore);

    q->connect(q, injectSignal, source,
               SLOT(injectInput(qint64, vsDescriptorInputPtr)));
    if (revokeSignal)
      {
      q->connect(q, revokeSignal, source,
                 SLOT(revokeInput(qint64, bool)));
      }
    }
}

//-----------------------------------------------------------------------------
qint64 vsCorePrivate::emitInput(InputSignal signal, vsDescriptorInput* input)
{
  QTE_Q(vsCore);

  // Create input
  qint64 inputId = this->NextInputId++;
  vsDescriptorInputPtr inputPtr(input);

  // Add to input history
  this->InputHistory.insert(inputId, inputPtr);

  // Emit input
  emit (q->*signal)(inputId, inputPtr);

  // Return ID of input
  return inputId;
}

//-----------------------------------------------------------------------------
qint64 vsCorePrivate::emitInput(const vsContour& contour)
{
  // If we are going to be creating a tripwire, make sure the descriptor exists
  if (contour.type() == vsContour::Tripwire)
    this->createTripwireDescriptor();

  // Emit input
  return this->emitInput(&vsCore::contourAvailable,
                         new vsDescriptorInput(contour));
}

//-----------------------------------------------------------------------------
void vsCorePrivate::revokeInput(
  RevokeInputSignal signal, qint64 inputId, bool revokeEvents)
{
  QTE_Q(vsCore);

  // Emit input revocation and remove from input history
  emit (q->*signal)(inputId, revokeEvents);
  this->InputHistory.remove(inputId);
}

//-----------------------------------------------------------------------------
void vsCorePrivate::mappedRevokeInput(
  qint64 inputId, RevokeInputSignal signal, bool revokeEvents)
{
  this->revokeInput(signal, inputId, revokeEvents);
}

//-----------------------------------------------------------------------------
void vsCorePrivate::updateDescriptorInputs()
{
  vsDescriptorInput::Types collectedInputs = vsDescriptorInput::NoType;
  foreach (vsDescriptorInput::Types input, this->DescriptorInputs.values())
    collectedInputs |= input;

  if (collectedInputs != this->CollectedDescriptorInputs)
    {
    QTE_Q(vsCore);

    this->CollectedDescriptorInputs = collectedInputs;
    emit q->acceptedInputsChanged(collectedInputs);

    if (!this->PersistentAlertsLoaded
        && collectedInputs.testFlag(vsDescriptorInput::Query))
      {
      this->loadPersistentAlerts();
      this->PersistentAlertsLoaded = true;
      }
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::expectEventGroup(vsEventInfo::Group group)
{
  QTE_Q(vsCore);
  this->ExpectedEventGroups.insert(group);
  emit q->eventGroupExpected(group);
}

//-----------------------------------------------------------------------------
void vsCorePrivate::createTripwireDescriptor()
{
  if (!this->TripwireDescriptor)
    {
    this->TripwireDescriptor =
      vsDescriptorSourcePtr(new vsTripwireDescriptor);
    this->addDescriptorSource(this->TripwireDescriptor);
    }
  this->expectEventGroup(vsEventInfo::General);
}

//-----------------------------------------------------------------------------
bool vsCorePrivate::setContourType(ContourInfo& info, vsContour::Type newType)
{
  vsContour::Type currentType = info.contour.type();

  // Anything to do?
  if (currentType != newType)
    {
    QTE_Q(vsCore);

    // Force contour closed, if required by new type
    QPolygonF points = info.contour.points();
    if (!points.isClosed() && vsContour::isLoopType(newType))
      {
      if (points.count() < 3)
        {
        // Not enough points to change type
        emit q->statusMessageAvailable("Not enough points in region");
        return false;
        }

      // Update points
      points.append(points.front());
      info.contour.setPoints(points);

      // Notify scenes of changes
      emit q->contourPointsChanged(info.contour.id(), points);
      }

    // Update the type
    info.contour.setType(newType);

    // Reissue to descriptors
    this->revokeInput(&vsCore::contourRevoked, info.inputId, true);
    info.inputId = this->emitInput(info.contour);

    // Notify scenes of changes
    emit q->contourTypeChanged(info.contour.id(), newType);
    emit q->updated();
    }

  return true;
}

//-----------------------------------------------------------------------------
vvTrack& vsCorePrivate::getVvTrack(const vvTrackId& trackId)
{
  if (!this->Tracks.contains(trackId))
    {
    vvTrack newTrack;
    newTrack.Id = trackId;
    this->Tracks.insert(trackId, newTrack);
    }

  return this->Tracks[trackId];
}

//-----------------------------------------------------------------------------
vtkVgTrack* vsCorePrivate::track(const vvTrackId& trackId)
{
  // Select appropriate model
  vtkVgTrackModelCollection* trackModel = this->TrackModel;
  if (trackId.Source == vsTrackInfo::GroundTruthSource)
    {
    this->GroundTruthDataPresent = true;
    trackModel = this->GroundTruthTrackModel;
    }

  // Return track, if it has been created already
  if (this->TrackModelIdMap.contains(trackId))
    return trackModel->GetTrack(this->TrackModelIdMap.value(trackId));

  // Track does not exist; assign a new model ID...
  vtkIdType trackModelId = this->NextTrackId++;
  this->TrackModelIdMap.insert(trackId, trackModelId);
  this->TrackLogicalIdMap.insert(trackModelId, trackId);

  // ...and create a new track
  vtkVgTrack* track = vtkVgTrack::New();
  track->SetId(trackModelId);
  QString name =
    QString("T %1:%2").arg(trackId.Source).arg(trackId.SerialNumber);
  track->SetName(qPrintable(name));
  trackModel->AddTrack(track);
  track->SetPoints(trackModel->GetInternalTrackModel()->GetPoints());
  track->FastDelete();
  this->flushDeferredEvents();

  // Return newly created track
  return track;
}

//-----------------------------------------------------------------------------
void vsCorePrivate::deferTrackUpdate(
  const vvTrackId& trackId, const vvTrackState& state)
{
  if (!this->DeferredTrackUpdates.contains(trackId))
    this->DeferredTrackUpdates.insert(trackId, DeferredTrackUpdate());

  DeferredTrackUpdate& tu = this->DeferredTrackUpdates[trackId];
  if (tu.updates.isEmpty() || tu.updates.last() < state)
    {
    tu.updates.append(state);
    }
  else
    {
    tu.updates.insert(
      qLowerBound(tu.updates.begin(), tu.updates.end(), state), state);
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::flushDeferredTrackUpdates(const vtkVgTimeStamp& ts)
{
  if (!this->LastHomographyTimestamp.IsValid()
      || this->LastHomographyTimestamp < ts)
    {
    this->LastHomographyTimestamp = ts;
    }

  // Flush any deferred updates that are older than the latest homography we
  // have received
  foreach (vvTrackId tid, this->DeferredTrackUpdates.keys())
    {
    QList<vvTrackState> states;
    DeferredTrackUpdate& tu = this->DeferredTrackUpdates[tid];
    while (!tu.updates.isEmpty()
           && tu.updates.first().TimeStamp < this->LastHomographyTimestamp)
      {
      states.append(tu.updates.takeFirst());
      }
    this->updateTrack(tid, states);
    if (tu.updates.isEmpty())
      {
      bool closed = tu.closed;
      this->DeferredTrackUpdates.remove(tid);
      if (closed)
        this->closeTrack(tid);
      }
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::updateTrack(
  const vvTrackId& trackId, const QList<vvTrackState>& states)
{
  QTE_Q(vsCore);

  vtkVgTrack* track = 0;
  vvTrack& vvTrack = this->getVvTrack(trackId);
  bool stateAccepted = false;
  bool isFirstPoint = false;
  foreach (const vvTrackState& state, states)
    {
    // Always accept the state for our internal vvTrack (used for QF), as we
    // don't need to do anything to the state for those
    vvTrack.Trajectory.insert(state);

    // Try to find a homography for this track state
    HomographyMapIterator iter =
      this->HomographyMap.constFind(state.TimeStamp, vg::SeekNearest);
    HomographyMapIterator notFound = this->HomographyMap.constEnd();

    // If no match was found...
    if (iter == notFound || iter.key() != state.TimeStamp)
      {
      // ...and we have not yet seen homographies later than this state, defer
      // the update in the hope that we will receive the needed homography at a
      // later time
      if (!this->LastHomographyTimestamp.IsValid()
          || this->LastHomographyTimestamp < state.TimeStamp)
        {
        // Defer the update
        this->deferTrackUpdate(trackId, state);
        continue;
        }

      // Otherwise, check if our 'nearest' match is good enough... we'll accept
      // a homography for a time that is off by less than expected gap between
      // frames, e.g. to compensate for rounding errors
      double delta = 1e100;
      if (iter != notFound && iter.key().HasTime()
          && state.TimeStamp.HasTime())
        {
        delta = fabs(iter.key().Time - state.TimeStamp.Time);
        }
      if (delta > 1e3) // 10 ms
        {
        // We've already seen later homographies, and don't have a "close
        // enough" match... nothing more we can do...
        // TODO: Given we can get things from streaming sources out of order,
        //       we really should also force some sort of time-based delay
        //       before giving up
        qWarning() << "warning: no homography available for time"
                   << state.TimeStamp << "- track state has been discarded";
        continue;
        }
      }

    stateAccepted = true;
    vsTrackState wstate;
    wstate.time = state.TimeStamp;

    // Use homography to get stabilized coordinates from image coordinates
    const vtkMatrix4x4& homography = iter.value();
    QVector<float> object;
    double wp[2];
    for (size_t n = 0; n < state.ImageObject.size(); ++n)
      {
      const vvImagePointF& ip = state.ImageObject[n];
      vtkVgApplyHomography(ip.X, ip.Y, homography, wp);
      object.append(wp[0]); object.append(wp[1]); object.append(0.0f);
      wstate.object.append(QPointF(wp[0], wp[1]));
      }
    vtkVgApplyHomography(state.ImagePoint.X, state.ImagePoint.Y,
                         homography, wp);
    wstate.point = QPointF(wp[0], wp[1]);

    // Accept the update
    track || (track = this->track(trackId));
    isFirstPoint = isFirstPoint || !track->GetStartFrame().IsValid();
    track->SetPoint(state.TimeStamp, wp, vtkVgGeoCoord(), object.count() / 3,
                    object.data());
    this->emitInput(&vsCore::trackUpdated,
                    new vsDescriptorInput(trackId, wstate));

    // If we're "following" a track and the id of this track matches the
    // "followed track", emit updated coordinates
    if (track->GetId() == this->FollowedTrackId)
      {
      if (!this->FollowedTrackTimeStamp.IsValid() ||
           state.TimeStamp > this->FollowedTrackTimeStamp)
        {
        this->FollowedTrackTimeStamp = state.TimeStamp;

        QMap<vgTimeStamp, vtkVgVideoFrameMetaData>::const_iterator iter =
          this->VideoFrameMetadata.find(this->FollowedTrackTimeStamp);
        if (iter != this->VideoFrameMetadata.end() && // How could it == end?
            iter.value().AreCornerPointsValid())
          {
          vtkSmartPointer<vtkMatrix4x4> imageToLatLonMatrix =
            iter.value().MakeImageToLatLonMatrix();
          if (imageToLatLonMatrix)
            {
            vgGeocodedCoordinate geoCoord;
            geoCoord.GCS = iter.value().WorldLocation.GCS;
            vtkVgApplyHomography(state.ImagePoint.X,
                                 iter.value().Height - state.ImagePoint.Y,
                                 imageToLatLonMatrix,
                                 geoCoord.Longitude, geoCoord.Latitude);
            emit q->followedTrackStateAvailable(state.TimeStamp, geoCoord);
            }
          }
        }
      }
    }

  if (stateAccepted)
    {
    QTE_Q(vsCore);

    // Notify observers that a track has been created or updated
    this->postTrackUpdateSignal(
      track, isFirstPoint ? &vsCore::trackAdded : &vsCore::trackChanged);
    emit q->updated();
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::updateTrackData(const vvTrackId &trackId,
                                    const vsTrackData& data)
{
  QTE_Q(vsCore);

  vtkVgTrack* track = this->track(trackId);

  // Loop over data sets
  foreach_iter(vsTrackData::const_iterator, dataItem, data)
    {
    // Check if scalars already exists
    vtkSmartPointer<vtkVgScalars> scalars =
      track->GetScalars(stdString(dataItem.key()));
    if(!scalars)
      {
      // No; create new one
      scalars = vtkSmartPointer<vtkVgScalars>::New();
      track->SetScalars(stdString(dataItem.key()), scalars);
      }
    // Add data
    scalars->InsertValues(dataItem.value());
    }

  emit q->updated();
}

//-----------------------------------------------------------------------------
void vsCorePrivate::closeTrack(const vvTrackId& trackId)
{
  QTE_Q(vsCore);

  this->track(trackId)->Close();
  this->emitInput(&vsCore::trackClosed,
                  new vsDescriptorInput(trackId));

  emit q->updated();
}

//-----------------------------------------------------------------------------
void vsCorePrivate::postTrackUpdateSignal(
  vtkVgTrack* track, vsCorePrivate::TrackUpdateSignal signal)
{
  // If we haven't already, queue an event to flush pending signals
  if (this->PendingTrackUpdateSignals.isEmpty())
    {
    QTE_Q(vsCore);
    QMetaObject::invokeMethod(q, "flushUpdateSignals", Qt::QueuedConnection);
    }

  if (!this->PendingTrackUpdateSignals.contains(track))
    {
    // Only insert signal if one doesn't already exist... this will both merge
    // updates, and (because we don't check what signal, and will always see
    // 'added' first) will collapse 'added' and 'changed' into just 'added'
    this->PendingTrackUpdateSignals.insert(track, signal);
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::removeDescriptorSource(vsDescriptorSource* source)
{
  QTE_Q(vsCore);

  vsDescriptorSourceIterator iter = this->DescriptorSources.begin();
  while (iter != this->DescriptorSources.end())
    {
    if (*iter == source)
      {
      iter = this->DescriptorSources.erase(iter);
      emit q->descriptorSourceStatusChanged(vsDataSource::NoSource);
      }
    else
      {
      ++iter;
      }
    }

  if (source == this->TripwireDescriptor)
    this->TripwireDescriptor.clear();

  this->DescriptorInputs.remove(source);
  this->updateDescriptorInputs();
}

//-----------------------------------------------------------------------------
bool vsCorePrivate::areEventTracksPresent(const vsEvent& event)
{
  int n = event->GetNumberOfTracks();
  while (n--)
    {
    vtkVsTrackInfo* ti = vtkVsTrackInfo::SafeDownCast(event->GetTrackInfo(n));
    if (!ti || !this->TrackModelIdMap.contains(ti->VvTrackId))
      return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
void vsCorePrivate::deferEventRegion(
  vtkVgEvent* event, const vtkVgTimeStamp& ts, const QPolygonF& region)
{
  // Find the model to which the event belongs
  vtkIdType eventId = event->GetId();
  vtkVgEventModelCollection* model = this->EventModel;
  if (event != model->GetEvent(eventId))
    {
    model = this->GroundTruthEventModel;
    if (event != model->GetEvent(eventId))
      {
      qWarning() << "warning: in vsCorePrivate::deferEventRegion:"
                    "unable to determine model for event" << eventId
                 << "- event region has been discarded";
      return;
      }
    }

  DeferredEventRegion er(model, eventId);
  er.time = ts;
  er.region = region;
  this->DeferredEventRegions.insert(ts, er);
}

//-----------------------------------------------------------------------------
void vsCorePrivate::flushDeferredEventRegions(const vtkVgTimeStamp& ts)
{
  if (!this->LastHomographyTimestamp.IsValid()
      || this->LastHomographyTimestamp < ts)
    {
    this->LastHomographyTimestamp = ts;
    }

  // Flush any deferred regions that are older than the latest homography we
  // have received
  bool updateNeeded = false;
  QMultiMap<vtkVgTimeStamp, DeferredEventRegion>::iterator iter =
    this->DeferredEventRegions.begin();
  while (iter != this->DeferredEventRegions.end() && iter.key() <= ts)
    {
    DeferredEventRegion e = iter.value();
    iter = this->DeferredEventRegions.erase(iter);
    vtkVgEvent* event = e.model->GetEvent(e.modelId);
    if (event)
      {
      const bool updated = this->addEventRegion(event, e.time, e.region);
      updateNeeded = updateNeeded || updated;
      }
    }

  // Issue an update if any regions were added
  if (updateNeeded)
    {
    QTE_Q(vsCore);
    emit q->updated();
    }
}

//-----------------------------------------------------------------------------
bool vsCorePrivate::addEventRegion(
  vtkVgEvent* event, const vtkVgTimeStamp& ts, const QPolygonF& region)
{
  HomographyMapIterator notFound = this->HomographyMap.constEnd();

  HomographyMapIterator iter =
    this->HomographyMap.constFind(ts.GetRawTimeStamp(), vg::SeekNearest);

  // If no match was found...
  if (iter == notFound || iter.key() != ts.GetRawTimeStamp())
    {
    // ...and we have not yet seen homographies later than this state, defer
    // the region in the hope that we will receive the needed homography at a
    // later time
    if (!this->LastHomographyTimestamp.IsValid()
        || this->LastHomographyTimestamp < ts)
      {
      // Defer the region
      this->deferEventRegion(event, ts, region);
      return false;
      }

    // Otherwise, check if our 'nearest' match is good enough... we'll accept
    // a homography for a time that is off by less than expected gap between
    // frames, e.g. to compensate for rounding errors
    double delta = 1e100;
    if (iter != notFound && iter.key().HasTime() && ts.HasTime())
      {
      delta = fabs(iter.key().Time - ts.GetTime());
      }
    if (delta > 1e3) // 10 ms
      {
      // We've already seen later homographies, and don't have a "close enough"
      // match... nothing more we can do...
      // TODO: Given we can get things from streaming sources out of order, we
      //       really should also force some sort of time-based delay before
      //       giving up
      qWarning() << "warning: no homography available for time"
                 << ts.GetRawTimeStamp() << "- event region has been discarded";
      return false;
      }
    }

  // Use the homography to convert the region from image coordinates to
  // stabilized coordinates
  int pointCount = region.count();
  QScopedArrayPointer<double> rawPoints(new double[2 * pointCount]);
  for (int i = 0; i < pointCount; ++i)
    {
    vtkVgApplyHomography(region[i].x(), region[i].y(), iter.value(),
                         &(rawPoints.data()[2 * i]));
    }
  event->SetRegion(ts, pointCount, rawPoints.data());

  return true;
}

//-----------------------------------------------------------------------------
void vsCorePrivate::setEventRegions(vtkVgEvent* dst, vtkVgEventBase* src)
{
  // Convert event regions from image coordinates to stabilized coordinates
  QList<QPair<vtkVgTimeStamp, QPolygonF> > regions;
  if (src->GetNumberOfRegions())
    {
    vtkVgTimeStamp ts;
    vtkIdType pointCount, *points;
    vtkPoints* eventPoints = src->GetRegionPoints();

    src->InitRegionTraversal();
    while (src->GetNextRegion(ts, pointCount, points))
      {
      // Get region points
      QPolygonF region;
      for (int i = 0; i < pointCount; ++i)
        {
        double pt[3];
        eventPoints->GetPoint(points[i], pt);
        region.append(QPointF(pt[0], pt[1]));
        }

      // Add source region to destination event
      this->addEventRegion(dst, ts, region);
      }
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::flushDeferredEvents()
{
  int i = 0;
  while (i < this->DeferredEvents.count())
    {
    if (this->areEventTracksPresent(this->DeferredEvents[i].event))
      {
      DeferredEvent e = this->DeferredEvents.takeAt(i);
      this->addReadyEvent(e.source, e.event);
      }
    else
      {
      ++i;
      }
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::addDescriptor(
  vsDescriptorSource* source, vvDescriptor* descriptor)
{
  Q_UNUSED(source);
  this->Descriptors.append(descriptor);
}

//-----------------------------------------------------------------------------
void vsCorePrivate::addDescriptors(
  vsDescriptorSource* source, const QList<vvDescriptor*>& descriptors)
{
  Q_UNUSED(source);
  this->Descriptors.append(descriptors);
}

//-----------------------------------------------------------------------------
void vsCorePrivate::addEvent(vsDescriptorSource* source, vsEvent eventBase)
{
  QHash<int, int>& typeMap = this->UserEventTypeMap[source];
  bool haveClassifiers =
    this->ExpectedEventGroups.contains(vsEventInfo::Classifier);

  const std::vector<int> classifiers = eventBase->GetClassifierTypes();

  // Remap user event types and check for classifier types
  for (size_t n = 0, k = classifiers.size(); n < k; ++n)
    {
    int classifier = classifiers[n];

    // Check if this is a user-defined event type
    if (typeMap.contains(classifier))
      {
      const int mappedType = typeMap[classifier];
      if (mappedType != classifier)
        {
        // Get classifier values and replace user type with mapped type
        const double probability = eventBase->GetProbability(classifier);
        const double normalcy = eventBase->GetNormalcy(classifier);
        eventBase->RemoveClassifier(classifier);
        eventBase->AddClassifier(mappedType, probability, normalcy);
        classifier = mappedType;
        }
      }

    // If we haven't already seen a classifier event, check if this is one
    if (!haveClassifiers && classifier > 0)
      {
      // The event has a classifier; show the filters
      this->expectEventGroup(vsEventInfo::Classifier);
      haveClassifiers = true;
      }
    }

  // Check if supporting tracks exist
  if (!this->areEventTracksPresent(eventBase))
    {
    this->DeferredEvents.append(DeferredEvent(source, eventBase));
    return;
    }

  // Everything is in place to add the event
  this->addReadyEvent(source, eventBase);
}

//-----------------------------------------------------------------------------
void vsCorePrivate::addReadyEvent(
  vsDescriptorSource* source, vsEvent eventBase)
{
  QTE_Q(vsCore);

  vtkVgEventModelCollection* eventModel = this->EventModel;
  vtkVgEventBase* eventPtr = eventBase.GetVolatilePointer();

  // Add model ID's to associated tracks
  int n = eventPtr->GetNumberOfTracks();
  while (n--)
    {
    vtkVsTrackInfo* ti =
      vtkVsTrackInfo::SafeDownCast(eventPtr->GetTrackInfo(n));
    // Cast must succeed or areEventTracksPresent would have returned false
    if (ti->VvTrackId.Source == vsTrackInfo::GroundTruthSource)
      eventModel = this->GroundTruthEventModel;
    ti->TrackId = this->TrackModelIdMap.value(ti->VvTrackId);
    }

  // Check if this is an update to a previously seen event
  vtkIdType descriptorEventId = eventPtr->GetId();
  QHash<vtkIdType, EventReference>& eventMap = this->EventMap[source];
  if (eventMap.contains(descriptorEventId))
    {
    // Yes; find and update the existing event
    EventReference& ref = eventMap[descriptorEventId];
    eventPtr->SetId(ref.modelId);

    vtkVgEvent* theEvent = ref.model->GetEvent(ref.modelId);
    theEvent->UpdateEvent(eventPtr);
    if (source) // If not a raw event...
      {
      // ...copy updated event regions, converting from image coordinates to
      // stabilized coordinates (raw event regions are already in stabilized
      // coordinates)
      this->setEventRegions(theEvent, eventPtr);
      }

    emit q->eventChanged(theEvent);
    emit q->updated();

    // Remove old instances of the event to reduce memory usage
    qtUtil::mapBound(ref.inputIds, this->InputHistory, &InputMap::remove);

    // Reemit the event to interested descriptors
    const vsEventId eid(eventBase.GetUniqueId(), ref.modelId,
                        descriptorEventId, source);
    vsDescriptorInput* const input = new vsDescriptorInput(eventBase, eid);
    ref.inputIds.append(this->emitInput(&vsCore::eventAvailable, input));

    // That's it for updated events
    return;
    }

  // Generate new global ID from source ID
  EventReference ref(eventModel, this->NextEventId++);
  eventPtr->SetId(ref.modelId);

  // Update alert matches, if this is an alert event
  if (source && eventPtr->GetNumberOfClassifiers() == 1)
    {
    eventPtr->InitClassifierTraversal();
    int classifier = eventPtr->GetClassifierType();
    if (this->Alerts.contains(classifier))
      {
      AlertInfo& ai = this->Alerts[classifier];
      ai.matchingEvents.insert(ref.modelId);
      emit q->alertMatchesChanged(classifier, ai.matchingEvents.count());
      }
    }

  // Generate new event
  vtkVgEvent* theEvent = eventModel->AddEvent(eventPtr);
  if (theEvent)
    {
    if (source) // If not a raw event...
      {
      // ...copy event regions, converting from image coordinates to stabilized
      // coordinates (raw event regions are already in stabilized coordinates)
      this->setEventRegions(theEvent, eventPtr);
      }

    // Emit notification that the event was added
    emit q->eventAdded(theEvent);
    emit q->updated();

    // Emit the event to interested descriptors and add to the map
    vsEventId eid(eventBase.GetUniqueId(), ref.modelId,
                  descriptorEventId, source);
    qint64 inputId = this->emitInput(&vsCore::eventAvailable,
                                     new vsDescriptorInput(eventBase, eid));
    ref.inputIds.append(inputId);
    eventMap.insert(descriptorEventId, ref);
    this->Events.insert(ref.modelId, eid);
    }
}

//-----------------------------------------------------------------------------
void vsCorePrivate::removeEvent(
  vsDescriptorSource* source, vtkIdType eventId)
{
  // Check if the event has been added already
  QHash<vtkIdType, EventReference>* eventMap;
  if (this->EventMap.contains(source)
      && (eventMap = &(this->EventMap[source]))->contains(eventId))
    {
    QTE_Q(vsCore);

    // The event was added; remove it from the appropriate model
    const EventReference& ref = (*eventMap)[eventId];
    ref.model->RemoveEvent(ref.modelId);
    qtUtil::mapBound(ref.inputIds, this, &vsCorePrivate::mappedRevokeInput,
                     &vsCore::eventRevoked, false);
    eventMap->remove(eventId);

    // The event may have deferred regions; if so, find them and yank them from
    // the queue
    QMultiMap<vtkVgTimeStamp, DeferredEventRegion>::iterator iter =
      this->DeferredEventRegions.begin();
    while (iter != this->DeferredEventRegions.end())
      {
      (iter->model == ref.model && iter->modelId == ref.modelId)
        ? iter = this->DeferredEventRegions.erase(iter)
        : ++iter;
      }

    // Remove from alert matches, if present
    foreach_iter (AlertMap::iterator, iter, this->Alerts)
      {
      QSet<vtkIdType>& me = iter->matchingEvents;
      if (me.contains(ref.modelId))
        {
        me.remove(ref.modelId);
        emit q->alertMatchesChanged(iter.key(), me.count());
        }
      }

    emit q->eventRemoved(ref.modelId);
    }
  else
    {
    // The event may be deferred; if so, find it and yank it from the queue
    QList<DeferredEvent>::iterator iter = this->DeferredEvents.begin();
    while (iter != this->DeferredEvents.end())
      {
      if (iter->source == source && iter->event->GetId() == eventId)
        {
        this->DeferredEvents.erase(iter);
        break;
        }
      ++iter;
      }
    }
}

//-----------------------------------------------------------------------------
vtkVgEvent* vsCorePrivate::event(
  vtkIdType id, vtkVgEventModelCollection** model)
{
  if (vtkVgEvent* event = this->EventModel->GetEvent(id))
    {
    if (model)
      {
      *model = this->EventModel;
      }
    return event;
    }

  if (vtkVgEvent* event = this->GroundTruthEventModel->GetEvent(id))
    {
    if (model)
      {
      *model = this->GroundTruthEventModel;
      }
    return event;
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vsCorePrivate::notifyEventModified(vtkVgEvent* event)
{
  if (!event)
    {
    // Shouldn't happen, but don't crash if event is bogus
    return;
    }

  QTE_Q(vsCore);

  emit q->eventChanged(event);
  emit q->updated();

  // Reemit the event to interested descriptors
  const vtkIdType eventId = event->GetId();
  if (this->Events.contains(eventId))
    {
    // Get event source and reference
    const vsEventId eid = this->Events.value(eventId);
    EventReference& ref = this->EventMap[eid.Source][eid.SourceId];

    // Remove old instances of the event to reduce memory usage
    qtUtil::mapBound(ref.inputIds, this->InputHistory, &InputMap::remove);

    // Create a copy of the event for descriptors (we can't give them our
    // internal one because it might change, and we don't already have a
    // vsEvent because we are modifying the internal event directly, not
    // through a vsEvent)
    vsEvent eventCopy(eid.UniqueId);
    eventCopy->DeepCopy(event);

    // Emit the new input and add to the reference descriptor inputs list
    vsDescriptorInput* const input = new vsDescriptorInput(eventCopy, eid);
    ref.inputIds.append(this->emitInput(&vsCore::eventAvailable, input));
    }
}
