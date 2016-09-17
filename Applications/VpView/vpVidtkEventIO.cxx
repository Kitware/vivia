/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkEventIO.h"

#include "vpVidtkReader.h"

#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTypeDefs.h>

#include <event_detectors/event_writer_kwe.h>
#include <event_detectors/human_event_includes.h>
#include <event_detectors/other_event_includes.h>
#include <event_detectors/scallop_event_includes.h>

#include <assert.h>

static vidtk::event_sptr CreateVidtkEvent(int type)
{
#define VEHICLE_EVENT_MACRO(name) \
  if (type == vidtk::name::type())  \
    {                               \
    return new vidtk::name;         \
    }
#define OTHER_EVENT_MACRO(name) VEHICLE_EVENT_MACRO(name)
#define HUMAN_EVENT_MACRO(name) VEHICLE_EVENT_MACRO(name)
#include <event_detectors/scallop_events.macro>
#include <event_detectors/other_events.macro>
#include <event_detectors/human_events.macro>
#undef VEHICLE_EVENT_MACRO
#undef HUMAN_EVENT_MACRO
#undef OTHER_EVENT_MACRO

  std::cerr << "Unknown event type: " << type << std::endl;
  return 0;
}

//-----------------------------------------------------------------------------
vpVidtkEventIO::vpVidtkEventIO(vpVidtkReader& reader,
                               vcl_map<vtkVgEvent*, vidtk::event_sptr>& eventMap,
                               vcl_map<unsigned int, vtkIdType>&
                                 sourceEventIdToModelIdMap,
                               const vcl_map<vtkVgTrack*, vidtk::track_sptr>&
                                 trackMap,
                               const vcl_map<unsigned int, vtkIdType>&
                                 sourceTrackIdToModelIdMap,
                               vtkVgEventModel* eventModel,
                               vtkVgEventTypeRegistry* eventTypes) :
  vpEventIO(eventModel, eventTypes), Reader(reader),
  EventMap(eventMap), SourceEventIdToModelIdMap(sourceEventIdToModelIdMap),
  TrackMap(trackMap), SourceTrackIdToModelIdMap(sourceTrackIdToModelIdMap)
{}

//-----------------------------------------------------------------------------
vpVidtkEventIO::~vpVidtkEventIO()
{}

//-----------------------------------------------------------------------------
bool vpVidtkEventIO::ReadEvents()
{
  return this->ReadEvents(0, 0.0f, 0.0f);
}

//-----------------------------------------------------------------------------
bool vpVidtkEventIO::ImportEvents(vtkIdType idsOffset,
                                  float offsetX, float offsetY)
{
  return this->ReadEvents(idsOffset, offsetX, offsetY);
}

