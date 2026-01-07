// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpFileReader_h
#define __vpFileReader_h

#include <QString>

class vpFileReader
{
public:
  virtual ~vpFileReader() {}

  void SetTracksFileName(const QString& tracksFileName)
    { this->TracksFileName = tracksFileName; }

  void SetTrackTraitsFileName(const QString& trackTraitsFileName)
    { this->TrackTraitsFileName = trackTraitsFileName; }

  void SetTrackClassifiersFileName(const QString& trackClassifiersFileName)
    { this->TrackClassifiersFileName = trackClassifiersFileName; }

  void SetEventsFileName(const QString& eventsFileName)
    { this->EventsFileName = eventsFileName; }

  void SetEventLinksFileName(const QString& eventLinksFileName)
    { this->EventLinksFileName = eventLinksFileName; }

  void SetActivitiesFileName(const QString& activitiesFileName)
    { this->ActivitiesFileName = activitiesFileName; }

  QString GetTracksFileName() const
    { return this->TracksFileName; }

  QString GetTrackTraitsFileName() const
    { return this->TrackTraitsFileName; }

  QString GetTrackClassifiersFileName() const
    { return this->TrackClassifiersFileName; }

  QString GetEventsFileName() const
    { return this->EventsFileName; }

  QString GetEventLinksFileName() const
    { return this->EventLinksFileName; }

  QString GetActivitiesFileName() const
    { return this->ActivitiesFileName; }

  virtual void SetImageHeight(unsigned int imageHeight) = 0;
  virtual unsigned int GetImageHeight() const = 0;

protected:
  QString TracksFileName;
  QString TrackTraitsFileName;
  QString TrackClassifiersFileName;
  QString EventsFileName;
  QString EventLinksFileName;
  QString ActivitiesFileName;
};

#endif // __vpFileReader_h
