/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsCore.h"
#include "vsCorePrivate.h"

#include <QProgressDialog>

#include <vgCheckArg.h>

#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventModelCollection.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackModelCollection.h>
#include <vtkVgTypeDefs.h>
#include <vtkVgUtil.h>

#include <qtStlUtil.h>

#include <vsDescriptorSource.h>
#include <vsTrackSource.h>
#include <vsVideoSource.h>

QTE_IMPLEMENT_D_FUNC(vsCore)

//-----------------------------------------------------------------------------
vsCore::vsCore(QObject* parent)
  : QObject(parent), d_ptr(new vsCorePrivate(this))
{
  QTE_D(vsCore);

  // Register types for loaded and system-created events
  foreach (const vsEventInfo& ei, vsEventInfo::events(vsEventInfo::NonUser))
    {
    d->registerEventType(ei, false);
    }

  // Register types for user-created ('manual') events
  foreach (vsEventInfo ei, vsEventInfo::events(vsEventInfo::User))
    {
    ei.type = d->NextUserType--; // Type from settings is not used
    d->registerEventType(ei, true);
    }
}

//-----------------------------------------------------------------------------
vsCore::~vsCore()
{
}

//-----------------------------------------------------------------------------
void vsCore::registerEventType(const vsEventInfo& ei, bool isManualType)
{
  QTE_D(vsCore);

  d->registerEventType(ei, isManualType);

  // Emit updated signal, as events may have changed
  emit this->updated();
}

//-----------------------------------------------------------------------------
void vsCore::unregisterEventType(int type)
{
  QTE_D(vsCore);

  // Look up the event type
  int index = d->EventTypeRegistry->GetTypeIndex(type);
  if (index >= 0)
    {
    // Remove the type from the registry
    d->EventTypeRegistry->RemoveType(index);

    // Emit updated signal, as events may have changed
    emit this->updated();
    }
}