//-----------------------------------------------------------------------------
bool vpVidtkEventIO::ReadEvents(vtkIdType idsOffset,
                                float offsetX, float offsetY)
{
  size_t prevEventsSize = this->Events.size();
  if (!this->Reader.ReadEvents(this->Events))
    {
    return false;
    }

  // Loop over all the newly added events.
  for (size_t i = prevEventsSize, size = this->Events.size(); i < size; ++i)
    {
    const vidtk::event_sptr& vEvent = this->Events[i];

    // Determine a unique id for the event.
    vtkIdType desiredId = idsOffset + vEvent->get_id();
    if (this->EventModel->GetEvent(desiredId))
      {
      // There is already an event in the model with the desired id.
      vtkIdType id = desiredId;
      desiredId = this->EventModel->GetNextAvailableId();
      std::cout << "Event id " << id
                << " is not unique: changing id of imported event to "
                << desiredId << '\n';
      }

    // Keep track of any ids that were changed.
    if (desiredId != static_cast<vtkIdType>(vEvent->get_id()))
      {
      this->SourceEventIdToModelIdMap[vEvent->get_id()] = desiredId;
      }

    int mode = -1;

    // Get the display mode for this type.
    if (this->EventTypes)
      {
      int index = this->EventTypes->GetTypeIndex(vEvent->my_type());
      if (index >= 0)
        {
        this->EventTypes->MarkTypeUsed(index);
        mode = this->EventTypes->GetType(index).GetDisplayMode();
        }
      }

    if (mode == vgEventType::DM_Regions ||
        mode == vgEventType::DM_TracksAndRegions)
      {
      // Region or combined display mode.
      vtkSmartPointer<vtkVgEventBase> eventBase =
        vtkSmartPointer<vtkVgEventBase>::New();
      eventBase->SetId(desiredId);
      if (this->SetupNodeEvent(vEvent, eventBase, offsetX, offsetY))
        {
        vtkVgEvent* vgEvent = this->EventModel->AddEvent(eventBase);
        this->EventMap[vgEvent] = vEvent;

        if (mode == vgEventType::DM_TracksAndRegions)
          {
          vgEvent->SetDisplayFlags(vtkVgEventBase::DF_TrackAndRegionEvent);
          }
        else
          {
          vgEvent->SetDisplayFlags(vtkVgEventBase::DF_RegionEvent);
          }
        }
      }
    else
      {
      // Track display mode.
      vtkSmartPointer<vtkVgEvent> vgEvent = vtkSmartPointer<vtkVgEvent>::New();
      vgEvent->SetId(desiredId);
      if (this->SetupEvent(vEvent, vgEvent))
        {
        vgEvent->SetDisplayFlags(vtkVgEventBase::DF_TrackEvent);
        this->EventModel->AddEvent(vgEvent);
        this->EventMap[vgEvent] = vEvent;
        }
      }
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
bool vpVidtkEventIO::WriteEvents(const char* filename) const
{
  vidtk::event_writer_kwe writer;
  if (!writer.open(filename))
    {
    return false;
    }

  // open supplementary log file
  std::string logFilename(filename);
  logFilename += ".log";
  std::ofstream logfile(logFilename.c_str());
  if (!logfile)
    {
    return false;
    }

  vcl_vector<vidtk::track_sptr> empty;
  vcl_vector<vidtk::timestamp> trackBeginTimes(2);
  vcl_vector<vidtk::timestamp> trackEndTimes(2);

  this->EventModel->InitEventTraversal();
  while (vtkVgEvent* event = this->EventModel->GetNextEvent().GetEvent())
    {
    vidtk::event_sptr e = this->EventMap[event];
    if (!e)
      {
      // If the event was not found in the map, it must have been added at
      // runtime. Create the vidtk event now.
      event->InitClassifierTraversal();
      e = CreateVidtkEvent(event->GetClassifierType());
      if (!e)
        {
        continue;
        }
      }
    e->set_id(event->GetId());

    // write status changes to log file
    switch (event->GetStatus())
      {
      case vgObjectStatus::None:
        if (event->IsDirty())
          {
          logfile << event->GetId();
          logfile << (event->IsUserCreated() ? "\tcreated\n" : "\tmodified\n");
          }
        break;

      case vgObjectStatus::Adjudicated:
        logfile << event->GetId() << '\t';
        if (event->IsDirty())
          {
          logfile << (event->IsUserCreated() ? "created, " : "modified, ");
          }
        logfile << "adjudicated\n";
        break;

      case vgObjectStatus::Excluded:
        logfile << event->GetId() << "\texcluded\n";
        continue; // do not write out excluded events
      }

    // unmodified events can be written out directly
    if (!event->IsDirty())
      {
      if (!writer.write(e))
        {
        return false;
        }
      // next event
      continue;
      }

    vgl_box_2d<unsigned> eventBounds;

    vidtk::timestamp trackBegin, trackEnd;

    // clear existing tracks
    e->set_parent_tracks(empty);

    trackBeginTimes.clear();
    trackEndTimes.clear();

    // iterate over the tracks for this event
    int numTracks = event->GetNumberOfTracks();
    for (int i = 0; i < numTracks; ++i)
      {
      vtkVgTrack* track;
      vtkVgTimeStamp vgStartFrame, vgEndFrame;

      // get the track info
      event->GetTrack(i, track, vgStartFrame, vgEndFrame);

      vidtk::timestamp startFrame(vgStartFrame.GetTime(),
                                  vgStartFrame.GetFrameNumber());
      vidtk::timestamp endFrame(vgEndFrame.GetTime(),
                                vgEndFrame.GetFrameNumber());

      vidtk::track_sptr t;
      vcl_map<vtkVgTrack*, vidtk::track_sptr>::const_iterator itr =
        this->TrackMap.find(track);
      if (itr != this->TrackMap.end())
        {
        t = itr->second;
        assert(t); // null pointers should never be inserted in the track map
        }
      else
        {
        // track was created after read
        t = new vidtk::track;
        t->set_id(track->GetId());
        }

      // add parent track pointer
      e->add_parent_track(t);

      // update the spatial bounds of the event
      bool foundStart = false;
      const vcl_vector<vidtk::track_state_sptr>& history = t->history();
      for (unsigned j = 0; j < history.size(); ++j)
        {
        // seek to the initial frame of the event track in the history
        if (!foundStart)
          {
          if (history[j]->time_ == startFrame)
            {
            foundStart = true;
            }
          else
            {
            continue;
            }
          }

        // enlarge the bounding box
        eventBounds.add(history[j]->amhi_bbox_);

        // done?
        if (history[j]->time_ == endFrame)
          {
          break;
          }
        }

      // write out the track start and end times for multitrack events
      if (numTracks > 1)
        {
        trackBeginTimes.push_back(startFrame);
        trackEndTimes.push_back(endFrame);
        }
      }

    vtkVgTimeStamp vgStartFrame = event->GetStartFrame();
    vtkVgTimeStamp vgEndFrame = event->GetEndFrame();
    vidtk::timestamp eb(vgStartFrame.GetTime(), vgStartFrame.GetFrameNumber());
    vidtk::timestamp ee(vgEndFrame.GetTime(), vgEndFrame.GetFrameNumber());

    e->set_event_begin(eb);
    e->set_event_end(ee);
    e->set_probability(event->GetActiveClassifierNormalcy());

    e->set_spatial_bounds(vgl_box_2d<double>(eventBounds.min_x(),
                                             eventBounds.max_x(),
                                             eventBounds.min_y(),
                                             eventBounds.max_y()));

    e->set_tracks_begin(trackBeginTimes);
    e->set_tracks_end(trackEndTimes);

    // write out the event
    if (!writer.write(e))
      {
      return false;
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkEventIO::SetupEvent(const vidtk::event_sptr vidtkEvent,
                                vtkVgEvent* vgEvent)
{
  // Currently using probability as normalcy but this will change in future.
  // TODO: Change this in near future.
  vgEvent->AddClassifier(vidtkEvent->my_type(), 0.0, vidtkEvent->get_probability());

  // NOTE:  There is a bug (in the "computer vision" code) specifying the event
  // frame span versus the frame span of the track:  the event MIGHT have a
  // start or end frame outside the track frame range, which we don't want!
  // Thus, we don't set the evnt start/end until we find out what the start/end
  // for our single track is (at this moment we only support 1 track, even
  // though it looks like we support more).
  vidtk::timestamp  eventBeginTimeStamp = vidtkEvent->get_event_begin();
  vidtk::timestamp  eventEndTimeStamp   = vidtkEvent->get_event_end();

  // for now we compute, since there can be a track/event start/end frame issue
  // as read from the file
  vtkVgTimeStamp eventBeginFrame(eventBeginTimeStamp.time(),
                                 eventBeginTimeStamp.frame_number());
  vtkVgTimeStamp eventEndFrame(eventEndTimeStamp.time(),
                               eventEndTimeStamp.frame_number());
  vtkVgTimeStamp eventCalculatedBeginFrame(true); // initialize to MaxTime
  vtkVgTimeStamp eventCalculatedEndFrame(false); // initialize to MinTime

  // Tracks associated with this event.
  unsigned int numOfTracks = vidtkEvent->get_num_parent_tracks();

  vcl_vector<vidtk::timestamp> trackBeginTimeStamp = vidtkEvent->get_tracks_begin();
  vcl_vector<vidtk::timestamp> trackEndTimeStamp   = vidtkEvent->get_tracks_end();

  // Check the assumption here.
  assert(trackBeginTimeStamp.size() == trackEndTimeStamp.size());

  vtkVgTrackModel* trackModel = this->EventModel->GetTrackModel();
  assert(trackModel);

  vcl_vector<unsigned int> parentTracksIds;
  vcl_vector<vidtk::track_sptr> parentTracks;
  parentTracksIds.reserve(numOfTracks);
  parentTracks.reserve(numOfTracks);

  for (unsigned int trackIndex = 0; trackIndex < numOfTracks; trackIndex++)
    {
    unsigned int srcTrackId = vidtkEvent->get_parent_track_id(trackIndex);

    // Check if the id of the track in the model is different than the source
    // id, and if so, use that one when looking up the track in the model.
    vcl_map<unsigned int, vtkIdType>::const_iterator itr =
      this->SourceTrackIdToModelIdMap.find(srcTrackId);

    vtkIdType trackId;
    if (itr != this->SourceTrackIdToModelIdMap.end())
      {
      trackId = itr->second;
      }
    else
      {
      trackId = static_cast<vtkIdType>(srcTrackId);
      }

    vtkVgTrack* track = trackModel->GetTrack(trackId);
    if (!track)
      {
      // should print out warning, but don't want during debugging!
      return false;
      }

    vidtk::track_sptr parentTrack = this->TrackMap.find(track)->second;
    parentTracks.push_back(parentTrack);
    parentTracksIds.push_back(parentTrack->id());

    vtkVgTimeStamp trackBeginFrame;
    vtkVgTimeStamp trackEndFrame;

    // We are assuming that timestamps are in sequence and all the empty ones
    // will be filled up with the event frame index and time.
    if (trackIndex < trackBeginTimeStamp.size() && numOfTracks > 1)
      {
      trackBeginFrame.SetTime(trackBeginTimeStamp[trackIndex].time());
      trackBeginFrame.SetFrameNumber(trackBeginTimeStamp[trackIndex].frame_number());
      trackEndFrame.SetTime(trackEndTimeStamp[trackIndex].time());
      trackEndFrame.SetFrameNumber(trackEndTimeStamp[trackIndex].frame_number());
      }
    else
      {
      trackBeginFrame = eventBeginFrame;
      trackEndFrame   = eventEndFrame;
      }

    if (trackBeginFrame < track->GetStartFrame())
      {
      trackBeginFrame = track->GetStartFrame();
      }
    else if (track->GetEndFrame() < trackBeginFrame)
      {
      trackBeginFrame = track->GetEndFrame();
      }
    if (track->GetEndFrame() < trackEndFrame)
      {
      trackEndFrame = track->GetEndFrame();
      }
    else if (trackEndFrame < trackBeginFrame)
      {
      trackEndFrame = trackBeginFrame;
      }

    if (trackBeginFrame < eventCalculatedBeginFrame)
      {
      eventCalculatedBeginFrame = trackBeginFrame;
      }
    if (eventCalculatedEndFrame < trackEndFrame)
      {
      eventCalculatedEndFrame = trackEndFrame;
      }

    vgEvent->AddTrack(track, trackBeginFrame, trackEndFrame);
    }

  // add the parent track pointers (with corrected ids) to the event
  vidtkEvent->set_parent_tracks_ids(parentTracksIds);
  for (unsigned int i = 0; i < numOfTracks; ++i)
    {
    vidtkEvent->set_track(parentTracks[i]);
    }

  // Currently dealing with only one track.
  vgEvent->SetStartFrame(eventCalculatedBeginFrame);
  vgEvent->SetEndFrame(eventCalculatedEndFrame);

  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkEventIO::SetupNodeEvent(const vidtk::event_sptr vidtkEvent,
                                    vtkVgEventBase* vgEvent,
                                    float offsetX, float offsetY)
{
  if (this->Reader.GetImageHeight() == 0)
    {
    return false;
    }

  double imageYExtent = this->Reader.GetImageHeight() - 1.0;

  const vgl_box_2d<double>& bbox = vidtkEvent->get_spatial_bounds();
  double region[8] = { bbox.min_x() + offsetX, imageYExtent - bbox.min_y() + offsetY,
                       bbox.min_x() + offsetX, imageYExtent - bbox.max_y() + offsetY,
                       bbox.max_x() + offsetX, imageYExtent - bbox.max_y() + offsetY,
                       bbox.max_x() + offsetX, imageYExtent - bbox.min_y() + offsetY
                     };

  vidtk::timestamp eventBeginTimeStamp = vidtkEvent->get_event_begin();
  vidtk::timestamp eventEndTimeStamp   = vidtkEvent->get_event_end();

  unsigned beginFrame = eventBeginTimeStamp.frame_number();
  unsigned endFrame = eventEndTimeStamp.frame_number();
  double beginTime = eventBeginTimeStamp.time();
  double endTime = eventEndTimeStamp.time();

  vtkVgTimeStamp eventBeginFrame(beginTime, beginFrame);
  vtkVgTimeStamp eventEndFrame(endTime, endFrame);

  // Add duplicate regions for each valid frame. This is wasteful of memory,
  // but right now the event representation expects per-frame regions.
  for (unsigned i = beginFrame; i <= endFrame; ++i)
    {
    vtkVgTimeStamp frame;
    frame.SetFrameNumber(i);
    vgEvent->AddRegion(frame, 4, region);
    }

  // Add tracks.
  unsigned numOfTracks = vidtkEvent->get_num_parent_tracks();

  vcl_vector<vidtk::timestamp> trackBeginTimeStamp = vidtkEvent->get_tracks_begin();
  vcl_vector<vidtk::timestamp> trackEndTimeStamp   = vidtkEvent->get_tracks_end();

  // Check the assumption here.
  assert(trackBeginTimeStamp.size() == trackEndTimeStamp.size());

  vtkVgTrackModel* trackModel = this->EventModel->GetTrackModel();
  assert(trackModel);

  unsigned int trackIndex;
  for (trackIndex = 0; trackIndex < numOfTracks; trackIndex++)
    {
    int trackId = static_cast<int>(vidtkEvent->get_parent_track_id(trackIndex));

    vtkVgTrack* track = trackModel->GetTrack(trackId);
    if (!track)
      {
      // should print out warning, but don't want during debugging!
      return false;
      }

    vtkVgTimeStamp trackBeginFrame;
    vtkVgTimeStamp trackEndFrame;

    // We are assuming that timestamps are in sequence and all the empty ones
    // will be filled up with the event frame index and time.
    if (trackIndex < trackBeginTimeStamp.size())
      {
      trackBeginFrame.SetTime(trackBeginTimeStamp[trackIndex].time());
      trackBeginFrame.SetFrameNumber(trackBeginTimeStamp[trackIndex].frame_number());
      trackEndFrame.SetTime(trackEndTimeStamp[trackIndex].time());
      trackEndFrame.SetFrameNumber(trackEndTimeStamp[trackIndex].frame_number());
      }
    else
      {
      trackBeginFrame = eventBeginFrame;
      trackEndFrame   = eventEndFrame;
      }

    if (trackBeginFrame < track->GetStartFrame())
      {
      trackBeginFrame = track->GetStartFrame();
      }
    else if (track->GetEndFrame() < trackBeginFrame)
      {
      trackBeginFrame = track->GetEndFrame();
      }
    if (track->GetEndFrame() < trackEndFrame)
      {
      trackEndFrame = track->GetEndFrame();
      }
    else if (trackEndFrame < trackBeginFrame)
      {
      trackEndFrame = trackBeginFrame;
      }

    vgEvent->AddTrack(trackId, trackBeginFrame, trackEndFrame);
    }

  vgEvent->AddClassifier(vidtkEvent->my_type(), 0.0,
                         vidtkEvent->get_probability());

  vgEvent->SetStartFrame(vtkVgTimeStamp(beginTime, beginFrame));
  vgEvent->SetEndFrame(vtkVgTimeStamp(endTime, endFrame));

  return true;
}
