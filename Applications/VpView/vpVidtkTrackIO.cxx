/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVidtkTrackIO.h"

#include "vpFrameMap.h"
#include "vpVidtkReader.h"
#include "vtkVpTrackModel.h"

#include <vtkVgScalars.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackTypeRegistry.h>

#include <vtkBoundingBox.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtksys/SystemTools.hxx>

#include <tracking_data/io/track_writer_process.h>
#include <tracking_data/pvo_probability.h>
#include <tracking_data/tracking_keys.h>

#include <assert.h>

//-----------------------------------------------------------------------------
vpVidtkTrackIO::vpVidtkTrackIO(vpVidtkReader& reader,
                               vcl_map<vtkVgTrack*, vidtk::track_sptr>& trackMap,
                               vcl_map<unsigned int, vtkIdType>&
                                 sourceIdToModelIdMap,
                               vtkVpTrackModel* trackModel,
                               TrackStorageMode storageMode,
                               TrackTimeStampMode timeStampMode,
                               vtkVgTrackTypeRegistry* trackTypes,
                               vtkMatrix4x4* geoTransform,
                               vpFrameMap* frameMap) :
  vpTrackIO(trackModel, storageMode, timeStampMode, trackTypes,
            geoTransform, frameMap),
  Reader(reader), TrackMap(trackMap), SourceIdToModelIdMap(sourceIdToModelIdMap)
{}

//-----------------------------------------------------------------------------
vpVidtkTrackIO::~vpVidtkTrackIO()
{}

