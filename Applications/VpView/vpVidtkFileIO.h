/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
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

  virtual void SetTrackModel(vtkVgTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap);

  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes);

  virtual void SetImageHeight(unsigned int imageHeight);
  virtual unsigned int GetImageHeight() const;

  void SetTracksFileName(const char* tracksFileName)
    { this->Reader.SetTracksFileName(tracksFileName); }

  void SetTrackTraitsFileName(const char* trackTraitsFileName)
    { this->Reader.SetTrackTraitsFileName(trackTraitsFileName); }

  void SetEventsFileName(const char* eventsFileName)
    { this->Reader.SetEventsFileName(eventsFileName); }

  void SetEventLinksFileName(const char* eventLinksFileName)
    { this->Reader.SetEventLinksFileName(eventLinksFileName); }

  void SetActivitiesFileName(const char* activitiesFileName)
    { this->Reader.SetActivitiesFileName(activitiesFileName); }

  void SetFseTracksFileName(const char* fseTracksFileName);

private:
  virtual vpVidtkReader& GetReader();
  virtual const vpVidtkReader& GetReader() const;

private:
  vpVidtkFileReader Reader;
  std::string FseTracksFileName;
  unsigned int ImageHeight;
};

#endif // __vpVidtkFileIO_h
