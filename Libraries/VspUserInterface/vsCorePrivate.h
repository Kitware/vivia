// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsCorePrivate_h
#define __vsCorePrivate_h

#include <vgSwatchCache.h>
#include <vgTimeMap.h>

#include <vtkVgInstance.h>
#include <vtkVgVideoMetadata.h>

#include <vsContour.h>
#include <vsDescriptorInput.h>
#include <vsEvent.h>
#include <vsTrackId.h>

#include "vsCore.h"

class vtkMatrix4x4;

class vtkVgEvent;
class vtkVgEventModelCollection;
class vtkVgEventTypeRegistry;
class vtkVgTrack;
class vtkVgTrackModelCollection;

struct vsEventInfo;
struct vvTrack;

class vsCorePrivate
{
public:
  vsCorePrivate(vsCore*);
  ~vsCorePrivate();

  struct UserEventType
    {
    int id;
    vsEventInfo::Group group;
    };

  struct DeferredTrackUpdate
    {
    QList<vvTrackState> updates;
    bool closed;
    DeferredTrackUpdate() : closed(false) {}
    };

  struct DeferredEvent
    {
    DeferredEvent() : source(0), event(QUuid()) {}
    DeferredEvent(vsDescriptorSource* s, const vsEvent& e)
      : source(s), event(e) {}

    vsDescriptorSource* source;
    vsEvent event;
    };

  struct DeferredEventRegion
    {
    DeferredEventRegion(vtkVgEventModelCollection* m = 0, vtkIdType i = 0)
      : model(m), modelId(i) {}
    vtkVgEventModelCollection* model;
    vtkIdType modelId;
    vtkVgTimeStamp time;
    QPolygonF region;
    };

  struct EventReference
    {
    EventReference() : model(0), modelId(-1) {}
    EventReference(vtkVgEventModelCollection* m, vtkIdType i)
      : model(m), modelId(i) {}

    vtkVgEventModelCollection* model;
    vtkIdType modelId;
    QList<qint64> inputIds;
    };

  struct ContourInfo
    {
    ContourInfo() : inputId(-1) {}
    ContourInfo(const vsContour& c, qint64 iid = -1)
      : contour(c), inputId(iid) {}

    vsContour contour;
    qint64 inputId;
    };

  struct AlertInfo
    {
    AlertInfo() : enabled(true) {}
    AlertInfo(const vsAlert& a) : alert(a), enabled(true) {}

    vsAlert alert;
    QList<qint64> inputIdList;
    bool enabled;
    QSet<vtkIdType> matchingEvents;
    };

  typedef QList<vsTrackSourcePtr>::iterator vsTrackSourceIterator;
  typedef QList<vsDescriptorSourcePtr>::iterator vsDescriptorSourceIterator;

  typedef void (vsCore::*InputSignal)(qint64, vsDescriptorInputPtr);
  typedef void (vsCore::*RevokeInputSignal)(qint64, bool);
  typedef QMap<qint64, vsDescriptorInputPtr> InputMap;
  typedef QHash<int, AlertInfo> AlertMap;

  typedef vtkVgInstance<vtkMatrix4x4> HomographyMatrix;
  typedef vgTimeMap<HomographyMatrix>::const_iterator HomographyMapIterator;

  typedef void (vsCore::*TrackUpdateSignal)(vtkVgTrack*);

  void setVideoSource(vsVideoSourcePtr);
  void addTrackSource(vsTrackSourcePtr);
  void addDescriptorSource(vsDescriptorSourcePtr);
  void removeDescriptorSource(vsDescriptorSource*);

  void connectDescriptorInputs(vsDescriptorSource*);
  void connectInput(vsDescriptorSource* source,
                    vsDescriptorInput::Types mask,
                    vsDescriptorInput::Type type,
                    const char* injectSignal,
                    const char* revokeSignal = 0);
  qint64 emitInput(InputSignal signal, vsDescriptorInput* input);
  qint64 emitInput(const vsContour& contour);
  void revokeInput(RevokeInputSignal, qint64 id, bool revokeEvents);
  void mappedRevokeInput(qint64 id, RevokeInputSignal, bool revokeEvents);

  void updateDescriptorInputs();
  void expectEventGroup(vsEventInfo::Group);

  void createTripwireDescriptor();

  bool setContourType(ContourInfo&, vsContour::Type);

  vvTrack& getVvTrack(const vvTrackId&);
  vtkVgTrack* track(const vsTrackId&, bool* created = 0);
  void deferTrackUpdate(const vsTrackId&, const vvTrackState&);
  void flushDeferredTrackUpdates(const vtkVgTimeStamp&);
  void flushDeferredEvents();
  void deferEventRegion(vtkVgEvent*, const vtkVgTimeStamp&, const QPolygonF&);
  void flushDeferredEventRegions(const vtkVgTimeStamp&);

