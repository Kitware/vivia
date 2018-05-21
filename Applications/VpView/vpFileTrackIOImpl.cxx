/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileTrackIOImpl.h"

#include "vpTrackIO.h"
#include "vtkVpTrackModel.h"

#include <vtkVgTrack.h>
#include <vtkVgTrackTypeRegistry.h>

#include <vtkPoints.h>
#include <vtksys/SystemTools.hxx>

#include <iterator>
#include <limits>

//-----------------------------------------------------------------------------
bool vpFileTrackIOImpl::ReadTrackTraits(vpTrackIO* io,
                                        const std::string& trackTraitsFileName)
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
    vtkVgTrack* track = io->TrackModel->GetTrack(id);

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
void vpFileTrackIOImpl::ReadTypesFile(vpTrackIO* io,
                                      const std::string& tracksFileName)
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
      vtkVgTrack* track = io->TrackModel->GetTrack(io->GetModelTrackId(id));
      if (!track)
        {
        std::cerr << trackTypes << ": track " << id << " does not exist!\n";
        continue;
        }

      int typeIndex = io->TrackTypes->GetTypeIndex(type.c_str());
      if (typeIndex == -1)
        {
        // Add a new type to the registry if it's not already defined
        vgTrackType tt;
        tt.SetId(type.c_str());
        typeIndex = io->TrackTypes->GetNumberOfTypes();
        io->TrackTypes->AddType(tt);
        }
      track->SetType(typeIndex);
      const vgTrackType& type = io->TrackTypes->GetType(typeIndex);
      type.GetColor(color[0], color[1], color[2]);
      track->SetColor(color[0], color[1], color[2]);
      }
    }
}

//-----------------------------------------------------------------------------
bool vpFileTrackIOImpl::ReadRegionsFile(vpTrackIO* io,
                                        const std::string& tracksFileName,
                                        int frameOffset,
                                        float offsetX/*=0*/,
                                        float offsetY/*=0*/)
{
  std::string trackRegions(tracksFileName);
  trackRegions += ".regions";

  // Load polygonal bounding regions
  if (vtksys::SystemTools::FileExists(trackRegions.c_str(), true))
    {
    if (io->TimeStampMode != vpTrackIO::TTM_FrameNumberOnly)
      {
      std::cerr << "Cannot load polygonal track regions "
                   "in current timestamp mode\n";
      return false;
      }

    std::ifstream file(trackRegions.c_str());
    int id;
    int frame;
    int numPoints;
    std::vector<float> points;
    bool isKeyFrame;
    while (file >> id >> frame >> isKeyFrame >> numPoints)
      {
      vtkVgTrack* track = io->TrackModel->GetTrack(io->GetModelTrackId(id));
      if (!track)
        {
        std::cerr << trackRegions << ": track " << id
                  << " does not exist!\n";
        continue;
        }

      if (numPoints < 3)
        {
        std::cerr << trackRegions << ": region for track " << id
                  << " has an invalid number of points (" << numPoints
                  << ")\n";
        continue;
        }

      vtkVgTimeStamp ts;
      ts.SetFrameNumber(frame);

      if (isKeyFrame)
        {
        io->TrackModel->AddKeyframe(id, ts);
        }

      points.reserve(numPoints * 3);
      std::back_insert_iterator<std::vector<float> > iter(points);

      for (int i = 0; i < numPoints; ++i)
        {
        float x, y;
        file >> x >> y;
        if (io->StorageMode == vpTrackIO::TSM_InvertedImageCoords)
          {
          y = io->GetImageHeight() - y - 1;
          }
        *iter++ = x + offsetX;
        *iter++ = y + offsetY;
        *iter++ = 0.0f;
        }

      // Use the original point supplied in the track file (which should be the
      // same whether using polygons or bounding boxes).
      double point[2];
      if (!track->GetPoint(ts, point, false))
        {
        std::cerr << trackRegions << ": region for track " << id
                  << " does not have a point in track file at frame "
                  << ts.GetFrameNumber() << '\n';
        continue;
        }


      track->SetPoint(ts, point, track->GetGeoCoord(ts), numPoints, &points[0]);
      points.clear();
      }
    }

  return true;
}
