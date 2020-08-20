/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFseTrackIO.h"

#include "vpFrameMap.h"
#include "vtkVpTrackModel.h"

#include <vtkVgScalars.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackTypeRegistry.h>
#include <vtkVgTypeDefs.h>

#include <vtkPoints.h>
#include <vtksys/SystemTools.hxx>

#include <qtStlUtil.h>

#include <QDebug>
#include <QFileInfo>
#include <QStringList>

#include <json.h>

#include <limits>

#include <cassert>

//-----------------------------------------------------------------------------
vpFseTrackIO::vpFseTrackIO(vtkVpTrackModel* trackModel,
                           TrackStorageMode storageMode,
                           bool interpolateToGround,
                           TrackTimeStampMode timeStampMode,
                           vtkVgTrackTypeRegistry* trackTypes,
                           vtkMatrix4x4* geoTransform,
                           vpFileDataSource* imageDataSource,
                           vpFrameMap* frameMap) :
  vpTrackIO(trackModel, storageMode, interpolateToGround, timeStampMode,
            trackTypes, geoTransform, imageDataSource, frameMap),
  ImageHeight(0)
{
  // TSM_TransformedGeoCoords is equal to TSM_ImageCoords for FSEs; the
  // "track head" is stored in non-inverted image coordinates
  if (!(this->StorageMode == TSM_ImageCoords ||
        this->StorageMode == TSM_InvertedImageCoords ||
        this->StorageMode == TSM_TransformedGeoCoords))
    {
    std::cerr << "Current track storage mode is not supported for FSE io\n";
    }
}

//-----------------------------------------------------------------------------
vpFseTrackIO::~vpFseTrackIO()
{}

//-----------------------------------------------------------------------------
bool vpFseTrackIO::ReadTracks(int frameOffset)
{
  return this->ImportTracks(frameOffset, 0, 0.0f, 0.0f);
}

