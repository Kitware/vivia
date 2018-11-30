/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVdfIO.h"

#include "vpVdfTrackIO.h"

#include <QUrl>

QTE_IMPLEMENT_D_FUNC(vpVdfIO)

//-----------------------------------------------------------------------------
class vpVdfEventIO : public vpEventIO
{
public:
  vpVdfEventIO(vtkVgEventModel* eventModel,
               vtkVgEventTypeRegistry* eventTypes)
    : vpEventIO{eventModel, eventTypes} {}

  ~vpVdfEventIO() {}

  virtual bool ReadEvents() QTE_OVERRIDE { return false; }
  virtual bool WriteEvents(const char*) const  QTE_OVERRIDE { return false; }
};

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
  vpTrackIO::TrackStorageMode storageMode,
  vpTrackIO::TrackTimeStampMode timeStampMode,
  vtkVgTrackTypeRegistry* trackTypes,
  vtkMatrix4x4* geoTransform, vpFrameMap* frameMap)
{
  QTE_D();

  auto* const trackIO =
    new vpVdfTrackIO{this, trackModel, storageMode, timeStampMode,
                     trackTypes, geoTransform, frameMap};
  this->TrackIO.reset(trackIO);

  trackIO->SetTracksUri(d->TracksUri);
}

//-----------------------------------------------------------------------------
void vpVdfIO::SetEventModel(vtkVgEventModel* eventModel,
                            vtkVgEventTypeRegistry* eventTypes)
{
  this->EventIO.reset(new vpVdfEventIO{eventModel, eventTypes});
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