  void updateTrack(const vsTrackId&, const QList<vvTrackState>&);
  void updateTrackData(const vsTrackId& trackId, const vsTrackData& data);
  void closeTrack(const vsTrackId&);
  void postTrackUpdateSignal(vtkVgTrack*, TrackUpdateSignal);

  bool areEventTracksPresent(const vsEvent&);

  void addDescriptor(vsDescriptorSource*, vvDescriptor*);
  void addDescriptors(vsDescriptorSource*, const QList<vvDescriptor*>&);
  void addEvent(vsDescriptorSource*, vsEvent);
  vtkIdType addReadyEvent(vsDescriptorSource*, vsEvent);
  void removeEvent(vsDescriptorSource*, vtkIdType);

  vtkVgEvent* event(vtkIdType, vtkVgEventModelCollection** = 0);
  void notifyEventModified(vtkVgEvent*);

  bool addEventRegion(vtkVgEvent*, const vtkVgTimeStamp&, const QPolygonF&);
  void setEventRegions(vtkVgEvent* dst, vtkVgEventBase* src);

  void registerEventType(const vsEventInfo&, bool isManualType);

  void loadPersistentAlerts();
  void findAndStorePossibleAlerts(QString path, QStringList& filePaths);

  vsVideoSourcePtr VideoSource;
  QList<vsTrackSourcePtr> TrackSources;
  QList<vsDescriptorSourcePtr> DescriptorSources;
  vsDescriptorSourcePtr TripwireDescriptor;

  QSet<vsEventInfo::Group> ExpectedEventGroups;

  QHash<const vsDescriptorSource*, vsDescriptorInput::Types> DescriptorInputs;
  vsDescriptorInput::Types CollectedDescriptorInputs;

  qint64 NextInputId;
  InputMap InputHistory;

  vtkIdType NextTrackId;
  vtkIdType NextEventId;
  vtkIdType NextRawEventId;
  vtkVgInstance<vtkVgTrackModelCollection> TrackModel;
  vtkVgInstance<vtkVgEventModelCollection> EventModel;
  QHash<vsTrackId, vtkIdType> TrackModelIdMap;
  QHash<vtkIdType, vsTrackId> TrackLogicalIdMap;

  vtkVgInstance<vtkVgEventTypeRegistry> EventTypeRegistry;
  vgSwatchCache SwatchCache;

  QHash<int, ContourInfo> Contours;
  int NextContourId;

  AlertMap Alerts;
  int NextAlertType;

  QHash<const vsDescriptorSource*, QHash<int, UserEventType> > UserEventTypeMap;
  QList<vsEventInfo> ManualEventTypes;
  QHash<int, int> ManualEventTypesMap;
  int NextUserType;

  bool GroundTruthDataPresent;
  vtkVgInstance<vtkVgTrackModelCollection> GroundTruthTrackModel;
  vtkVgInstance<vtkVgEventModelCollection> GroundTruthEventModel;

  std::map<vtkVgTimeStamp, vtkVgVideoMetadata> VideoMetadata;

  vgTimeMap<vtkVgVideoFrameMetaData> VideoFrameMetadata;

  vgTimeMap<HomographyMatrix> HomographyMap;
  QHash<long long, vtkVgTimeStamp> HomographyReferenceTimeMap;
  vtkVgTimeStamp LastHomographyTimestamp;

  QList<vvDescriptor*> Descriptors;
  QHash<vvTrackId, vvTrack> Tracks;
  QHash<vtkIdType, vsEventId> Events;
  QHash<const vsDescriptorSource*, QHash<vtkIdType, EventReference> > EventMap;

  QHash<vsTrackId, DeferredTrackUpdate> DeferredTrackUpdates;
  QList<DeferredEvent> DeferredEvents;
  QMultiMap<vtkVgTimeStamp, DeferredEventRegion> DeferredEventRegions;

  typedef QHash<vtkVgTrack*, TrackUpdateSignal> TrackUpdateSignalMap;
  TrackUpdateSignalMap PendingTrackUpdateSignals;

  bool PersistentAlertsLoaded;

  vtkIdType FollowedTrackId;
  vgTimeStamp FollowedTrackTimeStamp;

protected:
  QTE_DECLARE_PUBLIC_PTR(vsCore)

private:
  QTE_DECLARE_PUBLIC(vsCore)
  Q_DISABLE_COPY(vsCorePrivate)
};

#endif
