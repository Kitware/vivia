/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkFileReader.h"

#include <vdfArchiveSourceInfo.h>
#include <vdfDataSource.h>
#include <vdfSourceService.h>
#include <vdfTrackReader.h>

#include <vgGeodesy.h>

#include <qtGlobal.h>
#include <qtStlUtil.h>

#include <activity_detectors/activity_reader.h>
#include <event_detectors/event_reader_kwe.h>
#include <tracking_data/io/track_reader_process.h>
#include <track_oracle/file_format_manager.h>

#include <vtksys/Glob.hxx>

#include <QFileInfo>
#include <QUrl>

//-----------------------------------------------------------------------------
bool vpVidtkFileReader::ReadTracks(std::vector<vidtk::track_sptr>& outTracks)
{
  if (this->TracksFileName.isEmpty())
    {
    return false;
    }

  const auto& ext = QFileInfo{this->TracksFileName}.suffix();
  const QString& extpat = QString("*.%1").arg(ext);

  bool supportedFormat = false;
  foreach (const vdfArchivePluginInfo pluginInfo,
           vdfSourceService::archivePluginInfo())
    {
    foreach (const vdfArchiveFileType fileType, pluginInfo.SupportedFileTypes)
      {
      if (fileType.Patterns.contains(extpat))
        {
        supportedFormat = true;
        break;
        }
      }
    }

  if (supportedFormat)
    {
    vdfTrackReader reader;

    // Interpret the specifier as a glob
    vtksys::Glob glob;
    glob.FindFiles(stdString(this->TracksFileName));
    glob.SetRecurse(true);
    std::vector<std::string>& files = glob.GetFiles();
    if (files.empty())
      {
      return false;
      }

    // Read through each track file
    for (size_t i = 0, k = files.size(); i < k; ++i)
      {
      // Construct the track source and track reader
      const QUrl trackUri = QUrl::fromLocalFile(qtString(files[i]));
      QScopedPointer<vdfDataSource> source(
        vdfSourceService::createArchiveSource(trackUri));

      if (source)
        {
        reader.setSource(source.data());

        // Read tracks
        if (!reader.exec() && reader.failed())
          {
          // Track reading failed; die
          return false;
          }
        }
      }

    if (!reader.hasData())
      {
      // None of the supplied files were able to provide any data; die
      return false;
      }

    // Convert tracks to vidtk format
    typedef QHash<vdfTrackId, vdfTrackReader::Track> TrackCollection;
    const TrackCollection& inTracks = reader.tracks();
    foreach_iter (TrackCollection::const_iterator, iter, inTracks)
      {
      const vdfTrackId& id = iter.key();
      const vdfTrackReader::Track& in = iter.value();
      vidtk::track_sptr out(new vidtk::track);

      out->set_id(id.SerialNumber);

      foreach (const vvTrackState& si, in.Trajectory)
        {
        // Create state and set time stamp
        vidtk::track_state_sptr so(new vidtk::track_state);

        so->set_timestamp(vidtk::timestamp(si.TimeStamp.Time,
                                           si.TimeStamp.FrameNumber));

        // Clear unsupported attributes on state
        so->vel_[0] = so->vel_[1] = so->vel_[2] = 0.0;

        // Convert image location attributes
        so->loc_[0] = si.ImagePoint.X;
        so->loc_[1] = si.ImagePoint.Y;
        so->loc_[2] = 0.0;

        vidtk::image_object_sptr obj(new vidtk::image_object);
        obj->set_image_loc(si.ImagePoint.X, si.ImagePoint.Y);
        obj->set_bbox(si.ImageBox.TopLeft.X, si.ImageBox.BottomRight.X,
                      si.ImageBox.TopLeft.Y, si.ImageBox.BottomRight.Y);

        // Convert world location
        const vgGeocodedCoordinate& w =
          vgGeodesy::convertGcs(si.WorldLocation, vgGeodesy::LatLon_Wgs84);
        if (w.GCS != -1)
          {
          obj->set_world_loc(w.Easting, w.Northing, 0.0);
          }
        else
          {
          obj->set_world_loc(444.0, 444.0, 444.0);
          }

        const auto ai = in.Attributes.constFind(si.TimeStamp);
        if (ai != in.Attributes.constEnd())
          {
          vidtk::track_state_attributes attributes;
          foreach (const QString& attr, ai.value())
            {
            const auto a =
              vidtk::track_state_attributes::from_string(stdString(attr));
            attributes.set_attr(a); // no-op if a == _ATTR_NONE
            }
          so->set_attrs(attributes);
          }

        // Set location attributes on state and add to track
        so->set_image_object(obj);
        out->add_state(so);
        }

      outTracks.push_back(out);
      }
    }
  else
    {
    vidtk::track_reader_process trackReaderProcess(
      "vpVidtkFileReader::ReadTracks");
    trackReaderProcess.set_batch_mode(true);
    vidtk::config_block processConfiguration = trackReaderProcess.params();
    processConfiguration.set("disabled", "false");
    processConfiguration.set("format", stdString(ext));
    processConfiguration.set("filename", stdString(this->TracksFileName));

    if (!trackReaderProcess.set_params(processConfiguration))
      {
      return false;
      }
    if (!trackReaderProcess.initialize())
      {
      return false;
      }
    if (!trackReaderProcess.step2())
      {
      return false;
      }

    // We have to use a variable to hold the returned tracks
    // because it is not a ref
    const vidtk::track::vector_t& readerTracks = trackReaderProcess.tracks();
    outTracks.insert(outTracks.end(),
                     readerTracks.begin(),
                     readerTracks.end());
    }
  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkFileReader::ReadEvents(std::vector<vidtk::event_sptr>& events)
{
  if (this->EventsFileName.isEmpty())
    {
    return false;
    }

  vidtk::event_reader_kwe reader;
  if (!reader.open(stdString(this->EventsFileName)))
    {
    return false;
    }

  std::map<vidtk::event_sptr, unsigned int> stepMap;
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
}

//-----------------------------------------------------------------------------
bool vpVidtkFileReader::ReadActivities(
  std::vector<vidtk::activity_sptr>& activities)
{
  if (this->ActivitiesFileName.isEmpty())
    {
    return false;
    }

  vidtk::activity_reader reader;
  if (!reader.open(stdString(this->ActivitiesFileName)))
    {
    return false;
    }

  return reader.read(activities, this->EventMap);
}

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
