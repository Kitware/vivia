/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsCore_h
#define __vsCore_h

#include <QObject>
#include <QList>
#include <QHash>
#include <QMap>
#include <QSet>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vtkVgTimeStamp.h>

#include <vtkVgVideoFrameMetaData.h>

#include <vsDataSource.h>
#include <vsDescriptor.h>
#include <vsDescriptorInput.h>
#include <vsEvent.h>
#include <vsEventInfo.h>
#include <vsSourceFactory.h>
#include <vsTrackClassifier.h>
#include <vsTrackData.h>

#include "vsAlert.h"
#include "vsContourWidget.h"

class QStringList;

class vgSwatchCache;

class vtkVgEvent;
class vtkVgEventModel;
class vtkVgEventTypeRegistry;
class vtkVgTrack;
class vtkVgTrackModel;
struct vtkVgVideoMetadata;


class vsDescriptorSource;
class vsTrackSource;
class vsVideoSource;

class vsCorePrivate;

class VSP_USERINTERFACE_EXPORT vsCore : public QObject
{
  Q_OBJECT

public:
  enum FilterIds
    {
    GroundTruth = -10000
    };

  enum ModelType
    {
    NormalModel,
    GroundTruthModel
    };

  vsCore(QObject* parent = 0);
  ~vsCore();

  vsVideoSource* videoSource() const;
  vsDataSource::Status videoSourceStatus() const;
  QString videoSourceText() const;
  QString videoSourceToolTip() const;

  vsDataSource::Status trackSourceStatus(int sourceIndex) const;
  QString trackSourceText(int sourceIndex) const;
  QString trackSourceToolTip(int sourceIndex) const;

  vsDataSource::Status descriptorSourceStatus(int sourceIndex) const;
  QString descriptorSourceText(int sourceIndex) const;
  QString descriptorSourceToolTip(int sourceIndex) const;

  vsDescriptorInput::Types acceptedInputs() const;

  void addModel(vtkVgTrackModel*, ModelType type = NormalModel);
  void addModel(vtkVgEventModel*, ModelType type = NormalModel);

  vtkVgEventTypeRegistry* eventTypeRegistry();
  QList<vsEventInfo> manualEventTypes() const;
  const vgSwatchCache& swatchCache() const;

  bool isGroundTruthDataPresent() const;
  bool isEventGroupExpected(vsEventInfo::Group group) const;

  QStringList dynamicDataSets();

  int createContourId();

  vtkVgTimeStamp homographyReferenceTime(unsigned int frameNumber,
                                         vtkVgTimeStamp bestGuess) const;

  vvTrackId logicalTrackId(vtkIdType modelTrackId) const;

  vtkIdType modelEventId(vsDescriptorSource* source, vtkIdType sourceId) const;

  void getQueryFormulationData(vtkVgTimeStamp start, vtkVgTimeStamp end,
                               QList<vvTrack>& tracks,
                               QList<vvDescriptor>& descriptors,
                               bool* canceled);
  std::map<vtkVgTimeStamp, vtkVgVideoMetadata> allMetadata();

signals:
  void updated();
  void statusMessageAvailable(QString message);

  void videoSourceStatusChanged(vsDataSource::Status);
  void trackSourceStatusChanged(vsDataSource::Status);
  void descriptorSourceStatusChanged(vsDataSource::Status);

  void videoSourceChanged(vsVideoSource*);
  void videoTimeRangeAvailableUpdated(vtkVgTimeStamp first,
                                      vtkVgTimeStamp last);

  void eventGroupExpected(vsEventInfo::Group);

  void eventAdded(vtkVgEvent*);
  void eventChanged(vtkVgEvent*);
  void eventRatingChanged(vtkVgEvent*, int newRating);
  void eventStatusChanged(vtkVgEvent*, int newStatus);
  void eventNoteChanged(vtkVgEvent*, QString newNote);
  void eventRemoved(vtkIdType);

  void trackAdded(vtkVgTrack*);
  void trackChanged(vtkVgTrack*);

