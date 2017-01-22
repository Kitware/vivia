/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpVidtkReader_h
#define __vpVidtkReader_h

//#include <activity_detectors/activity.h>
//#include <event_detectors/event.h>
#include <tracking_data/track.h>

class vpVidtkReader
{
public:
  vpVidtkReader() : ImageHeight(0) {}
  virtual ~vpVidtkReader() {}
  virtual bool ReadTracks(vcl_vector<vidtk::track_sptr>& tracks) = 0;
//  virtual bool ReadEvents(vcl_vector<vidtk::event_sptr>& events) = 0;
//  virtual bool ReadActivities(vcl_vector<vidtk::activity_sptr>& activities) = 0;

  void SetImageHeight(unsigned int imageHeight)
    { this->ImageHeight = imageHeight; }

  unsigned int GetImageHeight() const
    { return this->ImageHeight; }

private:
  unsigned int ImageHeight;
};

#endif // __vpVidtkReader_h
