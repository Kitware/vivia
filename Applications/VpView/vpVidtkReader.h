// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpVidtkReader_h
#define __vpVidtkReader_h

#include <activity_detectors/activity.h>
#include <event_detectors/event.h>
#include <tracking_data/track.h>

class vpVidtkReader
{
public:
  vpVidtkReader() : ImageHeight(0) {}
  virtual ~vpVidtkReader() {}
  virtual bool ReadTracks(std::vector<vidtk::track_sptr>& tracks) = 0;
  virtual bool ReadEvents(std::vector<vidtk::event_sptr>& events) = 0;
  virtual bool ReadActivities(std::vector<vidtk::activity_sptr>& activities) = 0;

  void SetImageHeight(unsigned int imageHeight)
    { this->ImageHeight = imageHeight; }

  unsigned int GetImageHeight() const
    { return this->ImageHeight; }

private:
  unsigned int ImageHeight;
};

#endif // __vpVidtkReader_h
