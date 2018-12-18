/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileTrackReader.h"

#include "vpTrackIO.h"
#include "vtkVpTrackModel.h"

#include <vtkVgTrack.h>
#include <vtkVgTrackTypeRegistry.h>

#include <vtkPoints.h>
#include <vtksys/SystemTools.hxx>

#include <iterator>
#include <limits>

//-----------------------------------------------------------------------------
vpFileTrackReader::vpFileTrackReader(vpTrackIO* io) : IO{io}
{
}

//-----------------------------------------------------------------------------
bool vpFileTrackReader::ReadTrackTraits(
  const std::string& trackTraitsFileName) const
{
  std::ifstream file(trackTraitsFileName.c_str());
  if (!file)
    {
    return false;
    }

  int id;
  double normalcy;

  while (file >> id >> normalcy)
    {
    vtkVgTrack* track = this->IO->TrackModel->GetTrack(id);

    if (!track)
      {
      std::cerr << "Unknown track id: " << id << '\n';
      continue;
      }

    track->SetNormalcy(normalcy);
    }

  return true;
}

//-----------------------------------------------------------------------------
void vpFileTrackReader::ReadTypesFile(const std::string& tracksFileName) const
{
  // Look for files containing supplemental track info
  std::string trackTypes(tracksFileName);
  trackTypes += ".types";

  // Load track types
  if (vtksys::SystemTools::FileExists(trackTypes.c_str(), true))
    {
    std::ifstream file(trackTypes.c_str());
    int id;
    std::string type;
    double color[3];
    while (file >> id >> type)
      {
      vtkVgTrack* track =
        this->IO->TrackModel->GetTrack(this->IO->GetModelTrackId(id));
      if (!track)
        {
        std::cerr << trackTypes << ": track " << id << " does not exist!\n";
        continue;
        }

      int typeIndex = this->IO->TrackTypes->GetTypeIndex(type.c_str());
      if (typeIndex == -1)
        {
        // Add a new type to the registry if it's not already defined
        vgTrackType tt;
        tt.SetId(type.c_str());
        typeIndex = this->IO->TrackTypes->GetNumberOfTypes();
        this->IO->TrackTypes->AddType(tt);
        }
      track->SetType(typeIndex);
      const vgTrackType& type = this->IO->TrackTypes->GetType(typeIndex);
      type.GetColor(color[0], color[1], color[2]);
      track->SetColor(color[0], color[1], color[2]);
      }
    }
}

//-----------------------------------------------------------------------------
bool vpFileTrackReader::ReadRegionsFile(
  const std::string& tracksFileName, float offsetX, float offsetY,
  TrackRegionMap& trackRegionMap) const
{
  std::string trackRegions(tracksFileName);
  trackRegions += ".regions";

  // Load polygonal bounding regions
  if (vtksys::SystemTools::FileExists(trackRegions.c_str(), true))
    {
    if (this->IO->TimeStampMode != vpTrackIO::TTM_FrameNumberOnly)
      {
      std::cerr << "Cannot load polygonal track regions "
                   "in current timestamp mode\n";
      return false;
      }

    std::ifstream file(trackRegions.c_str());
    unsigned int id;
    int frame;
    int numPoints;
    bool isKeyFrame;

    while (file >> id >> frame >> isKeyFrame >> numPoints)
      {
      FrameRegionInfo frameRegion;
      frameRegion.KeyFrame = isKeyFrame;
      frameRegion.NumberOfPoints = numPoints;
      if (!isKeyFrame)
        {
        // If this is an interpolated region, we don't need to read the points
        // because we do not insert the point; instead the interpolated points
        // are recalculated.
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        trackRegionMap[id].insert(std::make_pair(frame, frameRegion));
        continue;
        }

      frameRegion.Points.reserve(numPoints * 3);
      for (int i = 0; i < numPoints; ++i)
        {
        float x, y;
        file >> x >> y;
        if (this->IO->StorageMode == vpTrackIO::TSM_InvertedImageCoords)
          {
          y = this->IO->GetImageHeight() - y - 1;
          }
        frameRegion.Points.push_back(x + offsetX);
        frameRegion.Points.push_back(y + offsetY);
        frameRegion.Points.push_back(0.0f);
        }

      trackRegionMap[id].emplace(frame, frameRegion);
      }
    }

  return true;
}
