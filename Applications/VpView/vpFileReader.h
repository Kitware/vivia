/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileReader_h
#define __vpFileReader_h

#include <string>

class vpFileReader
{
public:
  virtual ~vpFileReader() {}

  void SetTracksFileName(const char* tracksFileName)
    { this->TracksFileName = tracksFileName; }

  void SetTrackTraitsFileName(const char* trackTraitsFileName)
    { this->TrackTraitsFileName = trackTraitsFileName; }

  void SetEventsFileName(const char* eventsFileName)
    { this->EventsFileName = eventsFileName; }

  void SetEventLinksFileName(const char* eventLinksFileName)
    { this->EventLinksFileName = eventLinksFileName; }

  void SetActivitiesFileName(const char* activitiesFileName)
    { this->ActivitiesFileName = activitiesFileName; }

  std::string GetTracksFileName() const
    { return this->TracksFileName; }

  std::string GetTrackTraitsFileName() const
    { return this->TrackTraitsFileName; }

  std::string GetEventsFileName() const
    { return this->EventsFileName; }

  std::string GetEventLinksFileName() const
    { return this->EventLinksFileName; }

  std::string GetActivitiesFileName() const
    { return this->ActivitiesFileName; }

  virtual void SetImageHeight(unsigned int imageHeight) = 0;
  virtual unsigned int GetImageHeight() const = 0;

protected:
  std::string TracksFileName;
  std::string TrackTraitsFileName;
  std::string EventsFileName;
  std::string EventLinksFileName;
  std::string ActivitiesFileName;
};

#endif // __vpFileReader_h