//-----------------------------------------------------------------------------
bool vpVidtkTrackIO::ReadTracks()
{
  assert(this->StorageMode != TSM_TransformedGeoCoords ||
         this->GeoTransform);
  assert(this->StorageMode != TSM_InvertedImageCoords ||
         this->Reader.GetImageHeight() != 0);

  size_t prevTracksSize = this->Tracks.size();
  if (!this->Reader.ReadTracks(this->Tracks))
    {
    return false;
    }

  // Make a quick pass through to find out roughly how many points we should
  // allocate.
  if (this->TrackModel->GetPoints()->GetNumberOfPoints() == 0)
    {
    vtkIdType numberOfActualPoints = 0, totalNumberOfMissingFrames = 0;
    for (size_t i = 0, size = this->Tracks.size(); i < size; ++i)
      {
      const vcl_vector<vidtk::track_state_sptr>& history =
        this->Tracks[i]->history();

      numberOfActualPoints += static_cast<vtkIdType>(history.size());
      vtkIdType numberOfMissingFrames =
        static_cast<vtkIdType>(history.back()->time_.frame_number() -
                               history.front()->time_.frame_number() + 1) -
        static_cast<vtkIdType>(history.size());

      // Perchance we come up with a negative number, which we shouldn't!
      if (numberOfMissingFrames > 0)
        {
        totalNumberOfMissingFrames += numberOfMissingFrames;
        }
      }

    // Should be correct for bbox (4pts) + pt for all actual points, and only
    // single point for missing frames.
    this->TrackModel->GetPoints()->Allocate(
      5 * numberOfActualPoints + totalNumberOfMissingFrames);
    }

  // Process the newly added tracks.
  for (size_t i = prevTracksSize, size = this->Tracks.size(); i < size; ++i)
    {
    this->ReadTrack(this->Tracks[i]);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkTrackIO::ImportTracks(vtkIdType idsOffset,
                                  float offsetX, float offsetY)
{
  assert(this->StorageMode != TSM_TransformedGeoCoords ||
         this->GeoTransform);
  assert(this->StorageMode != TSM_InvertedImageCoords ||
         this->Reader.GetImageHeight() != 0);

  size_t prevTracksSize = this->Tracks.size();
  if (!this->Reader.ReadTracks(this->Tracks))
    {
    return false;
    }

  for (size_t i = prevTracksSize, size = this->Tracks.size(); i < size; ++i)
    {
    this->ReadTrack(this->Tracks[i], offsetX, offsetY, false, 0, 0,
                    idsOffset + this->Tracks[i]->id());
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vpVidtkTrackIO::WriteTracks(const char* filename,
                                 bool writeSceneElements) const
{
  if (writeSceneElements)
    {
    std::cerr << "ERROR: Can't write scene element tracks to kw18\n";
    return false;
    }

  vidtk::track_writer_process writer("vpVidtkTrackIO::WriteTracks");

  std::string filestr(filename);
  vcl_string format = filestr.substr(filestr.rfind('.') + 1);

  if (!writer.set_params(
        writer.params().set_value("filename", filename)
                       .set_value("overwrite_existing", true)
                       .set_value("format", format)
                       .set_value("disabled", false)))
    {
    return false;
    }

  if (!writer.initialize())
    {
    return false;
    }

  bool useWorldCoords = this->StorageMode == TSM_TransformedGeoCoords;
  bool useRawImageCoords = this->StorageMode == TSM_ImageCoords;

  vcl_vector<vidtk::track_sptr> tracks;
  vcl_vector<vidtk::image_object_sptr> objs(1);

  vtkPoints* points = this->TrackModel->GetPoints();
  double imageYExtent = this->Reader.GetImageHeight() - 1;

  std::ofstream typesFile;
  std::ofstream regionsFile;

  bool typesFileOpenFailed = false;
  bool regionsFileOpenFailed = false;

  vcl_string typesFilename(filename);
  typesFilename += ".types";

  vcl_string regionsFilename(filename);
  regionsFilename += ".regions";

  // Remove any existing companion files
  vtksys::SystemTools::RemoveFile(typesFilename.c_str());
  vtksys::SystemTools::RemoveFile(regionsFilename.c_str());

  // Convert track model tracks back to vidtk tracks.
  this->TrackModel->InitTrackTraversal();
  while (vtkVgTrack* modelTrack = this->TrackModel->GetNextTrack().GetTrack())
    {
    bool isSceneElement =
      (modelTrack->GetDisplayFlags() & vtkVgTrack::DF_SceneElement) != 0;

    if (isSceneElement)
      {
      continue;
      }

    vidtk::track_sptr track(new vidtk::track);
    track->set_id(modelTrack->GetId());

    vidtk::track_sptr origTrack;
    vcl_map<vtkVgTrack*, vidtk::track_sptr>::const_iterator itr =
      this->TrackMap.find(modelTrack);
    if (itr != this->TrackMap.end())
      {
      origTrack = itr->second;
      }

    // Write out track type to supplemental text file
    if (!typesFileOpenFailed && modelTrack->GetType() != -1)
      {
      if (!typesFile.is_open())
        {
        typesFile.open(typesFilename.c_str(), ios::out | ios::trunc);
        if (!typesFile)
          {
          std::cerr << "ERROR: Failed to write track types to "
                    << typesFilename << '\n';
          typesFileOpenFailed = true;
          }
        }

      if (!typesFileOpenFailed)
        {
        // Write out the type id
        typesFile << modelTrack->GetId() << ' '
                  << this->TrackTypes->GetType(modelTrack->GetType()).GetId()
                  << '\n';
        }
      }

    vcl_vector<vidtk::track_state_sptr>::const_iterator origTrkStateIter,
                                                        origTrkStateEnd;
    if (origTrack)
      {
      origTrkStateIter = origTrack->history().begin();
      origTrkStateEnd = origTrack->history().end();
      }

    modelTrack->InitPathTraversal();
    vtkVgTimeStamp ts;
    for (;;)
      {
      vtkIdType ptId = modelTrack->GetNextPathPt(ts);
      if (ptId == -1)
        {
        break;
        }

      vtkIdType nHeadPts;
      vtkIdType trackPtId;
      vtkIdType* headPts;
      modelTrack->GetHeadIdentifier(ts, nHeadPts, headPts, trackPtId);

      const bool frameIsInterpolated =
        !this->TrackModel->GetIsKeyframe(modelTrack->GetId(), ts);

      // Don't export frames without regions
      if (nHeadPts == 0)
        {
        continue;
        }

      // Catch up the state iterator on the vidtk track
      while (origTrack && origTrkStateIter != origTrkStateEnd &&
             (*origTrkStateIter)->time_.frame_number() < ts.GetFrameNumber())
        {
        ++origTrkStateIter;
        }
      vidtk::track_state_sptr orig_state;
      if (origTrack && origTrkStateIter != origTrkStateEnd)
        {
        orig_state = *origTrkStateIter;
        }

      bool isAABB = nHeadPts == 5;
      if (isAABB)
        {
        double ul[3];
        double ll[3];
        double lr[3];
        double ur[3];
        points->GetPoint(headPts[0], ul);
        points->GetPoint(headPts[1], ll);
        points->GetPoint(headPts[2], lr);
        points->GetPoint(headPts[3], ur);

        isAABB =
          ul[0] == ll[0] &&
          ll[1] == lr[1] &&
          lr[0] == ur[0] &&
          ur[1] == ul[1];
        }

      // Write out polygonal or interpolated regions to a supplemental file
      if (nHeadPts != 0 && (!isAABB || frameIsInterpolated))
        {
        if (!regionsFileOpenFailed)
          {
          if (!regionsFile.is_open())
            {
            regionsFile.open(regionsFilename.c_str(), ios::out | ios::trunc);
            if (!regionsFile)
              {
              std::cerr << "ERROR: Failed to write track regions to "
                        << regionsFilename << '\n';
              regionsFileOpenFailed = true;
              }
            }

          if (!regionsFileOpenFailed)
            {
            regionsFile << modelTrack->GetId() << ' '
                        << ts.GetFrameNumber() << ' '
                        << !frameIsInterpolated << ' '
                        << nHeadPts - 1;
            for (int i = 0; i < nHeadPts - 1; ++i)
              {
              double point[3];
              points->GetPoint(headPts[i], point);
              // seems like the yflip is in error if storage mode is
              // TSM_ImageCoords but let it go for now (trying to get
              // it to work for world coordinates)
              regionsFile << ' ' << point[0] << ' ' <<
                (useWorldCoords ? point[1] : imageYExtent - point[1]);
              }
            regionsFile << '\n';
            }
          }
        }

      double minX;
      double maxX;
      double minY;
      double maxY;

      if (!isAABB)
        {
        // compute the bounding rect of the points so we can export that instead
        vtkBoundingBox bbox;
        for (int i = 0; i < nHeadPts; ++i)
          {
          bbox.AddPoint(points->GetPoint(headPts[i]));
          }

        double z;
        bbox.GetMinPoint(minX, maxY, z);
        bbox.GetMaxPoint(maxX, minY, z);

        if (!useRawImageCoords && !useWorldCoords)
          {
          minY = imageYExtent - minY;
          maxY = imageYExtent - maxY;
          }
        }
      else
        {
        minX = points->GetPoint(headPts[0])[0];
        maxX = points->GetPoint(headPts[2])[0];

        if (useRawImageCoords || useWorldCoords)
          {
          minY = points->GetPoint(headPts[0])[1];
          maxY = points->GetPoint(headPts[1])[1];
          }
        else
          {
          minY = imageYExtent - points->GetPoint(headPts[0])[1];
          maxY = imageYExtent - points->GetPoint(headPts[1])[1];
          }
        }

      vidtk::track_state_sptr new_state(new vidtk::track_state);
      vidtk::image_object_sptr obj(new vidtk::image_object);

      new_state->time_.set_frame_number(ts.GetFrameNumber());

      if (ts.HasTime())
        {
        new_state->time_.set_time(ts.GetTime());
        }
      else
        {
        // HACK: This time is not necessarily valid, but this state must have a
        // time *different* from that of the last state, or the kw18 reader will
        // silently refuse to append the states to the track history when
        // reading the file back in.
        new_state->time_.set_time(ts.GetFrameNumber() * 0.5e6);
        }

      vtkVgGeoCoord geoCoord = modelTrack->GetGeoCoord(ts);

      double lat = 444.0;
      double lon = 444.0;
      if (orig_state && !orig_state->latitude_longitude(lat, lon))
        {
        vcl_vector<vidtk::image_object_sptr> objs;
        if (orig_state->data_.get(vidtk::tracking_keys::img_objs, objs))
          {
          lon = objs[0]->world_loc_[0];
          lat = objs[0]->world_loc_[1];
          }
        }

      // Use the original ground information if this frame of the track hasn't
      // been modified. Otherwise just write zeros.
      if (orig_state &&
          orig_state->time_.frame_number() == ts.GetFrameNumber() &&
          ((nHeadPts == 0 && geoCoord.Latitude == lat &&
            geoCoord.Longitude == lon) ||
           (orig_state->amhi_bbox_.min_x() == minX &&
            orig_state->amhi_bbox_.min_y() == minY &&
            orig_state->amhi_bbox_.max_x() == maxX &&
            orig_state->amhi_bbox_.max_y() == maxY)))
        {
        new_state->loc_ = orig_state->loc_;
        new_state->vel_ = orig_state->vel_;
        new_state->amhi_bbox_ = orig_state->amhi_bbox_;

        vcl_vector<vidtk::image_object_sptr> objs;
        if (orig_state->data_.get(vidtk::tracking_keys::img_objs, objs))
          {
          obj->img_loc_ = objs[0]->img_loc_;
          obj->world_loc_ = objs[0]->world_loc_;
          obj->area_ = objs[0]->area_;
          obj->bbox_ = objs[0]->bbox_;
          }
        else
          {
          obj->img_loc_[0] = 0;
          obj->img_loc_[1] = 0;
          obj->img_loc_[2] = 0;
          obj->world_loc_[0] = 0;
          obj->world_loc_[1] = 0;
          obj->world_loc_[2] = 0;
          }
        }
      else
        {
        new_state->loc_[0] = 0;
        new_state->loc_[1] = 0;
        new_state->loc_[2] = 0;

        new_state->vel_[0] = 0;
        new_state->vel_[1] = 0;
        new_state->vel_[2] = 0;

        new_state->amhi_bbox_.set_min_x(minX);
        new_state->amhi_bbox_.set_min_y(minY);
        new_state->amhi_bbox_.set_max_x(maxX);
        new_state->amhi_bbox_.set_max_y(maxY);

        double pt[3];
        points->GetPoint(ptId, pt);

        if (geoCoord.IsValid())
          {
          obj->world_loc_[0] = geoCoord.Longitude;
          obj->world_loc_[1] = geoCoord.Latitude;
          obj->world_loc_[2] = 0.0;
          }
        else
          {
          obj->world_loc_[0] = 0;
          obj->world_loc_[1] = 0;
          obj->world_loc_[2] = 0;
          }

        if (useWorldCoords)
          {
          obj->img_loc_[0] = 0.5 * (minX + maxX);
          obj->img_loc_[1] = 0.5 * (minY + maxY);
          }
        else
          {
          obj->img_loc_[0] = pt[0];
          obj->img_loc_[1] = useRawImageCoords ? pt[1] : imageYExtent - pt[1];
          }
        obj->bbox_ = new_state->amhi_bbox_;
        }

      objs[0] = obj;
      new_state->data_.set(vidtk::tracking_keys::img_objs, objs);

      track->add_state(new_state);
      }

    tracks.push_back(track);
    }

  writer.set_tracks(tracks);
  return writer.step();
}

//-----------------------------------------------------------------------------
void vpVidtkTrackIO::UpdateTracks(const vcl_vector<vidtk::track_sptr>& tracks,
                                  unsigned int updateStartFrame,
                                  unsigned int updateEndFrame)
{
  for (vcl_vector<vidtk::track_sptr>::const_iterator itr = tracks.begin(),
       end = tracks.end(); itr != end; ++itr)
    {
    this->ReadTrack(*itr, 0.0f, 0.0f, true, updateStartFrame, updateEndFrame);
    }
}

//-----------------------------------------------------------------------------
bool vpVidtkTrackIO::GetNextValidTrackFrame(vtkVgTrack* track,
                                            unsigned int startFrame,
                                            vtkVgTimeStamp& timeStamp) const
{
  vcl_map<vtkVgTrack*, vidtk::track_sptr>::const_iterator itr =
    this->TrackMap.find(track);
  if (itr == this->TrackMap.end())
    {
    return false;
    }

  vidtk::track_sptr vtrack = itr->second;
  vcl_vector<vidtk::track_state_sptr>::const_iterator iter;
  vcl_vector<vidtk::track_state_sptr>::const_iterator end =
    vtrack->history().end();

  // search forwards through the track history until we find the start frame
  // or go past
  for (iter = vtrack->history().begin(); iter != end; ++iter)
    {
    if ((*iter)->time_.frame_number() >= startFrame)
      {
      timeStamp.SetTime((*iter)->time_.time());
      timeStamp.SetFrameNumber((*iter)->time_.frame_number());
      return true;
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vpVidtkTrackIO::GetPrevValidTrackFrame(vtkVgTrack* track,
                                            unsigned int startFrame,
                                            vtkVgTimeStamp& timeStamp) const
{
  vcl_map<vtkVgTrack*, vidtk::track_sptr>::const_iterator itr =
    this->TrackMap.find(track);
  if (itr == this->TrackMap.end())
    {
    return false;
    }

  vidtk::track_sptr vtrack = itr->second;
  vcl_vector<vidtk::track_state_sptr>::const_reverse_iterator iter;
  vcl_vector<vidtk::track_state_sptr>::const_reverse_iterator end
    = vtrack->history().rend();

  // search backwards through the track history until we find the start frame
  // or go past
  for (iter = vtrack->history().rbegin(); iter != end; ++iter)
    {
    if ((*iter)->time_.frame_number() <= startFrame)
      {
      timeStamp.SetTime((*iter)->time_.time());
      timeStamp.SetFrameNumber((*iter)->time_.frame_number());
      return true;
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
vtkIdType vpVidtkTrackIO::GetModelTrackId(unsigned int sourceId) const
{
  vcl_map<unsigned int, vtkIdType>::const_iterator itr =
    this->SourceIdToModelIdMap.find(sourceId);

  if (itr == this->SourceIdToModelIdMap.end())
    {
    return vpTrackIO::GetModelTrackId(sourceId);
    }

  return itr->second;
}

//-----------------------------------------------------------------------------
unsigned int vpVidtkTrackIO::GetImageHeight() const
{
  return this->Reader.GetImageHeight();
}

//-----------------------------------------------------------------------------
void vpVidtkTrackIO::ReadTrack(
  const vidtk::track_sptr vidtkTrack,
  float offsetX, float offsetY,
  bool update, unsigned int updateStartFrame, unsigned int updateEndFrame,
  vtkIdType desiredId)
{
  if (desiredId == -1)
    {
    desiredId = static_cast<vtkIdType>(vidtkTrack->id());
    }

  bool newTrack = false;
  vtkVgTrack* track = 0;
  if (update)
    {
    track = this->TrackModel->GetTrack(desiredId);
    }
  else if (this->TrackModel->GetTrack(desiredId))
    {
    // There is already a track in the model with the desired id.
    vtkIdType id = desiredId;
    desiredId = this->TrackModel->GetNextAvailableId();
    std::cout << "Track id " << id
              << " is not unique: changing id of imported track to "
              << desiredId << '\n';
    }

  // Keep track of any ids that were changed.
  if (desiredId != static_cast<vtkIdType>(vidtkTrack->id()))
    {
    this->SourceIdToModelIdMap[vidtkTrack->id()] = desiredId;
    }
  vidtkTrack->set_id(static_cast<unsigned int>(desiredId));

  if (!track)
    {
    track = vtkVgTrack::New();
    track->InterpolateMissingPointsOnInsertOn();
    track->SetId(desiredId);
    track->SetPoints(this->TrackModel->GetPoints());
    newTrack = true;
    }

  vidtk::pvo_probability pvo;
  if (vidtkTrack->get_pvo(pvo))
    {
    track->SetPVO(pvo.get_probability_person(),
                  pvo.get_probability_vehicle(),
                  pvo.get_probability_other());
    }

  const vcl_vector<vidtk::track_state_sptr>& trackHistory =
    vidtkTrack->history();

  vtkSmartPointer<vtkVgScalars> attrScalars;
  if (newTrack)
    {
    attrScalars = vtkSmartPointer<vtkVgScalars>::New();
    attrScalars->SetNotFoundValue(0.0);

    track->SetScalars("StateAttributes", attrScalars);

    // set the size the track should need for its id lists
    track->Allocate(static_cast<vtkIdType>(
                      trackHistory.back()->time_.frame_number() -
                      trackHistory.front()->time_.frame_number() + 1));
    }
  else
    {
    attrScalars = track->GetScalars("StateAttributes");
    }

#ifdef VPVIEW_ASSIGN_FAKE_TRACK_ATTRIBUTES
  unsigned int count = 0;
  unsigned int attrs = 0;
  unsigned int attrType = 0;
#endif

  vcl_vector<vidtk::image_object_sptr> objs;
  vcl_vector<vidtk::track_state_sptr>::const_iterator trkStateIter;
  for (trkStateIter = trackHistory.begin(); trkStateIter != trackHistory.end();
       trkStateIter++)
    {
    // Only look at the frames in the update range.
    if (update)
      {
      unsigned int frame = (*trkStateIter)->time_.frame_number();
      if (frame < updateStartFrame)
        {
        continue;
        }
      if (frame > updateEndFrame)
        {
        break;
        }
      }

    vtkVgTimeStamp timeStamp;
    if (this->TimeStampMode & TSF_FrameNumber)
      {
      timeStamp.SetFrameNumber((*trkStateIter)->time_.frame_number());
      }
    if (this->TimeStampMode & TSF_Time)
      {
      timeStamp.SetTime((*trkStateIter)->time_.time());
      }

    if ((*trkStateIter)->data_.get(vidtk::tracking_keys::img_objs, objs))
      {
      double lat, lon;
      if (!(*trkStateIter)->latitude_longitude(lat, lon))
        {
        lon = objs[0]->world_loc_[0];
        lat = objs[0]->world_loc_[1];
        }
      vtkVgGeoCoord geoCoord(lat, lon);

      bool trackPointAvailable = true;
      double minY;
      double maxY;
      vgl_box_2d<unsigned>& vglBBox = objs[0]->bbox_;
      double pt[4] = {0.0, 0.0, 0.0, 1.0};
      if (this->StorageMode == TSM_TransformedGeoCoords)
        {
        // tracks points stored in geo coordinates, head point in image
        // coordinates (non-inverted)
        pt[0] = lon;
        pt[1] = lat;
        this->GeoTransform->MultiplyPoint(pt, pt);
        pt[0] /= pt[3];
        pt[1] /= pt[3];

        minY = static_cast<double>(vglBBox.min_y());
        maxY = static_cast<double>(vglBBox.max_y());
        }
      else
        {
        pt[0] = objs[0]->img_loc_[0];
        unsigned int imageMaxY = this->Reader.GetImageHeight() - 1;
        if (this->StorageMode == TSM_InvertedImageCoords)
          {
          pt[1] = imageMaxY - objs[0]->img_loc_[1];
          }
        else // TSM_ImageCoords || TSM_HomographyTransformedImageCoords
          {
          pt[1] = objs[0]->img_loc_[1];
          }

        if (this->StorageMode == TSM_HomographyTransformedImageCoords)
          {
          // we expect there to be a homography for every frame
          vpFrame frameMetaData;
          if (!this->FrameMap->getFrame((*trkStateIter)->time_.frame_number(),
                                        frameMetaData) ||
              !frameMetaData.Homography)
            {
            std::cerr << "ERROR: Homgraphy for frame # "
              << (*trkStateIter)->time_.frame_number()
              << "is unavailable.  Track point not added!\n";
            trackPointAvailable = false;
            }
          else
            {
            frameMetaData.Homography->MultiplyPoint(pt, pt);
            pt[0] /= pt[3];
            pt[1] /= pt[3];
            }
          }
        else
          {
          pt[0] += offsetX;
          pt[1] += offsetY;
          }

        if (this->StorageMode == TSM_InvertedImageCoords)
          {
          minY = imageMaxY - static_cast<double>(vglBBox.min_y());
          maxY = imageMaxY - static_cast<double>(vglBBox.max_y());
          }
        else // TSM_ImageCoords || TSM_HomographyTransformedImageCoords
          {
          minY = static_cast<double>(vglBBox.min_y());
          maxY = static_cast<double>(vglBBox.max_y());
          }
        }

      float bbox[] =
        {
        offsetX + static_cast<float>(vglBBox.min_x()), offsetY + static_cast<float>(minY), 0.0f,
        offsetX + static_cast<float>(vglBBox.min_x()), offsetY + static_cast<float>(maxY), 0.0f,
        offsetX + static_cast<float>(vglBBox.max_x()), offsetY + static_cast<float>(maxY), 0.0f,
        offsetX + static_cast<float>(vglBBox.max_x()), offsetY + static_cast<float>(minY), 0.0f
        };
      // Don't currently have ability to add shell (bbox) without setting a
      // track point, but may at some point.
      if (trackPointAvailable)
        {
        if (update)
          {
          track->SetPoint(timeStamp, pt, geoCoord, 4, bbox);
          }
        else
          {
          track->InsertNextPoint(timeStamp, pt, geoCoord, 4, bbox);
          }
        }

#ifdef VPVIEW_ASSIGN_FAKE_TRACK_ATTRIBUTES
      // assign fake attributes for testing purposes
      if (count++ % 4 == 0)
        {
        switch (attrType++ % 3)
          {
          case 0:
            attrs = vidtk::track_state_attributes::ATTR_ASSOC_FG_SSD |
                    vidtk::track_state_attributes::ATTR_ASSOC_DA_KINEMATIC |
                    vidtk::track_state_attributes::ATTR_KALMAN_ESH |
                    vidtk::track_state_attributes::ATTR_INTERVAL_FORWARD |
                    vidtk::track_state_attributes::ATTR_LINKING_START;
            break;
          case 1:
            attrs = vidtk::track_state_attributes::ATTR_ASSOC_FG_CSURF |
                    vidtk::track_state_attributes::ATTR_ASSOC_DA_MULTI_FEATURES |
                    vidtk::track_state_attributes::ATTR_KALMAN_LVEL |
                    vidtk::track_state_attributes::ATTR_INTERVAL_BACK |
                    vidtk::track_state_attributes::ATTR_LINKING_END;
            break;
          case 2:
            attrs = vidtk::track_state_attributes::ATTR_ASSOC_DA_KINEMATIC |
                    vidtk::track_state_attributes::ATTR_ASSOC_DA_MULTI_FEATURES |
                    vidtk::track_state_attributes::ATTR_INTERVAL_INIT |
                    vidtk::track_state_attributes::ATTR_LINKING_START |
                    vidtk::track_state_attributes::ATTR_LINKING_END;
            break;
          }
        }
      attrScalars->InsertValue(timeStamp,
                               static_cast<double>(attrs));
#else
      attrScalars->InsertValue(timeStamp,
                               static_cast<double>((*trkStateIter)->get_attrs()));
#endif
      }
    }

  if (!update)
    {
    track->Close();
    }

  if (newTrack)
    {
    if (this->HasOverrideColor)
      {
      track->SetColor(this->OverrideColor);
      }
    else
      {
      double color[3];
      int typeIndex = track->GetType();
      if (typeIndex != -1)
        {
        // If the track has a valid type, use that to look up a color
        const vgTrackType& type = this->TrackTypes->GetType(typeIndex);
        type.GetColor(color[0], color[1], color[2]);
        track->SetColor(color[0], color[1], color[2]);
        }
      else
        {
        this->GetDefaultTrackColor(track->GetId(), color);
        track->SetColor(color);
        }
      }
    this->TrackModel->AddTrack(track);
    track->FastDelete();
    }

  this->TrackMap[track] = vidtkTrack;
}
