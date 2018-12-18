/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileTrackIOImpl.h"

#include "vpFrameMap.h"
#include "vpTrackIO.h"
#include "vtkVpTrackModel.h"

#include <qtKstReader.h>

#include <vtkVgScalars.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackTypeRegistry.h>
#include <vtkVgUtil.h>

#include <vgAttributeSet.h>


#include <vtkPoints.h>
#include <vtksys/SystemTools.hxx>

#include <QUrl>

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
bool vpFileTrackIOImpl::ReadTrackClassifiers(
  vpTrackIO* io, const std::string& trackClassifiersFileName)
{
  // Read P/V/O's
  // TODO read TOC's instead
  qtKstReader reader(QUrl::fromLocalFile(trackClassifiersFileName.c_str()),
                     QRegExp("\\s+"), QRegExp("\n"));
  if (!reader.isValid())
    {
    return false;
    }

  // Process the P/V/O's
  while (!reader.isEndOfFile())
    {
    int id;
    double pvo[3];
    if (reader.readInt(id, 0) &&
        reader.readReal(pvo[0], 1) &&
        reader.readReal(pvo[1], 2) &&
        reader.readReal(pvo[2], 3))
      {
      vtkVgTrack* track = io->TrackModel->GetTrack(id);
      if (!track)
        {
        std::cerr << "Unknown track id: " << id << '\n';
        }
      else
        {
        track->SetPVO(pvo);
        }
      }
    reader.nextRecord();
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
bool vpFileTrackIOImpl::ReadAttributesFile(
  vpTrackIO* io, const std::string& tracksFileName,
  vgAttributeSet* trackAttributes)
{
  auto trackAttributesFileName = tracksFileName + ".attributes";

  // Load track attributes
  if (vtksys::SystemTools::FileExists(trackAttributesFileName.c_str(), true))
    {
    std::ifstream file(trackAttributesFileName.c_str());

    // Read and set the attributes (first clear any existing attributes)
    trackAttributes->Clear();
    io->TrackModel->InitTrackTraversal();
    vtkSmartPointer<vtkVgScalars> attrScalars;
    while (vtkVgTrack* track = io->TrackModel->GetNextTrack().GetTrack())
      {
      attrScalars = vtkSmartPointer<vtkVgScalars>::New();
      attrScalars->SetNotFoundValue(0.0);
      track->SetScalars("DetectionAttributes", attrScalars);
      }

    std::string label;
    int numAttributes;
    file >> label >> numAttributes;
    std::string groupName, attributeName;
    unsigned int bitShift;
    vtkTypeUInt64 one = 1;
    for (int i = 0; i < numAttributes; ++i)
      {
      file >> groupName >> attributeName >> bitShift;
      trackAttributes->SetMask(groupName, attributeName, (one << bitShift));
      }

    int id;
    int frameNumber;
    while (file >> id >> frameNumber >> numAttributes)
      {
      vtkVgTrack* track = io->TrackModel->GetTrack(io->GetModelTrackId(id));
      if (!track)
        {
        std::cerr << trackAttributesFileName << ": track " << id
                  << " does not exist!\n";
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        continue;
        }

      vtkTypeUInt64 attributeValue = 0;
      for (int i = 0; i < numAttributes; ++i)
        {
        file >> bitShift;
        attributeValue |= (one << bitShift);
        }

      vtkVgTimeStamp ts;
      ts.SetFrameNumber(frameNumber);

      track->GetScalars("DetectionAttributes")->InsertValue(
        ts, static_cast<double>(attributeValue));
      }
    }

   return true;
}

//-----------------------------------------------------------------------------
bool vpFileTrackIOImpl::ReadRegionsFile(
  vpTrackIO* io, const std::string& tracksFileName,
  float offsetX, float offsetY,
  TrackRegionMap& trackRegionMap)
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
        if (io->StorageMode == vpTrackIO::TSM_InvertedImageCoords)
          {
          y = io->GetImageHeight() - y - 1;
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