//-----------------------------------------------------------------------------
vsVideoSource* vsCore::videoSource() const
{
  QTE_D_CONST(vsCore);
  return d->VideoSource.data();
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsCore::videoSourceStatus() const
{
  QTE_D_CONST(vsCore);
  return (d->VideoSource ? d->VideoSource->status() : vsDataSource::NoSource);
}

//-----------------------------------------------------------------------------
QString vsCore::videoSourceText() const
{
  QTE_D_CONST(vsCore);
  return (d->VideoSource ? d->VideoSource->text() : QString("(none)"));
}

//-----------------------------------------------------------------------------
QString vsCore::videoSourceToolTip() const
{
  QTE_D_CONST(vsCore);
  return (d->VideoSource ? d->VideoSource->toolTip()
                         : QString("(no video source)"));
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsCore::trackSourceStatus(int sourceIndex) const
{
  QTE_D_CONST(vsCore);
  if (sourceIndex < 0 || sourceIndex + 1 > d->TrackSources.count())
    return vsDataSource::NoSource;
  return d->TrackSources[sourceIndex]->status();
}

//-----------------------------------------------------------------------------
QString vsCore::trackSourceText(int sourceIndex) const
{
  QTE_D_CONST(vsCore);
  if (sourceIndex < 0 || sourceIndex + 1 > d->TrackSources.count())
    return "(none)";
  return d->TrackSources[sourceIndex]->text();
}

//-----------------------------------------------------------------------------
QString vsCore::trackSourceToolTip(int sourceIndex) const
{
  QTE_D_CONST(vsCore);
  if (sourceIndex < 0 || sourceIndex + 1 > d->TrackSources.count())
    return "(no track source)";
  return d->TrackSources[sourceIndex]->toolTip();
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsCore::descriptorSourceStatus(int sourceIndex) const
{
  QTE_D_CONST(vsCore);
  if (sourceIndex < 0 || sourceIndex + 1 > d->DescriptorSources.count())
    return vsDataSource::NoSource;
  return d->DescriptorSources[sourceIndex]->status();
}

//-----------------------------------------------------------------------------
QString vsCore::descriptorSourceText(int sourceIndex) const
{
  QTE_D_CONST(vsCore);
  if (sourceIndex < 0 || sourceIndex + 1 > d->DescriptorSources.count())
    return "(none)";
  return d->DescriptorSources[sourceIndex]->text();
}

//-----------------------------------------------------------------------------
QString vsCore::descriptorSourceToolTip(int sourceIndex) const
{
  QTE_D_CONST(vsCore);
  if (sourceIndex < 0 || sourceIndex + 1 > d->DescriptorSources.count())
    return "(no descriptor source)";
  return d->DescriptorSources[sourceIndex]->toolTip();
}

//-----------------------------------------------------------------------------
void vsCore::addSources(vsSourceFactoryPtr factory)
{
  QTE_D(vsCore);

  vsVideoSourcePtr vs = factory->videoSource();
  if (vs)
    d->setVideoSource(vs);
  foreach (vsTrackSourcePtr ts, factory->trackSources())
    d->addTrackSource(ts);
  foreach (vsDescriptorSourcePtr ds, factory->descriptorSources())
    d->addDescriptorSource(ds);
}

//-----------------------------------------------------------------------------
void vsCore::connectDescriptorInputs(vsDescriptorSource* source)
{
  QTE_D(vsCore);
  d->connectDescriptorInputs(source);
}

//-----------------------------------------------------------------------------
vsDescriptorInput::Types vsCore::acceptedInputs() const
{
  QTE_D_CONST(vsCore);
  return d->CollectedDescriptorInputs;
}

//-----------------------------------------------------------------------------
void vsCore::addModel(vtkVgTrackModel* model, ModelType type)
{
  QTE_D(vsCore);
  vtkVgTrackModelCollection* collection =
    (type == vsCore::NormalModel ? d->TrackModel : d->GroundTruthTrackModel);
  model->SetPoints(collection->GetInternalTrackModel()->GetPoints());
  collection->AddTrackModel(model);
}

//-----------------------------------------------------------------------------
void vsCore::addModel(vtkVgEventModel* model, ModelType type)
{
  QTE_D(vsCore);
  vtkVgEventModelCollection* collection =
    (type == vsCore::NormalModel ? d->EventModel : d->GroundTruthEventModel);
  collection->AddEventModel(model);
}

//-----------------------------------------------------------------------------
vtkVgEventTypeRegistry* vsCore::eventTypeRegistry()
{
  QTE_D(vsCore);
  return d->EventTypeRegistry;
}

//-----------------------------------------------------------------------------
QList<vsEventInfo> vsCore::manualEventTypes() const
{
  QTE_D_CONST(vsCore);
  return d->ManualEventTypes;
}

//-----------------------------------------------------------------------------
const vgSwatchCache& vsCore::swatchCache() const
{
  QTE_D_CONST(vsCore);
  return d->SwatchCache;
}

//-----------------------------------------------------------------------------
bool vsCore::isGroundTruthDataPresent() const
{
  QTE_D_CONST(vsCore);
  return d->GroundTruthDataPresent;
}

//-----------------------------------------------------------------------------
bool vsCore::isEventGroupExpected(vsEventInfo::Group group) const
{
  QTE_D_CONST(vsCore);
  return d->ExpectedEventGroups.contains(group);
}

//-----------------------------------------------------------------------------
void vsCore::expectEventGroup(vsEventInfo::Group group)
{
  QTE_D(vsCore);
  d->expectEventGroup(group);
}

//-----------------------------------------------------------------------------
QStringList vsCore::dynamicDataSets()
{
  QTE_D(vsCore);
  QStringList sets;

  vtkVgTrackModel::SmartPtr trackModel =
    d->TrackModel->GetInternalTrackModel();
  std::vector<std::string> scalarsName = trackModel->GetAllScalarsName();
  for (size_t i = 0; i < scalarsName.size(); ++i)
    {
    sets.append(qtString(scalarsName[i]));
    }
  return sets;
}

//-----------------------------------------------------------------------------
int vsCore::createContourId()
{
  QTE_D(vsCore);
  return d->NextContourId++;
}

//-----------------------------------------------------------------------------
void vsCore::addContour(vsContour contour)
{
  QTE_D(vsCore);

  // Emit to descriptors
  vsCorePrivate::ContourInfo info(contour);
  info.inputId = d->emitInput(info.contour);

  // Add to internal map and notify interested observers
  d->Contours.insert(info.contour.id(), info);
  emit this->contourAdded(info.contour);
}

//-----------------------------------------------------------------------------
bool vsCore::setContourName(int id, QString name)
{
  QTE_D(vsCore);

  if (!d->Contours.contains(id))
    return false;

  vsCorePrivate::ContourInfo& info = d->Contours[id];
  if (info.contour.name() != name)
    {
    info.contour.setName(name);
    emit this->contourNameChanged(id, name);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vsCore::setContourType(int id, vsContour::Type newType)
{
  QTE_D(vsCore);

  if (!d->Contours.contains(id))
    return false;

  vsCorePrivate::ContourInfo& info = d->Contours[id];
  if (!d->setContourType(info, newType))
    {
    // If we can't set the type, emit that the type was changed... this is a
    // somewhat crude way to notify the region list to cancel an edit, but the
    // only way that works if the region list is going to notify us of a change
    // by emitting a signal, rather than something much more tightly coupled
    emit this->contourTypeChanged(id, info.contour.type());
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vsCore::removeContour(int id, bool removeEvents)
{
  QTE_D(vsCore);

  if (d->Contours.contains(id))
    {
    // Remove contour from internal map
    vsCorePrivate::ContourInfo info = d->Contours.take(id);

    // Signal removal to descriptors
    d->revokeInput(&vsCore::contourRevoked, info.inputId, removeEvents);

    // Done
    emit this->contourRemoved(id);
    emit this->updated();
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vsCore::convertContourToEvent(int id, int eventType, vtkVgTimeStamp ts)
{
  QTE_D(vsCore);

  if (d->Contours.contains(id))
    {
    // Get contour and points
    vsCorePrivate::ContourInfo info = d->Contours[id];
    QPolygonF points = info.contour.points();

    // Create and add event
    this->createManualEvent(eventType, points, ts);

    // Remove contour
    this->removeContour(id);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void vsCore::createManualEvent(
  int eventType, QPolygonF region, vtkVgTimeStamp ts)
{
  QTE_D(vsCore);

  // Create event from region
  vsEvent event(QUuid::createUuid());
  event->SetId(d->NextRawEventId++);
  event->AddClassifier(eventType, 1.0);

  // Only add a region if there is one present
  if (!region.isEmpty())
    {
    // Convert points to raw double array
    const int k = region.isClosed() ? region.count() - 1 : region.count();
    QScopedArrayPointer<double> points(new double[2 * k]);
    for (int i = 0; i < k; ++i)
      {
      points[(2 * i) + 0] = region[i].x();
      points[(2 * i) + 1] = region[i].y();
      }
    event->AddRegion(ts, k, points.data());
    }

  event->SetStartFrame(ts);
  event->SetEndFrame(ts);
  event->SetFlags(vtkVgEventBase::EF_UserCreated |
                  vtkVgEventBase::EF_Modifiable);

  // Notify views to show filter
  d->expectEventGroup(vsEventInfo::User);

  // Add event
  vtkIdType eventId = d->addReadyEvent(0, event);

  if (eventId != -1)
    {
    emit this->manualEventCreated(eventId);
    }
}

//-----------------------------------------------------------------------------
bool vsCore::addAlert(vsAlert alert)
{
  QTE_D(vsCore);

  // Generate new ID
  int alertId = d->NextAlertType--;
  vsCorePrivate::AlertInfo info(alert);

  // Register alert with event type registry
  info.alert.eventInfo.type = alertId;
  this->registerEventType(info.alert.eventInfo);
  d->expectEventGroup(vsEventInfo::Alert);

  // Emit alert to interested observers
  emit this->alertAdded(alertId, info.alert);
  qint64 inputId =
    d->emitInput(&vsCore::queryAvailable,
                 new vsDescriptorInput(info.alert.query, alertId));
  info.inputIdList.append(inputId);

  // Done
  d->Alerts.insert(alertId, info);
  return true;
}

//-----------------------------------------------------------------------------
void vsCore::updateAlert(int id, vsAlert updatedAlert)
{
  QTE_D(vsCore);

  if (!d->Alerts.contains(id))
    return;

  // Update internal map with the new event info
  vsCorePrivate::AlertInfo& info = d->Alerts[id];
  vsEventInfo& ei = info.alert.eventInfo = updatedAlert.eventInfo;

  // Re-register the event type
  this->registerEventType(ei);

  // Notify interested observers of the change
  emit this->alertChanged(id, updatedAlert);
}

//-----------------------------------------------------------------------------
void vsCore::setAlertEnabled(int id, bool enabled)
{
  QTE_D(vsCore);

  if (!d->Alerts.contains(id))
    return;

  vsCorePrivate::AlertInfo& info = d->Alerts[id];
  if (info.enabled != enabled)
    {
    info.enabled = enabled;
    if (enabled)
      {
      qint64 inputId =
        d->emitInput(&vsCore::queryAvailable,
                     new vsDescriptorInput(info.alert.query, id));
      info.inputIdList.append(inputId);
      }
    else
      {
      qint64 inputId = info.inputIdList.last();
      d->revokeInput(&vsCore::queryRevoked, inputId, false);
      }
    emit this->alertEnabledChanged(id, enabled);
    }
}

//-----------------------------------------------------------------------------
void vsCore::removeAlert(int id, bool removeEvents)
{
  QTE_D(vsCore);

  if (!d->Alerts.contains(id))
    return;

  // Remove from internal map
  vsCorePrivate::AlertInfo info = d->Alerts.take(id);

  // Revoke alert from descriptors
  foreach (qint64 inputId, info.inputIdList)
    d->revokeInput(&vsCore::queryRevoked, inputId, removeEvents);

  // If removing events, unregister the event type
  if (removeEvents)
    {
    int type = info.alert.eventInfo.type;
    this->unregisterEventType(type);
    }

  // Done
  emit this->alertRemoved(id, removeEvents);
}

//-----------------------------------------------------------------------------
void vsCore::updateTrackSourceStatus(vsDataSource::Status status)
{
  QTE_D(vsCore);

  if (!d->TrackSources.size())
    status = vsDataSource::NoSource;

  emit this->trackSourceStatusChanged(status);
}

//-----------------------------------------------------------------------------
void vsCore::unregisterTrackSource(vsTrackSource* source)
{
  QTE_D(vsCore);

  vsCorePrivate::vsTrackSourceIterator iter = d->TrackSources.begin();
  while (iter != d->TrackSources.end())
    {
    if (*iter == source)
      {
      iter = d->TrackSources.erase(iter);
      emit this->trackSourceStatusChanged(vsDataSource::NoSource);
      }
    else
      {
      ++iter;
      }
    }
}

//-----------------------------------------------------------------------------
void vsCore::updateDescriptorSourceStatus(vsDataSource::Status status)
{
  QTE_D(vsCore);

  if (!d->DescriptorSources.size())
    status = vsDataSource::NoSource;

  emit this->descriptorSourceStatusChanged(status);
}

//-----------------------------------------------------------------------------
void vsCore::unregisterDescriptorSource(vsDescriptorSource* source)
{
  QTE_D(vsCore);
  d->removeDescriptorSource(source);
}

//-----------------------------------------------------------------------------
void vsCore::addMetadata(QList<vtkVgVideoFrameMetaData> metadata)
{
  QTE_D(vsCore);

  foreach (const vtkVgVideoFrameMetaData& fmd, metadata)
    {
    vgTimeStamp ts = fmd.Time.GetRawTimeStamp();
    if (!d->HomographyMap.contains(ts))
      {
      // We haven't seen this metadata before; first, add the homography to our
      // internal map...
      d->HomographyMap.insert(fmd.Time.GetRawTimeStamp(), fmd.Homography);

      // ...then to the map that QF uses...
      const vtkVgVideoMetadata vmd(fmd.Time, fmd.Gsd, fmd.Homography,
                                   fmd.HomographyReferenceFrame);
      d->VideoMetadata.insert(std::make_pair(fmd.Time, vmd));

      // ...and lastly, emit to any interested descriptors
      d->emitInput(&vsCore::metaDataAvailable,
                   new vsDescriptorInput(fmd));
      }

    // Update the homography reference frame time map
    unsigned int hrf = fmd.HomographyReferenceFrame;
    vtkVgTimeStamp& hrfTime = d->HomographyReferenceTimeMap[hrf];
    if (hrfTime.IsValid())
      {
      // If the time existed already, check if we have received an earlier
      // frame with the same reference; if so, we should update the time with
      // the current earlier time
      hrfTime = qMin(hrfTime, fmd.Time);
      }
    else
      {
      // Didn't exist; set it to time of current meta data
      hrfTime = fmd.Time;
      }

    // Also save the full metadata in a map
    d->VideoFrameMetadata.insert(fmd.Time.GetRawTimeStamp(), fmd);
    }

  // Flush any deferred updates that can now be satisfied
  d->flushDeferredTrackUpdates(metadata.last().Time);
  d->flushDeferredEventRegions(metadata.last().Time);
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vsCore::homographyReferenceTime(
  unsigned int frameNumber, vtkVgTimeStamp bestGuess) const
{
  QTE_D_CONST(vsCore);
  return d->HomographyReferenceTimeMap.value(frameNumber, bestGuess);
}

//-----------------------------------------------------------------------------
void vsCore::flushUpdateSignals()
{
  QTE_D(vsCore);

  typedef vsCorePrivate::TrackUpdateSignalMap::const_iterator TrackIterator;
  foreach_iter (TrackIterator, tIter, d->PendingTrackUpdateSignals)
    {
    emit (this->*tIter.value())(tIter.key());
    }
  d->PendingTrackUpdateSignals.clear();
}

//-----------------------------------------------------------------------------
void vsCore::updateTrack(vsTrackId trackId, vvTrackState state)
{
  QTE_D(vsCore);

  QList<vvTrackState> states;
  states.append(state);

  d->updateTrack(trackId, states);
}

//-----------------------------------------------------------------------------
void vsCore::updateTrack(vsTrackId trackId, QList<vvTrackState> states)
{
  QTE_D(vsCore);
  d->updateTrack(trackId, states);
}

//-----------------------------------------------------------------------------
void vsCore::updateTrackData(vsTrackId trackId, vsTrackData data)
{
  QTE_D(vsCore);
  d->updateTrackData(trackId, data);
}

//-----------------------------------------------------------------------------
void vsCore::closeTrack(vsTrackId trackId)
{
  QTE_D(vsCore);

  if (d->DeferredTrackUpdates.contains(trackId))
    d->DeferredTrackUpdates[trackId].closed = true;
  else
    d->closeTrack(trackId);
}

//-----------------------------------------------------------------------------
void vsCore::setTrackClassification(
  vsTrackId trackId, vsTrackObjectClassifier toc)
{
  QTE_D(vsCore);

  // Set PVO on vvTrack
  vvTrack& theVvTrack = d->getVvTrack(trackId);
  theVvTrack.Classification["Fish"] = toc.probabilityFish;
  theVvTrack.Classification["Scallop"] = toc.probabilityScallop;
  theVvTrack.Classification["Other"] = toc.probabilityOther;

  // Set PVO on vtkVgTrack
  bool isNewTrack = false;
  vtkVgTrack* const track = d->track(trackId, &isNewTrack);
  track->SetPVO(toc.probabilityFish,
                toc.probabilityScallop,
                toc.probabilityOther);

  d->emitInput(&vsCore::tocAvailable,
               new vsDescriptorInput(trackId, toc));

  // d->trackLabelRepresentation_->Modified(); \FIXME must communicate this

  d->postTrackUpdateSignal(
    track, isNewTrack ? &vsCore::trackAdded : &vsCore::trackChanged);
  emit this->updated();
}

//-----------------------------------------------------------------------------
vsTrackId vsCore::logicalTrackId(vtkIdType modelTrackId) const
{
  QTE_D_CONST(vsCore);
  return d->TrackLogicalIdMap.value(modelTrackId);
}

//-----------------------------------------------------------------------------
vtkIdType vsCore::modelTrackId(const vsTrackId& trackId) const
{
  QTE_D_CONST(vsCore);
  return (d->TrackModelIdMap.value(trackId, -1));
}

//-----------------------------------------------------------------------------
vsEventId vsCore::logicalEventId(vtkIdType modelEventId) const
{
  QTE_D_CONST(vsCore);
  return d->Events.value(modelEventId);
}

//-----------------------------------------------------------------------------
vtkIdType vsCore::modelEventId(vsDescriptorSource* source,
                               vtkIdType sourceId) const
{
  QTE_D_CONST(vsCore);

  if (!d->EventMap.contains(source))
    {
    // No such source
    return -1;
    }

  const QHash<vtkIdType, vsCorePrivate::EventReference>& map =
    d->EventMap[source];

  if (!map.contains(sourceId))
    {
    // Not found
    return -1;
    }

  return map[sourceId].modelId;
}

//-----------------------------------------------------------------------------
void vsCore::setTrackNote(vtkIdType trackId, QString note)
{
  QTE_D(vsCore);

  if (vtkVgTrack* t = d->TrackModel->GetTrack(trackId))
    {
    t->SetNote(note.isEmpty() ? 0 : qPrintable(note));
    emit this->trackNoteChanged(t, note);
    vsTrackId tid = d->TrackLogicalIdMap[trackId];
    d->emitInput(&vsCore::trackNoteAvailable,
                 new vsDescriptorInput(tid, note,
                                       t->GetStartFrame().GetRawTimeStamp(),
                                       t->GetEndFrame().GetRawTimeStamp()));
    }
}

//-----------------------------------------------------------------------------
void vsCore::addEventType(
  vsDescriptorSource* source, vsEventInfo info, double initialThreshold)
{
  QTE_D(vsCore);

  // Check for existing type
  QHash<int, int>& typeMap = d->UserEventTypeMap[source];
  if (!typeMap.contains(info.type))
    {
    // Generate new type
    typeMap.insert(info.type, d->NextUserType--);
    }
  info.type = typeMap[info.type];

  // Register user type with event type registry
  this->registerEventType(info);
  d->expectEventGroup(vsEventInfo::User);

  // Emit user event type to interested observers
  emit this->userEventTypeAdded(info.type, info, initialThreshold);
}

//-----------------------------------------------------------------------------
void vsCore::addDescriptor(vsDescriptorSource* source, vsDescriptor ds)
{
  QTE_D(vsCore);
  d->addDescriptor(source, ds.take());
}

//-----------------------------------------------------------------------------
void vsCore::addDescriptors(vsDescriptorSource* source, vsDescriptorList dl)
{
  QTE_D(vsCore);
  d->addDescriptors(source, dl.take());
}

//-----------------------------------------------------------------------------
void vsCore::addEvent(vsDescriptorSource* source, vsEvent eventBase)
{
  QTE_D(vsCore);
  d->addEvent(source, eventBase);
}

//-----------------------------------------------------------------------------
void vsCore::removeEvent(vsDescriptorSource* source, vtkIdType eventId)
{
  QTE_D(vsCore);
  d->removeEvent(source, eventId);
}

//-----------------------------------------------------------------------------
void vsCore::setEventRating(vtkIdType eventId, int rating)
{
  QTE_D(vsCore);

  if (vtkVgEvent* e = d->EventModel->GetEvent(eventId))
    {
    // CAUTION: If rating changes to carry information besides IQR score, this
    //          code should be modified to check for a change in IQR score
    //          before unilaterally emitting descriptor input
    vvIqr::Classification classification;
    switch (rating)
      {
      case vgObjectStatus::Adjudicated:
        {
        classification = vvIqr::PositiveExample;
        break;
        }
      case vgObjectStatus::Excluded:
        {
        classification = vvIqr::NegativeExample;
        break;
        }
      case vgObjectStatus::None:
        {
        classification = vvIqr::UnclassifiedExample;
        break;
        }
      }
    vsEventId eid = d->Events.value(eventId);
    d->emitInput(&vsCore::eventRatingAvailable,
                 new vsDescriptorInput(eid, classification));

    e->SetStatus(rating);
    emit this->eventRatingChanged(e, rating);
    }
}

//-----------------------------------------------------------------------------
void vsCore::setEventStatus(vtkIdType eventId, int status)
{
  QTE_D(vsCore);

  if (vtkVgEvent* e = d->EventModel->GetEvent(eventId))
    {
    emit this->eventStatusChanged(e, status);
    }
}

//-----------------------------------------------------------------------------
void vsCore::setEventNote(vtkIdType eventId, QString note)
{
  QTE_D(vsCore);

  if (vtkVgEvent* e = d->EventModel->GetEvent(eventId))
    {
    e->SetNote(note.isEmpty() ? 0 : qPrintable(note));
    emit this->eventNoteChanged(e, note);
    vsEventId eid = d->Events.value(eventId);
    d->emitInput(&vsCore::eventNoteAvailable,
                 new vsDescriptorInput(eid, note,
                                       e->GetStartFrame().GetRawTimeStamp(),
                                       e->GetEndFrame().GetRawTimeStamp()));
    }
}

//-----------------------------------------------------------------------------
void vsCore::setEventStart(vtkIdType eventId, vtkVgTimeStamp ts)
{
  QTE_D(vsCore);

  vtkVgEventModelCollection* model;
  vtkVgEvent* event = d->event(eventId, &model);
  if (!(event && event->IsModifiable()))
    {
    // Punt if event not found, or does not have the modifiable flag set
    return;
    }

  // Don't allow start to be set after the first region in the event (if there
  // are regions). Rather than adding a specific interface for this purpose,
  // use the current interface and the assumption that start frame is never
  // going to be after the first region. If there are no regions in the event,
  // set the start to the value passed and adjust the end, if necessary, to
  // ensure that the end is greater than or equal to the start.
  vtkIdType npts, *pts;
  vtkVgTimeStamp timeStamp = event->GetStartFrame();
  event->GetRegionAtOrAfter(timeStamp, npts, pts);
  if (npts > 0 && ts > timeStamp)
    {
    // Clamp requested start time to no later than first region
    ts = timeStamp;
    }

  if (event->GetStartFrame() != ts)
    {
    model->Modified();
    if (ts > event->GetEndFrame())
      {
      event->SetEndFrame(ts);
      }
    event->SetStartFrame(ts);
    d->notifyEventModified(event);
    }
}

//-----------------------------------------------------------------------------
void vsCore::setEventEnd(vtkIdType eventId, vtkVgTimeStamp ts)
{
  QTE_D(vsCore);

  vtkVgEventModelCollection* model;
  vtkVgEvent* event = d->event(eventId, &model);
  if (!(event && event->IsModifiable()))
    {
    // Punt if event not found, or does not have the modifiable flag set
    return;
    }

  // Don't allow end to be set before the last region in the event (if there
  // are regions). GetClosestDisplayRegion() returns closest less than or
  // equal, or first if none are earlier, thus specifying max time as input
  // time. If there are no regions in the event, set the end to the value
  // passed and adjust the start, if necessary, to ensure that the end is
  // greater than or equal to the start.
  vtkIdType npts, *pts;
  vtkVgTimeStamp timeStamp(true);
  event->GetClosestDisplayRegion(timeStamp, npts, pts);
  if (npts > 0 && ts < timeStamp)
    {
    // Clamp requested end time to no earlier than last region
    ts = timeStamp;
    }

  if (event->GetEndFrame() != ts)
    {
    model->Modified();
    if (ts < event->GetStartFrame())
      {
      event->SetStartFrame(ts);
      }
    event->SetEndFrame(ts);
    d->notifyEventModified(event);
    }
}

//-----------------------------------------------------------------------------
void vsCore::startFollowingTrack(vtkIdType trackId)
{
  QTE_D(vsCore);
  d->FollowedTrackId = trackId;
  d->FollowedTrackTimeStamp.Time = vgTimeStamp::InvalidTime();
}

//-----------------------------------------------------------------------------
void vsCore::cancelFollowing()
{
  QTE_D(vsCore);
  d->FollowedTrackId = -1;
}

//-----------------------------------------------------------------------------
static inline bool updateGetQfDataProgress(
  QProgressDialog& dialog, int& itemsProcessed, bool* canceled)
{
  if ((++itemsProcessed % 50) == 0)
    {
    dialog.setValue(itemsProcessed);
    if (dialog.wasCanceled())
      {
      canceled && (*canceled = true);
      return false;
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
void vsCore::getQueryFormulationData(
  vtkVgTimeStamp start, vtkVgTimeStamp end, QList<vvTrack>& tracks,
  QList<vvDescriptor>& descriptors, bool* canceled)
{
  QTE_D(vsCore);

  const int totalItems = d->Tracks.count() + d->Descriptors.count();
  int itemsProcessed = 0;
  QProgressDialog progressDlg;
  progressDlg.setLabelText("Preparing data for query formulation...");
  progressDlg.setRange(0, totalItems);
  progressDlg.show();
  if (!canceled)
    {
    // If caller is not paying attention to cancellation, do not allow user to
    // cancel the operation
    progressDlg.setCancelButton(0);
    }

  // Find tracks (at least partly) within the time span
  foreach (const vvTrack& track, d->Tracks)
    {
    if (!track.Trajectory.empty() &&
        track.Trajectory.begin()->TimeStamp < end &&
        start < track.Trajectory.rbegin()->TimeStamp)
      {
      // We have a winner
      tracks.append(track);
      }

    // Update progress
    CHECK_ARG(
      updateGetQfDataProgress(progressDlg, itemsProcessed, canceled));
    }

  // Find descriptors (at least partly) within the time span
  foreach (const vvDescriptor* descriptor, d->Descriptors)
    {
    if (!descriptor->Region.empty() &&
        descriptor->Region.begin()->TimeStamp < end &&
        start < descriptor->Region.rbegin()->TimeStamp)
      {
      // We have a winner
      descriptors.append(*descriptor);
      }

    // Update progress
    CHECK_ARG(
      updateGetQfDataProgress(progressDlg, itemsProcessed, canceled));
    }
}

//-----------------------------------------------------------------------------
std::map<vtkVgTimeStamp, vtkVgVideoMetadata> vsCore::allMetadata()
{
  QTE_D(vsCore);

  return d->VideoMetadata;
}

//-----------------------------------------------------------------------------
const vtkVgVideoFrameMetaData vsCore::frameMetadata(const vtkVgTimeStamp& ts,
                                                    vg::SeekMode direction)
{
  QTE_D_CONST(vsCore);

  const vgTimeMap<vtkVgVideoFrameMetaData>::const_iterator md =
    d->VideoFrameMetadata.constFind(ts.GetRawTimeStamp(), direction);
  if (md == d->VideoFrameMetadata.constEnd())
    {
    return vtkVgVideoFrameMetaData();
    }
  return *md;
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate vsCore::imageToWorld(
  double x, double y, const vtkVgTimeStamp& ts) const
{
  QTE_D_CONST(vsCore);

  vgGeocodedCoordinate geoCoord;

  const vgTimeMap<vtkVgVideoFrameMetaData>::const_iterator md =
    d->VideoFrameMetadata.constFind(ts.GetRawTimeStamp(), vg::SeekNearest);

  if (md != d->VideoFrameMetadata.constEnd() && ts.FuzzyEquals(md.key(), 10.0))
    {
    vtkSmartPointer<vtkMatrix4x4> imageToLatLonMatrix =
      md->MakeImageToLatLonMatrix();

    if (imageToLatLonMatrix)
      {
      vtkVgApplyHomography(x, y, imageToLatLonMatrix,
                           geoCoord.Longitude, geoCoord.Latitude);
      geoCoord.GCS = md->WorldLocation.GCS;
      }
    }

  return geoCoord;
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate vsCore::stabToWorld(
  double x, double y, const vtkVgTimeStamp& ts) const
{
  QTE_D_CONST(vsCore);

  vgGeocodedCoordinate geoCoord;

  const vgTimeMap<vtkVgVideoFrameMetaData>::const_iterator md =
    d->VideoFrameMetadata.constFind(ts.GetRawTimeStamp(), vg::SeekNearest);

  if (md != d->VideoFrameMetadata.constEnd() && ts.FuzzyEquals(md.key(), 10.0))
    {
    vtkSmartPointer<vtkMatrix4x4> imageToLatLonMatrix =
      md->MakeImageToLatLonMatrix();

    if (imageToLatLonMatrix)
      {
      // First compute stabilized to image
      vtkVgInstance<vtkMatrix4x4> hmi;
      hmi->DeepCopy(md->Homography);
      hmi->Invert();
      const vgPoint2d imagePoint = vtkVgApplyHomography(x, y, hmi);

      // Now compute image to world
      vtkVgApplyHomography(imagePoint, imageToLatLonMatrix,
                           geoCoord.Longitude, geoCoord.Latitude);
      geoCoord.GCS = md->WorldLocation.GCS;
      }
    }

  return geoCoord;
}
