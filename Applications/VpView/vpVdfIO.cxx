/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVdfIO.h"

#include "vpVdfEventIO.h"
#include "vpVdfTrackIO.h"

#include <QHash>
#include <QUrl>

QTE_IMPLEMENT_D_FUNC(vpVdfIO)

//-----------------------------------------------------------------------------
class vpVdfActivityIO : public vpActivityIO
{
public:
  vpVdfActivityIO(vtkVgActivityManager* activityManager,
                  vpActivityConfig* activityConfig)
    : vpActivityIO{activityManager, activityConfig} {}

  ~vpVdfActivityIO() {}

  virtual bool ReadActivities() QTE_OVERRIDE { return false; }
};

//-----------------------------------------------------------------------------
class vpVdfIOPrivate
{
public:
  unsigned int ImageHeight;

  QUrl TracksUri;
  QString TrackTraitsFilePath;
  QString TrackClassifiersFilePath;

  QUrl EventsUri;

  QHash<long long, vtkIdType> TrackSourceIdToModelIdMap;
  QHash<long long, vtkIdType> EventSourceIdToModelIdMap;
};

//-----------------------------------------------------------------------------
vpVdfIO::vpVdfIO() : d_ptr{new vpVdfIOPrivate}
{
  QTE_D(vpVdfIO);

  d->ImageHeight = 0;
}

//-----------------------------------------------------------------------------
vpVdfIO::~vpVdfIO()
{
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetTrackModel(
  vtkVpTrackModel* trackModel,
  vpTrackIO::TrackStorageMode storageMode, bool interpolateToGround,
  vpTrackIO::TrackTimeStampMode timeStampMode,
  vtkVgTrackTypeRegistry* trackTypes, vgAttributeSet* trackAttributes,
  vtkMatrix4x4* geoTransform, vpFrameMap* frameMap)
{
  QTE_D();

  auto* const trackIO =
    new vpVdfTrackIO{this, d->TrackSourceIdToModelIdMap, trackModel,
                     storageMode, interpolateToGround, timeStampMode,
                     trackTypes, trackAttributes, geoTransform, frameMap};
  this->TrackIO.reset(trackIO);

  trackIO->SetTracksUri(d->TracksUri);
  trackIO->SetTrackTraitsFilePath(d->TrackTraitsFilePath);
  trackIO->SetTrackClassifiersFilePath(d->TrackClassifiersFilePath);
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetEventModel(vtkVgEventModel* eventModel,
                            vtkVgEventTypeRegistry* eventTypes)
{
  QTE_D();

  auto* const eventIO =
    new vpVdfEventIO{
      d->EventSourceIdToModelIdMap,
      d->TrackSourceIdToModelIdMap,
      eventModel, eventTypes};
  this->EventIO.reset(eventIO);

  eventIO->SetEventsUri(d->EventsUri);
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetActivityModel(vtkVgActivityManager* activityManager,
                               vpActivityConfig* activityConfig)
{
  this->ActivityIO.reset(new vpVdfActivityIO{activityManager, activityConfig});
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetImageHeight(unsigned int imageHeight)
{
  QTE_D();
  d->ImageHeight = imageHeight;
}

//-----------------------------------------------------------------------------
unsigned int vpVdfIO::GetImageHeight() const
{
  QTE_D();
  return d->ImageHeight;
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetTracksUri(const QUrl& uri)
{
  QTE_D();
  d->TracksUri = uri;
  if (auto* const trackIO = dynamic_cast<vpVdfTrackIO*>(this->TrackIO.get()))
    {
    trackIO->SetTracksUri(uri);
    }
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetTrackTraitsFilePath(const QString& filePath)
{
  QTE_D();
  d->TrackTraitsFilePath = filePath;
  if (auto* const trackIO = dynamic_cast<vpVdfTrackIO*>(this->TrackIO.get()))
    {
    trackIO->SetTrackTraitsFilePath(filePath);
    }
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetTrackClassifiersFilePath(const QString& filePath)
{
  QTE_D();
  d->TrackClassifiersFilePath = filePath;
  if (auto* const trackIO = dynamic_cast<vpVdfTrackIO*>(this->TrackIO.get()))
    {
    trackIO->SetTrackClassifiersFilePath(filePath);
    }
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetEventsUri(const QUrl& uri)
{
  QTE_D();
  d->EventsUri = uri;
  if (auto* const eventIO = dynamic_cast<vpVdfEventIO*>(this->EventIO.get()))
    {
    eventIO->SetEventsUri(uri);
    }
}
