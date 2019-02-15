/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileTrackReader.h"

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

#include <qtIndexRange.h>
#include <qtStlUtil.h>

#include <QFileInfo>
#include <QUrl>

#include <iterator>
#include <limits>

//-----------------------------------------------------------------------------
vpFileTrackReader::vpFileTrackReader(vpTrackIO* io) : IO{io}
{
}

//-----------------------------------------------------------------------------
bool vpFileTrackReader::ReadTrackTraits(
  const QString& trackTraitsFileName) const
{
  std::ifstream file(stdString(trackTraitsFileName));
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
bool vpFileTrackReader::ReadTrackClassifiers(
  const QString& trackClassifiersFileName) const
{
  // Read P/V/O's or TOC's
  qtKstReader reader(QUrl::fromLocalFile(trackClassifiersFileName),
                     QRegExp("\\s+"), QRegExp("\n"));
  if (!reader.isValid())
    {
    return false;
    }

  // Process the P/V/O's / TOC's
  while (!reader.isEndOfFile())
    {
    int id;
    if (reader.readInt(id, 0))
      {
      vtkVgTrack* track = this->IO->TrackModel->GetTrack(id);
      if (track)
        {
        double pvo[3];
        if (reader.valueCount() == 4 &&
            reader.readReal(pvo[0], 1) &&
            reader.readReal(pvo[1], 2) &&
            reader.readReal(pvo[2], 3))
          {
          track->SetPVO(pvo);
          }
        else
          {
          std::map<int, double> toc;

          reader.seek(reader.currentRecord(), 1);
          while (!reader.isEndOfRecord())
            {
            QString type;
            double score;
            if (reader.readString(type) && reader.readReal(score))
              {
              const auto tti = this->IO->GetTrackTypeIndex(qPrintable(type));
              toc.emplace(tti, score);
              }
            else
              {
              std::cerr << "Track " << id << ": Corrupt TOC record at "
                        << reader.currentRecord() << ", "
                        << reader.currentValue() << '\n';
              break;
              }
            }
          if (!toc.empty())
            {
            track->SetTOC(toc);
            }
          }
        }
      else
        {
        std::cerr << "Unknown track id: " << id << '\n';
        }
      }
    reader.nextRecord();
    }

  return true;
}

//-----------------------------------------------------------------------------
void vpFileTrackReader::ReadTypesFile(const QString& tracksFileName) const
{
  // Look for files containing supplemental track info
  auto trackTypes = tracksFileName + ".types";

  // Load track types
  std::ifstream file(stdString(trackTypes));
  if (file)
    {
    int id;
    std::string type;
    double color[3];
    while (file >> id >> type)
      {
      vtkVgTrack* track =
        this->IO->TrackModel->GetTrack(this->IO->GetModelTrackId(id));
      if (!track)
        {
        std::cerr << qPrintable(trackTypes)
                  << ": track " << id << " does not exist!\n";
        continue;
        }

      const auto tti = this->IO->GetTrackTypeIndex(type.c_str());
      const vgTrackType& type = this->IO->TrackTypes->GetType(tti);
      type.GetColor(color[0], color[1], color[2]);
      track->SetType(tti);
      track->SetColor(color[0], color[1], color[2]);
      }
    }
}

//-----------------------------------------------------------------------------
bool vpFileTrackReader::ReadAttributesFile(
  const QString& tracksFileName, vgAttributeSet* trackAttributes) const
{
  auto trackAttributesFileName = tracksFileName + ".attributes";

  // Load track attributes
  std::ifstream file(stdString(trackAttributesFileName));
  if (file)
    {

    // Read and set the attributes (first clear any existing attributes)
    trackAttributes->Clear();
    this->IO->TrackModel->InitTrackTraversal();
    vtkSmartPointer<vtkVgScalars> attrScalars;
    while (vtkVgTrack* track = this->IO->TrackModel->GetNextTrack().GetTrack())
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
    for (const auto i QTE_UNUSED : qtIndexRange(numAttributes))
      {
      file >> groupName >> attributeName >> bitShift;
      trackAttributes->SetMask(groupName, attributeName, (one << bitShift));
      }

    int id;
    int frameNumber;
    while (file >> id >> frameNumber >> numAttributes)
      {
      vtkVgTrack* track =
        this->IO->TrackModel->GetTrack(this->IO->GetModelTrackId(id));
      if (!track)
        {
        std::cerr << qPrintable(trackAttributesFileName)
                  << ": track " << id << " does not exist!\n";
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        continue;
        }

      vtkTypeUInt64 attributeValue = 0;
      for (const auto i QTE_UNUSED : qtIndexRange(numAttributes))
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
bool vpFileTrackReader::ReadRegionsFile(
  const QString& tracksFileName, float offsetX, float offsetY,
  TrackRegionMap& trackRegionMap) const
{
  auto trackRegions = tracksFileName + ".regions";

  // Load polygonal bounding regions
  if (QFileInfo{trackRegions}.exists())
    {
    if (this->IO->TimeStampMode != vpTrackIO::TTM_FrameNumberOnly)
      {
      std::cerr << "Cannot load polygonal track regions "
                   "in current timestamp mode\n";
      return false;
      }

    std::ifstream file(stdString(trackRegions));
    if (!file)
      {
      std::cerr << "Error reading file " << qPrintable(trackRegions) << '\n';
      }

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
        trackRegionMap[id].emplace(frame, frameRegion);
        continue;
        }

      frameRegion.Points.reserve(numPoints * 3);
      for (const auto i QTE_UNUSED : qtIndexRange(numPoints))
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
