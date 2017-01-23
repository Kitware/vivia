/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkFileReader.h"

//#include <activity_detectors/activity_reader.h>
//#include <event_detectors/event_reader_kwe.h>
#include <tracking_data/io/track_reader_process.h>

//-----------------------------------------------------------------------------
bool vpVidtkFileReader::ReadTracks(vcl_vector<vidtk::track_sptr>& tracks)
{
  if (this->TracksFileName.empty())
    {
    return false;
    }

  std::string format = this->TracksFileName.substr(
                        this->TracksFileName.rfind('.') + 1);

  vidtk::track_reader_process trackReaderProcess(
    "vpVidtkFileReader::ReadTracks");
  trackReaderProcess.set_batch_mode(true);
  vidtk::config_block processConfiguration = trackReaderProcess.params();
  processConfiguration.set("disabled", "false");
  processConfiguration.set("format", format);
  processConfiguration.set("filename", this->TracksFileName);

  if (!trackReaderProcess.set_params(processConfiguration))
    {
    return false;
    }
  if (!trackReaderProcess.initialize())
    {
    return false;
    }
  if (!trackReaderProcess.step())
    {
    return false;
    }

  // We have to use a variable to hold the returned tracks
  // because it is not a ref
  const vidtk::track::vector_t& readerTracks = trackReaderProcess.tracks();
  tracks.insert(tracks.end(),
                readerTracks.begin(),
                readerTracks.end());

  return true;
}

//-----------------------------------------------------------------------------
/*bool vpVidtkFileReader::ReadEvents(vcl_vector<vidtk::event_sptr>& events)
{
  if (this->EventsFileName.empty())
    {
    return false;
    }

  vidtk::event_reader_kwe reader;
  if (!reader.open(this->EventsFileName))
    {
    return false;
    }

  vcl_map<vidtk::event_sptr, unsigned int> stepMap;
  if (reader.read(events, stepMap))
    {
    // Build the map of event ids to events that will be needed if we read
    // activities later.
    this->EventMap.clear();
    for (size_t i = 0, size = events.size(); i < size; ++i)
      {
      this->EventMap.insert(std::make_pair(events[i]->get_id(), events[i]));
      }
    return true;
    }

  return false;
}*/

//-----------------------------------------------------------------------------
/*bool vpVidtkFileReader::ReadActivities(
  vcl_vector<vidtk::activity_sptr>& activities)
{
  if (this->ActivitiesFileName.empty())
    {
    return false;
    }

  vidtk::activity_reader reader;
  if (!reader.open(this->ActivitiesFileName))
    {
    return false;
    }

  return reader.read(activities, this->EventMap);
}*/

//-----------------------------------------------------------------------------
void vpVidtkFileReader::SetImageHeight(unsigned int imageHeight)
{
  vpVidtkReader::SetImageHeight(imageHeight);
}

//-----------------------------------------------------------------------------
unsigned int vpVidtkFileReader::GetImageHeight() const
{
  return vpVidtkReader::GetImageHeight();
}
