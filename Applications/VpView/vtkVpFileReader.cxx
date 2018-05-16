/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVpFileReader.h"
#include "vtkVpReaderInternalBase.h"

// VTK includes.
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

#include "vtkVgActivity.h"
#include "vtkVgActivityManager.h"
#include "vtkVgEvent.h"
#include "vtkVgEventModel.h"
#include "vtkVpReaderInternalBase.h"
#include "vtkVgTimeStamp.h"
#include "vtkVgTrack.h"
#include "vtkVgTrackModel.h"
#include "vtkVgTrackTypeRegistry.h"

#include <vgTrackType.h>

#include <activity_detectors/activity_reader.h>
#include <activity_detectors/activity.h>

#include <event_detectors/event_reader_kwe.h>
#include <event_detectors/event_types.h>
#include <event_detectors/event.h>

#include <tracking/track_reader_process.h>
#include <tracking/tracking_keys.h>

#include <utilities/config_block.h>

#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkVpFileReader);

//-----------------------------------------------------------------------------
vtkVpFileReader::vtkVpFileReader() : vtkVpReaderBase()
{
  this->TracksFileName = 0;
  this->EventsFileName = 0;
  this->ActivitiesFileName = 0;
}

//-----------------------------------------------------------------------------
vtkVpFileReader::~vtkVpFileReader()
{
  this->Reset();
}

