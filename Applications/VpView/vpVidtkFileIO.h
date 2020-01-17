/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkFileIO_h
#define __vpVidtkFileIO_h

#include "vpVidtkIO.h"

#include "vpVidtkFileReader.h"

class vpVidtkFileIO : public vpVidtkIO
{
public:
  vpVidtkFileIO();
  virtual ~vpVidtkFileIO();

  virtual void SetTrackModel(vtkVpTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             bool interpolateToGround,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vgAttributeSet* trackAttributes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap);

  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes);

  virtual void SetImageHeight(unsigned int imageHeight);
  virtual unsigned int GetImageHeight() const;

  void SetTracksFileName(const QString& tracksFileName)
    { this->Reader.SetTracksFileName(tracksFileName); }

  void SetTrackTraitsFileName(const QString& trackTraitsFileName)
    { this->Reader.SetTrackTraitsFileName(trackTraitsFileName); }

  void SetTrackClassifiersFileName(const QString& trackClassifiersFileName)
    { this->Reader.SetTrackClassifiersFileName(trackClassifiersFileName); }

  void SetEventsFileName(const QString& eventsFileName)
    { this->Reader.SetEventsFileName(eventsFileName); }

  void SetEventLinksFileName(const QString& eventLinksFileName)
    { this->Reader.SetEventLinksFileName(eventLinksFileName); }

  void SetActivitiesFileName(const QString& activitiesFileName)
    { this->Reader.SetActivitiesFileName(activitiesFileName); }

  void SetFseTracksFileName(const QString& fseTracksFileName);

private:
  virtual vpVidtkReader& GetReader();
  virtual const vpVidtkReader& GetReader() const;

private:
  vpVidtkFileReader Reader;
  QString FseTracksFileName;
  unsigned int ImageHeight;
};

#endif // __vpVidtkFileIO_h