  void contourAdded(vsContour);
  void contourTypeChanged(int id, vsContour::Type);
  void contourNameChanged(int id, QString);
  void contourPointsChanged(int id, QPolygonF);
  void contourRemoved(int id);

  void userEventTypeAdded(int id, vsEventInfo, double initialThreshold);
  void manualEventTypesUpdated(QList<vsEventInfo>);
  void manualEventCreated(vtkIdType);

  void alertAdded(int id, vsAlert);
  void alertChanged(int id, vsAlert);
  void alertMatchesChanged(int id, int matches);
  void alertEnabledChanged(int id, bool enabled);
  void alertRemoved(int id, bool unregister);

  void acceptedInputsChanged(vsDescriptorInput::Types);

  void    metaDataAvailable(qint64 id, vsDescriptorInputPtr input);
  void         trackUpdated(qint64 id, vsDescriptorInputPtr input);
  void          trackClosed(qint64 id, vsDescriptorInputPtr input);
  void         tocAvailable(qint64 id, vsDescriptorInputPtr input);
  void       eventAvailable(qint64 id, vsDescriptorInputPtr input);
  void eventRatingAvailable(qint64 id, vsDescriptorInputPtr input);
  void   eventNoteAvailable(qint64 id, vsDescriptorInputPtr input);
  void     contourAvailable(qint64 id, vsDescriptorInputPtr input);
  void       queryAvailable(qint64 id, vsDescriptorInputPtr input);

  void   eventRevoked(qint64 id, bool removeEvents);
  void contourRevoked(qint64 id, bool removeEvents);
  void   queryRevoked(qint64 id, bool removeEvents);

  void followedTrackStateAvailable(vgTimeStamp, vgGeocodedCoordinate);

public slots:
  void addContour(vsContour);
  bool setContourName(int, QString);
  bool setContourType(int, vsContour::Type);
  bool removeContour(int, bool removeEvents = true);
  bool convertContourToEvent(int, int eventType, vtkVgTimeStamp);

  void createManualEvent(int eventType, QPolygonF region, vtkVgTimeStamp);

  void setEventRating(vtkIdType, int rating);
  void setEventStatus(vtkIdType, int status);
  void setEventNote(vtkIdType, QString note);

  void setEventStart(vtkIdType, vtkVgTimeStamp);
  void setEventEnd(vtkIdType, vtkVgTimeStamp);

  bool addAlert(vsAlert);
  void updateAlert(int id, vsAlert);
  void setAlertEnabled(int id, bool enabled);
  void removeAlert(int id, bool removeEvents = true);

  void addSources(vsSourceFactoryPtr);

  void startFollowingTrack(vtkIdType trackId);
  void stopFollowingTrack();

protected slots:
  void addMetadata(QList<vtkVgVideoFrameMetaData> metadata);

  void updateTrackSourceStatus(vsDataSource::Status);
  void unregisterTrackSource(vsTrackSource*);
  void updateTrack(vvTrackId trackId, vvTrackState state);
  void updateTrack(vvTrackId trackId, QList<vvTrackState> state);
  void updateTrackData(vvTrackId trackId, vsTrackData data);
  void closeTrack(vvTrackId trackId);

  void updateDescriptorSourceStatus(vsDataSource::Status);
  void unregisterDescriptorSource(vsDescriptorSource*);
  void connectDescriptorInputs(vsDescriptorSource*);

  void expectEventGroup(vsEventInfo::Group group);
  void addEventType(vsDescriptorSource*, vsEventInfo,
                    double initialThreshold);

  void addDescriptor(vsDescriptorSource*, vsDescriptor);
  void addDescriptors(vsDescriptorSource*, vsDescriptorList);
  void addEvent(vsDescriptorSource*, vsEvent);
  void removeEvent(vsDescriptorSource*, vtkIdType);
  void setTrackClassification(vvTrackId trackId, vsTrackObjectClassifier toc);

  void flushUpdateSignals();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsCore)

  void registerEventType(const vsEventInfo&, bool isManualType = false);
  void unregisterEventType(int);

private:
  QTE_DECLARE_PRIVATE(vsCore)
  Q_DISABLE_COPY(vsCore)
};

#endif
