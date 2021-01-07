// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpVidtkFileReader_h
#define __vpVidtkFileReader_h

#include "vpFileReader.h"
#include "vpVidtkReader.h"

class vpVidtkFileReader : public vpFileReader, public vpVidtkReader
{
public:
  virtual bool ReadTracks(std::vector<vidtk::track_sptr>& tracks);
  virtual bool ReadEvents(std::vector<vidtk::event_sptr>& events);
  virtual bool ReadActivities(std::vector<vidtk::activity_sptr>& activities);

  virtual void SetImageHeight(unsigned int imageHeight);
  virtual unsigned int GetImageHeight() const;

private:
  std::map<unsigned int, vidtk::event_sptr> EventMap;
};

#endif // __vpVidtkFileReader_h
