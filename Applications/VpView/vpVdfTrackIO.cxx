/*ckwg +5
 * Copyright 2020 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVdfTrackIO.h"

#include "vpFrameMap.h"
#include "vpFileTrackReader.h"
#include "vpFileUtil.h"
#include "vpVdfIO.h"
#include "vtkVpTrackModel.h"

#include <vdfDataSource.h>
#include <vdfSourceService.h>
#include <vdfTrackReader.h>

#include <qtEnumerate.h>
#include <qtStlUtil.h>

#include <QDir>
#include <QFileInfo>
#include <QUrl>

namespace
{

//-----------------------------------------------------------------------------
template <typename T>
T* getValue(T* item)
{
  return item;
}

//-----------------------------------------------------------------------------
template <typename Key, typename Value>
auto getValue(const std::pair<Key, Value>* pair)
  -> decltype(getValue(std::declval<const Value*>()))
{
  return (pair ? getValue(std::addressof(pair->second)) : nullptr);
}

//-----------------------------------------------------------------------------
template <typename Container, typename Key>
auto getValue(Container const& c, Key const& key)
  -> decltype(getValue(std::addressof(*c.find(key))))
{
  auto const iter = c.find(key);
  return getValue(iter == c.end() ? nullptr : std::addressof(*iter));
}

//-----------------------------------------------------------------------------
template <typename Container, typename Key>
auto getValue(Container const* c, Key const& key)
  -> decltype(getValue(std::addressof(*(c->find(key)))))
{
  return (c ? getValue(*c, key) : nullptr);
}

//-----------------------------------------------------------------------------
void addPoint(std::vector<float>& points, float x, float y, float z = 0.0f)
{
  points.push_back(x);
  points.push_back(y);
  points.push_back(z);
}

} // namespace <anonymous>

QTE_IMPLEMENT_D_FUNC(vpVdfTrackIO)

//-----------------------------------------------------------------------------
class vpVdfTrackIOPrivate
{
public:
  vpFileTrackReader FileReader;
  vpVdfIO* Base;
  QUrl TracksUri;
  QString TrackTraitsFilePath;
  QString TrackClassifiersFilePath;
};

//-----------------------------------------------------------------------------
vpVdfTrackIO::vpVdfTrackIO(
  vpVdfIO* base, vtkVpTrackModel* trackModel,
  TrackStorageMode storageMode, bool interpolateToGround,
  TrackTimeStampMode timeStampMode, vtkVgTrackTypeRegistry* trackTypes,
  vgAttributeSet* trackAttributes, vtkMatrix4x4* geoTransform,
  vpFrameMap* frameMap)
  : vpTrackIO{trackModel, storageMode, interpolateToGround, timeStampMode,
              trackTypes, geoTransform, frameMap},
    d_ptr{new vpVdfTrackIOPrivate{{this}, base, {}}}
{
}

//-----------------------------------------------------------------------------
vpVdfTrackIO::~vpVdfTrackIO()
{
}

//-----------------------------------------------------------------------------
unsigned int vpVdfTrackIO::GetImageHeight() const
{
  QTE_D();
  return d->Base->GetImageHeight();
}

//-----------------------------------------------------------------------------
void vpVdfTrackIO::SetTracksUri(const QUrl& uri)
{
  QTE_D();
  d->TracksUri = uri;
}

//-----------------------------------------------------------------------------
void vpVdfTrackIO::SetTrackTraitsFilePath(const QString& filePath)
{
  QTE_D();
  d->TrackTraitsFilePath = filePath;
}

//-----------------------------------------------------------------------------
void vpVdfTrackIO::SetTrackClassifiersFilePath(const QString& filePath)
{
  QTE_D();
  d->TrackClassifiersFilePath = filePath;
}

//-----------------------------------------------------------------------------
bool vpVdfTrackIO::ReadTracks(int /*frameOffset*/)
{
  QTE_D();

  vdfTrackReader reader;
  QStringList supplementalFileBases;
  vpFileTrackReader::TrackRegionMap trackRegionMap;

  if (d->TracksUri.isLocalFile())
  {
    // TODO resolve relative globs
    // const auto& globPath = d->TracksUri.toLocalFile();
    // const auto& files = vpGlobFiles(QDir::current(), pattern);

    QFileInfo fi{d->TracksUri.toLocalFile()};
    const auto& dir = fi.absoluteDir();
    const auto& pattern = fi.fileName();

    const auto& files = vpGlobFiles(dir, pattern);
    if (files.isEmpty())
    {
      return false;
    }

    // Read through each track file
    for (const auto& filePath : files)
    {
      d->FileReader.ReadRegionsFile(filePath, 0.0f, 0.0f, trackRegionMap);

      // Construct the track source and track reader
      const auto& trackUri = QUrl::fromLocalFile(filePath);
      QScopedPointer<vdfDataSource> source{
        vdfSourceService::createArchiveSource(trackUri)};

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

    supplementalFileBases = files;
  }
  else
  {
    if (d->TracksUri.isLocalFile())
    {
      const auto& filePath = d->TracksUri.toLocalFile();
      d->FileReader.ReadRegionsFile(filePath, 0.0f, 0.0f, trackRegionMap);
      supplementalFileBases.append(filePath);
    }

    // Construct the track source and track reader
    QScopedPointer<vdfDataSource> source{
      vdfSourceService::createArchiveSource(d->TracksUri)};

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
    // No data was obtained; die
    return false;
  }

  const auto& inTracks = reader.tracks();
  foreach (const auto& ti, qtEnumerate(inTracks))
  {
    const vdfTrackReader::Track& in = ti.value();

    auto* const track = vtkVgTrack::New();
    track->InterpolateMissingPointsOnInsertOn();
    track->SetPoints(this->TrackModel->GetPoints());

    // Get (desired) track model identifier and matching region set
    const auto vtkId = static_cast<vtkIdType>(ti.key().SerialNumber);
    const auto* trackRegions =
      getValue(trackRegionMap, static_cast<unsigned int>(vtkId));

    // Set (actual) track model identifier
    if (this->TrackModel->GetTrack(vtkId))
    {
      const auto fallbackId = this->TrackModel->GetNextAvailableId();
      std::cout << "Track id " << vtkId
                << " is not unique: changing id of imported track to "
                << fallbackId << std::endl;
      track->SetId(fallbackId);
    }
    else
    {
      track->SetId(vtkId);
    }

    // Set track type classifiers
    if (!in.Classification.empty())
    {
      std::map<int, double> toc;
      foreach (const auto& c, in.Classification)
      {
        toc.emplace(this->GetTrackTypeIndex(c.first.c_str()), c.second);
      }
      track->SetTOC(toc);
    }

    // Iterate over track states
    bool skippedInterpolationPointSinceLastInsert = false;
    foreach (const auto& s, in.Trajectory)
    {
      const auto frameNumber = s.TimeStamp.FrameNumber;
      const auto* region = getValue(trackRegions, frameNumber);

      if (region && !region->KeyFrame)
      {
        // Since this is an interpolated frame, don't set it; the track object
        // will handle recomputing interpolated frames
        // TODO - add option to track class to insert interpolated frames
        skippedInterpolationPointSinceLastInsert = true;
        continue;
      }

      // Get frame metadata
      QScopedPointer<vpFrame> frame;
      if (this->FrameMap)
      {
        frame.reset(new vpFrame);
        if (!this->FrameMap->getFrame(static_cast<int>(frameNumber), *frame))
        {
          frame.reset();
        }
      }

      // Extract time stamp
      auto ts = vtkVgTimeStamp{s.TimeStamp};
      if (!ts.HasTime() && frame)
      {
        ts = frame->Time;
      }

      // Extract point location
      double p[4] = {s.ImagePoint.X, s.ImagePoint.Y, 0.0, 1.0};
      if (this->StorageMode == TSM_InvertedImageCoords)
      {
        p[1] = this->GetImageHeight() - p[1];
      }
      else if (this->StorageMode == TSM_HomographyTransformedImageCoords)
      {
        if (!frame || !frame->Homography)
        {
          std::cerr << "ERROR: Homgraphy for frame " << ts.GetFrameNumber()
                    << "is unavailable.  Track point not added!\n";
          continue;
        }
        else
        {
          frame->Homography->MultiplyPoint(p, p);
          p[0] /= p[3];
          p[1] /= p[3];
        }
      }

      // Extract shell points
      std::vector<float> shell;
      if (region)
      {
        shell = region->Points;
      }
      else
      {
        if (s.ImageObject.empty())
        {
          shell.reserve(12);
          const auto& tl = s.ImageBox.TopLeft;
          const auto& br = s.ImageBox.BottomRight;
          addPoint(shell, tl.X, tl.Y);
          addPoint(shell, tl.X, br.Y);
          addPoint(shell, br.X, br.Y);
          addPoint(shell, br.X, tl.Y);
        }
        else
        {
          shell.reserve(3 * s.ImageObject.size());
          foreach (const auto& sp, s.ImageObject)
          {
            const auto x = static_cast<float>(sp.X);
            const auto y = static_cast<float>(sp.Y);
            addPoint(shell, x, y);
          }
        }

        // Adjust shell points for storage mode
        if (this->StorageMode == TSM_InvertedImageCoords)
        {
          const auto imageHeight = static_cast<float>(this->GetImageHeight());
          for (size_t i = 1; i < shell.size(); i += 3)
          {
            shell[i] = imageHeight - shell[i];
          }
        }
      }

      track->InsertNextPoint(ts, p, s.WorldLocation,
                             shell.size() / 3, shell.data(),
                             skippedInterpolationPointSinceLastInsert);
      skippedInterpolationPointSinceLastInsert = false;
      this->TrackModel->AddKeyframe(track->GetId(), ts);
    }

    track->Close();
    this->AddTrack(track);
  }

  for (const auto& n : supplementalFileBases)
  {
    d->FileReader.ReadTypesFile(n);
    // TODO attributes?
  }

  return true;
}

//-----------------------------------------------------------------------------
bool vpVdfTrackIO::ReadTrackTraits()
{
  QTE_D();
  return d->FileReader.ReadTrackTraits(d->TrackTraitsFilePath);
}

//-----------------------------------------------------------------------------
bool vpVdfTrackIO::ReadTrackClassifiers()
{
  QTE_D();
  return d->FileReader.ReadTrackClassifiers(d->TrackClassifiersFilePath);
}