//-----------------------------------------------------------------------------
bool vpFseTrackIO::ImportTracks(int vtkNotUsed(frameOffset),
                                vtkIdType idsOffset,
                                float offsetX, float offsetY)
{
  assert(this->ImageHeight != 0);
  assert(!this->TracksFileName.isEmpty());

  if (!QFileInfo{this->TracksFileName}.exists())
    {
    qCritical() << "Track file" << this->TracksFileName << "does not exist!";
    return false;
    }

  // Must read fse tracks *after* regular tracks if ids might collide
  if (this->TrackModel->GetSceneElementIdsOffset() == 0)
    {
    this->TrackModel->SetSceneElementIdsOffset(
      this->TrackModel->GetNextAvailableId());
    }

  JSONNode root;
  try
    {
    std::ifstream file(stdString(this->TracksFileName));
    std::string str((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

    root = libjson::parse(str);
    }
  catch (...)
    {
    std::cerr << "Error parsing track JSON\n";
    return false;
    }

  JSONNode tracks;
  try
    {
    tracks = root.at("tracks");
    }
  catch (...)
    {
    std::cerr << "No \"tracks\" element found\n";
    return false;
    }

  std::vector<float> points;

  try
    {
    for (unsigned int i = 0, size = tracks.size(); i < size; ++i)
      {
      const JSONNode& node = tracks[i];
      int id = node.at("id").as_int() + idsOffset;

      vtkIdType trackId = this->TrackModel->GetTrackIdForSceneElement(id);
      if (this->TrackModel->GetTrack(trackId))
        {
        // There is already a track in the model with the desired id.
        vtkIdType newId = this->TrackModel->GetNextAvailableId();
        std::cout << "Track id " << trackId
                  << " is not unique: changing id of imported track to "
                  << newId << '\n';
        trackId = newId;
        }

      vtkVgTrack* track = vtkVgTrack::New();
      track->InterpolateMissingPointsOnInsertOn();
      track->SetPoints(this->TrackModel->GetPoints());
      track->SetDisplayFlags(vtkVgTrack::DF_SceneElement);
      track->SetId(trackId);
      // This will always be false for now, since
      // this->StorageMode == TSM_HomographyTransformedImageCoords isn't
      // supported for FSE
      track->SetInterpolateToGround(this->InterpolateToGround &&
        this->StorageMode == TSM_HomographyTransformedImageCoords);

      // Get optional label
      try
        {
        std::string type = node.at("label").as_string();
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
      catch (...) {}

      // Get optional probability value
      try
        {
        double probability = node.at("probability").as_float();
        track->SetNormalcy(probability);
        }
      catch (...) {}

      // Get status string
      try
        {
        std::string status = node.at("status").as_string();
        if (status == "positive")
          {
          track->SetStatus(vgObjectStatus::Positive);
          }
        else if (status == "negative")
          {
          track->SetStatus(vgObjectStatus::Negative);
          }
        }
      catch (...) {}

      // Get is_computed bit
      try
        {
        track->SetUserCreated(!node.at("is_computed").as_bool());
        }
      catch (...) {}

      const JSONNode& frames = node.at("frames");
      for (unsigned int i = 0, size = frames.size(); i < size; ++i)
        {
        const JSONNode& frame = frames[i];

        int frameNum = frame.at("frame_id").as_int();

        vtkVgTimeStamp ts;
        if (this->TimeStampMode & TSF_FrameNumber)
          {
          ts.SetFrameNumber(frameNum);
          }
        if (this->TimeStampMode & TSF_Time)
          {
          try
            {
            double seconds = frame.at("seconds").as_float();
            ts.SetTime(seconds * 1e6);
            }
          catch (...)
            {
            std::cerr << "Track " << id
                      << " has a frame with no time information in full "
                      << "timestamp mode (" << frameNum << ")\n";
            }
          }

        double img_x = frame.at("image_x").as_float();
        double img_y = frame.at("image_y").as_float();

        vtkVgGeoCoord geoCoord;
        try
          {
          double latitude = frame.at("latitude").as_float();
          double longitude = frame.at("longitude").as_float();
          geoCoord = vtkVgGeoCoord(latitude, longitude);
          }
        catch (...) {}

        unsigned int imageMaxY = this->GetImageHeight() - 1;

        // Read the polygon points
        JSONNode polygon = frame.at("polygon");
        points.reserve(polygon.size() * 3);

        for (unsigned int i = 0, size = polygon.size(); i < size; ++i)
          {
          const JSONNode& vert = polygon[i];
          float x = vert.at("image_x").as_float();
          float y = vert.at("image_y").as_float();
          if (this->StorageMode == TSM_InvertedImageCoords)
            {
            y = imageMaxY - y;
            }
          points.push_back(x + offsetX);
          points.push_back(y + offsetY);
          points.push_back(0.0f);
          }

        if (this->StorageMode == TSM_InvertedImageCoords)
          {
          img_y = imageMaxY - img_y;
          }

        double point[] = { img_x + offsetX, img_y + offsetY, 0.0 };
        track->SetPoint(ts, point, geoCoord, vtkIdType(polygon.size()),
                        &points[0]);
        points.clear();
        }

      this->AddTrack(track);
      }
    }
  catch (...)
    {
    std::cerr << "Bad track element JSON\n";
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
QStringList vpFseTrackIO::GetSupportedFormats() const
{
  return {"Scene Elements (JSON) (*.json)"};
}

//-----------------------------------------------------------------------------
QString vpFseTrackIO::GetDefaultFormat() const
{
  return "json";
}

//-----------------------------------------------------------------------------
bool vpFseTrackIO::WriteTracks(
  const QString& filename, int vtkNotUsed(frameOffset),
  QPointF aoiOffset, bool writeSceneElements) const
{
  vtkPoints* points = this->TrackModel->GetPoints();
  double imageYExtent = this->GetImageHeight() - 1;

  // Not using this root node for anything now, but include it for future use
  JSONNode root(JSON_NODE);
  JSONNode tracks(JSON_ARRAY);
  tracks.set_name("tracks");

  // Only adjust track output by the aoiOffset if the StorageMode is
  // TSM_InvertedImageCoords.
  if (this->StorageMode != TSM_InvertedImageCoords)
      {
      aoiOffset = {0, 0};
      }

  this->TrackModel->InitTrackTraversal();
  while (vtkVgTrack* track = this->TrackModel->GetNextTrack().GetTrack())
    {
    bool isSceneElement =
      (track->GetDisplayFlags() & vtkVgTrack::DF_SceneElement) != 0;

    if (isSceneElement != writeSceneElements)
      {
      continue;
      }

    JSONNode node(JSON_NODE);

    vtkIdType id = track->GetId();
    if (isSceneElement)
      {
      id = this->TrackModel->GetSceneElementIdForTrack(id);
      }

    node.push_back(JSONNode("id", id));

    int trackType = track->GetType();
    if (trackType != -1)
      {
      node.push_back(JSONNode("label",
                              this->TrackTypes->GetType(trackType).GetId()));
      }

    node.push_back(JSONNode("probability", track->GetNormalcy()));

    const char* status;
    switch (track->GetStatus())
      {
      case vgObjectStatus::Positive: status = "positive"; break;
      case vgObjectStatus::Negative: status = "negative"; break;
      default: status = "none"; break;
      }
    node.push_back(JSONNode("status", status));

    node.push_back(JSONNode("is_computed", !track->IsUserCreated()));

    JSONNode frames(JSON_ARRAY);
    frames.set_name("frames");

    track->InitPathTraversal();
    vtkVgTimeStamp ts;
    for (;;)
      {
      vtkIdType ptId = track->GetNextPathPt(ts);
      if (ptId == -1)
        {
        break;
        }

      vtkIdType nHeadPts;
      vtkIdType trackPtId;
      vtkIdType* headPts;
      track->GetHeadIdentifier(ts, nHeadPts, headPts, trackPtId);

      if (nHeadPts == 0 || track->GetFrameIsInterpolated(ts))
        {
        continue;
        }

      JSONNode frame;
      frame.push_back(JSONNode("frame_id", ts.GetFrameNumber()));
      if (ts.HasTime())
        {
        frame.push_back(JSONNode("seconds", ts.GetTime() * 1e-6));
        }

      // image coord
      double pt[3];
      points->GetPoint(ptId, pt);
      if (this->StorageMode == TSM_InvertedImageCoords)
        {
        pt[1] = imageYExtent - pt[1];
        }
      frame.push_back(JSONNode("image_x", pt[0] + aoiOffset.x()));
      frame.push_back(JSONNode("image_y", pt[1] + aoiOffset.y()));

      // geo coord
      vtkVgGeoCoord geoCoord = track->GetGeoCoord(ts);
      if (geoCoord.IsValid())
        {
        frame.push_back(JSONNode("latitude", geoCoord.Latitude));
        frame.push_back(JSONNode("longitude", geoCoord.Longitude));
        }

      // polygon
      JSONNode polygon(JSON_ARRAY);
      polygon.set_name("polygon");
      for (int i = 0; i < nHeadPts - 1; ++i)
        {
        double point[3];
        points->GetPoint(headPts[i], point);
        if (this->StorageMode == TSM_InvertedImageCoords)
          {
          point[1] = imageYExtent - point[1];
          }
        JSONNode vertex(JSON_NODE);
        vertex.push_back(JSONNode("image_x", point[0] + aoiOffset.x()));
        vertex.push_back(JSONNode("image_y", point[1] + aoiOffset.y()));
        polygon.push_back(vertex);
        }

      frame.push_back(polygon);
      frames.push_back(frame);
      }

    node.push_back(frames);
    tracks.push_back(node);
    }
  root.push_back(tracks);

  std::ofstream file(stdString(filename), ios::out | ios::trunc | ios::binary);
  if (!file.is_open())
    {
    std::cerr << "ERROR: Failed to write tracks to "
              << qPrintable(filename) << '\n';
    return false;
    }

  file << root.write_formatted();
  return true;
}