//-----------------------------------------------------------------------------
bool vtkVpFileReader::GetNextValidTrackFrame(vtkVgTrack* t, vtkIdType startIndex,
                                             vtkVgTimeStamp& timeStamp)
{
  vidtk::track_sptr track = this->InternalBase->TrackMap[t];

  if (!track)
    {
    return false;
    }

  std::vector<vidtk::track_state_sptr>::const_iterator iter;
  std::vector<vidtk::track_state_sptr>::const_iterator end = track->history().end();

  // search forwards through the track history until we find the start frame
  // or go past
  for (iter = track->history().begin(); iter != end; ++iter)
    {
    if ((*iter)->time_.frame_number() >= (unsigned) startIndex)
      {
      timeStamp.SetTime((*iter)->time_.time());
      timeStamp.SetFrameNumber((*iter)->time_.frame_number());
      return true;
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vtkVpFileReader::GetPrevValidTrackFrame(vtkVgTrack* t, vtkIdType startIndex,
                                             vtkVgTimeStamp& timeStamp)
{
  vidtk::track_sptr track = this->InternalBase->TrackMap[t];

  if (!track)
    {
    return false;
    }

  std::vector<vidtk::track_state_sptr>::const_reverse_iterator iter;
  std::vector<vidtk::track_state_sptr>::const_reverse_iterator end
    = track->history().rend();

  // search backwards through the track history until we find the start frame
  // or go past
  for (iter = track->history().rbegin(); iter != end; ++iter)
    {
    if ((*iter)->time_.frame_number() <= (unsigned) startIndex)
      {
      timeStamp.SetTime((*iter)->time_.time());
      timeStamp.SetFrameNumber((*iter)->time_.frame_number());
      return true;
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
int vtkVpFileReader::ReadTracks(const char* filename,
                                enumTrackStorageModes trackStorageMode,
                                vtkMatrix4x4* latLonToWorld,
                                double* overrideColor)
{
  if (this->TracksFileName && !strcmp(this->TracksFileName, filename))
    {
    return this->InternalBase->Tracks.size() > 0 ? VTK_OK : VTK_ERROR;
    }

  this->SetTracksFileName(filename);

  this->InternalBase->PreReadTracks();

  if (!this->TracksFileName)
    {
    vtkErrorMacro("Must read tracks, but no filename specified!");
    return VTK_ERROR;
    }
  if (this->TracksFileName &&
      !vtksys::SystemTools::FileExists(this->TracksFileName, true))
    {
    vtkErrorMacro("Track file doesn't exist!");
    return VTK_ERROR;
    }

  std::string fname(filename);
  std::string fext = fname.substr(fname.rfind('.') + 1);

  // Just run this process without making a pipeline (setting one up is probably overkill)
  vidtk::track_reader_process trackReaderProcess("VpFileReader::vtkInternal::ReadTracks");
  trackReaderProcess.set_batch_mode(true); // All the tracks in one step
  vidtk::config_block processConfiguration = trackReaderProcess.params();
  processConfiguration.set("disabled", "false");
  processConfiguration.set("format", fext);
  processConfiguration.set("filename", fname);
  trackReaderProcess.set_params(processConfiguration);
  trackReaderProcess.initialize();
  trackReaderProcess.step();

  this->InternalBase->Tracks = trackReaderProcess.tracks();

  if (this->InternalBase->ReadTracks(this->TrackModel,
                                     this->ImageHeight,
                                     this->TimeStampMode,
                                     trackStorageMode,
                                     latLonToWorld,
                                     overrideColor) != VTK_OK)
    {
    return VTK_ERROR;
    }

  // Look for files containing supplemental track info
  std::string trackTypes(filename);
  trackTypes += ".types";

  // Load track types
  if (vtksys::SystemTools::FileExists(trackTypes.c_str(), true))
    {
    std::ifstream file(trackTypes.c_str());
    int id;
    std::string type;
    while (file >> id >> type)
      {
      vtkVgTrack* track = this->TrackModel->GetTrack(id);
      if (!track)
        {
        vtkErrorMacro(<< trackTypes << ": track " << id
                      << " does not exist!");
        continue;
        }

      int typeIndex = this->TrackTypes->GetTypeIndex(type.c_str());
      if (typeIndex == -1)
        {
        // Add a new type to the registry if it's not already defined
        vgTrackType tt;
        tt.SetId(type.c_str());
        typeIndex = this->TrackTypes->GetNumberOfTypes();
        this->TrackTypes->AddType(tt);
        }
      track->SetType(typeIndex);
      }
    }

  std::string trackRegions(filename);
  trackRegions += ".regions";

  // Load polygonal bounding regions
  if (vtksys::SystemTools::FileExists(trackRegions.c_str(), true))
    {
    if (this->TimeStampMode != FrameNumberOnly)
      {
      vtkErrorMacro("Cannot load polygonal track regions "
                    "in current timestamp mode");
      return VTK_OK;
      }

    std::ifstream file(trackRegions.c_str());
    int id;
    int frame;
    int numPoints;
    std::vector<float> points;
    bool isKeyFrame;
    while (file >> id >> frame >> isKeyFrame >> numPoints)
      {
      vtkVgTrack* track = this->TrackModel->GetTrack(id);
      if (!track)
        {
        vtkErrorMacro(<< trackRegions << ": track " << id
                      << " does not exist!");
        continue;
        }

      if (numPoints < 3)
        {
        vtkErrorMacro(<< trackRegions << ": region for track " << id
                      << " has an invalid number of points (" << numPoints
                      << ")");
        continue;
        }

      vtkVgTimeStamp ts;
      ts.SetFrameNumber(frame);

      // "Delete" frames from the track that we now know are interpolated. The
      // track code will re-generate the interpolated frames, which is
      // inefficient, but this is the only way (currently) to preserve the
      // information about which frames are true keyframes. We want to keep
      // track of this info since it will make the annotators' lives easier.
      if (!isKeyFrame)
        {
        track->DeletePoint(ts, true);
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        continue;
        }

      points.reserve(numPoints * 3);
      std::back_insert_iterator<std::vector<float> > iter(points);

      for (int i = 0; i < numPoints; ++i)
        {
        float x, y;
        file >> x >> y;
        y = this->ImageHeight - y - 1;
        *iter++ = x;
        *iter++ = y;
        *iter++ = 0.0f;
        }

      // Use the original point supplied in the track file (which should be the
      // same whether using polygons or bounding boxes).
      double point[2];
      if (!track->GetPoint(ts, point, false))
        {
        vtkErrorMacro(<< trackRegions << ": region for track " << id
                      << " does not have a point in track file at frame "
                      << ts.GetFrameNumber());
        continue;
        }

      track->SetPoint(ts, point, track->GetGeoCoord(ts), numPoints, &points[0]);
      points.clear();
      }
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkVpFileReader::ReadTrackTraits(const char* filename)
{
  if (!filename)
    {
    return VTK_ERROR;
    }

  if (filename &&
      !vtksys::SystemTools::FileExists(filename, true))
    {
    vtkErrorMacro("Track traits file doesn't exist!");
    return VTK_ERROR;
    }

  std::ifstream file(filename);

  int id;
  double normalcy;

  while (file >> id >> normalcy)
    {
    vtkVgTrack* track = this->TrackModel->GetTrack(id);

    if (!track)
      {
      vtkErrorMacro("Unknown track id: " << id);
      continue;
      }

    track->SetNormalcy(normalcy);
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkVpFileReader::ReadEvents(const char* filename)
{
  if (this->InternalBase->Tracks.size() == 0)
    {
    vtkErrorMacro("No track. Can't read events.");
    return VTK_ERROR;
    }

  if (this->EventsFileName && !strcmp(this->EventsFileName, filename))
    {
    return this->InternalBase->Tracks.size() > 0 ? VTK_OK : VTK_ERROR;
    }

  this->SetEventsFileName(filename);

  this->InternalBase->PreReadEvents();

  if (!this->EventsFileName)
    {
    vtkErrorMacro("Trying to read events, but no filename specified!");
    return VTK_ERROR;
    }
  if (this->EventsFileName &&
      !vtksys::SystemTools::FileExists(this->EventsFileName, true))
    {
    vtkErrorMacro("Event file doesn't exist!");
    return VTK_ERROR;
    }

  vidtk::event_reader_kwe reader;
  const std::string  file(filename);
  reader.open(file);

  this->InternalBase->EventStepMap.clear();
  reader.read(this->InternalBase->Events, this->InternalBase->EventStepMap);

  return this->InternalBase->ReadEvents(this->TrackModel,
                                        this->EventModel,
                                        this->ImageHeight,
                                        this->EventConfig);
}

//-----------------------------------------------------------------------------
int vtkVpFileReader::ReadEventLinks(const char* filename)
{
  if (!filename)
    {
    return VTK_ERROR;
    }

  if (filename &&
      !vtksys::SystemTools::FileExists(filename, true))
    {
    vtkErrorMacro("Event links file doesn't exist!");
    return VTK_ERROR;
    }

  std::ifstream file(filename);

  EventLink link;
  while (file >> link.Source >> link.Destination >> link.Probability)
    {
    // convert from log probability
    link.Probability = exp(link.Probability);
    this->EventModel->AddEventLink(link);
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkVpFileReader::ReadActivities(const char* filename)
{
  if (this->InternalBase->Events.size() == 0)
    {
    vtkErrorMacro("No events. Can't read activities.");
    return VTK_ERROR;
    }

  if (this->ActivitiesFileName && !strcmp(this->ActivitiesFileName, filename))
    {
    return this->InternalBase->Activities.size() > 0 ? VTK_OK : VTK_ERROR;
    }

  this->SetActivitiesFileName(filename);

  if (!this->ActivitiesFileName)
    {
    vtkErrorMacro("Trying to read activities, but no filename specified!");
    return VTK_ERROR;
    }
  if (this->ActivitiesFileName &&
      !vtksys::SystemTools::FileExists(this->ActivitiesFileName, true))
    {
    vtkErrorMacro("Activity file doesn't exist!");
    return VTK_ERROR;
    }

  this->InternalBase->PreReadActivities();

  // the event manager actually hasthis map, but the reader's need for it should
  // go away, I think, since "ids" are going away "soon"
  std::map<unsigned int, vidtk::event_sptr> eventMap;

  std::vector<vidtk::event_sptr>::const_iterator eventIter =
    this->InternalBase->Events.begin();
  for (; eventIter != this->InternalBase->Events.end(); eventIter++)
    {
    eventMap[(*eventIter)->get_id()] = *eventIter;
    }

  vidtk::activity_reader reader;
  reader.open(filename);
  reader.read(this->InternalBase->Activities, eventMap);

  return this->InternalBase->ReadActivities(this->EventModel,
                                            this->ActivityManager,
                                            this->ImageHeight,
                                            this->ActivityConfig);
}

//-----------------------------------------------------------------------------
int vtkVpFileReader::Reset()
{
  vtkVpReaderBase::Reset();

  this->SetTracksFileName(0);
  this->SetEventsFileName(0);
  this->SetActivitiesFileName(0);

  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkVpFileReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Tracks FileName: " <<
     (this->TracksFileName != 0 ? this->TracksFileName : "NULL") << endl;
  os << indent << "Events FileName: " <<
     (this->EventsFileName != 0 ? this->EventsFileName : "NULL") << endl;
  os << indent << "Activities FileName: " <<
     (this->ActivitiesFileName != 0 ? this->ActivitiesFileName : "NULL") << endl;
}
