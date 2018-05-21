/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
