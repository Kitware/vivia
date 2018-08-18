/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpVdfTrackIO.h"

#include "vpFileDataSource.h"
#include "vpFrameMap.h"
#include "vpVdfIO.h"
#include "vtkVpTrackModel.h"

#include <vdfDataSource.h>
#include <vdfSourceService.h>
#include <vdfTrackReader.h>

#include <vtkVgTrackFilter.h>
#include <vtkVgTrackTypeRegistry.h>

#include <qtEnumerate.h>
#include <qtStlUtil.h>

#include <vtksys/Glob.hxx>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include <QUrl>

namespace
{

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
  vpVdfIO* Base;
  QUrl TracksUri;
};

//-----------------------------------------------------------------------------
vpVdfTrackIO::vpVdfTrackIO(
  vpVdfIO* base, vtkVpTrackModel* trackModel,
  TrackStorageMode storageMode, TrackTimeStampMode timeStampMode,
  vtkVgTrackTypeRegistry* trackTypes, vtkMatrix4x4* geoTransform,
  vpFileDataSource* imageDataSource, vpFrameMap* frameMap)
  : vpTrackIO{trackModel, storageMode, timeStampMode,
              trackTypes, geoTransform, imageDataSource, frameMap},
    d_ptr{new vpVdfTrackIOPrivate}
{
  QTE_D();
  d->Base = base;
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
QString vpVdfTrackIO::GetImageFile(unsigned int frame) const
{
  if (this->ImageDataSource)
    {
    const auto fi = static_cast<int>(frame);
    return qtString(this->ImageDataSource->getDataFile(fi));
    }
  return {};
}

//-----------------------------------------------------------------------------
void vpVdfTrackIO::SetTracksUri(const QUrl& uri)
{
  QTE_D();
  d->TracksUri = uri;
}

//-----------------------------------------------------------------------------
bool vpVdfTrackIO::ReadTracks()
{
  QTE_D();

  vdfTrackReader reader;

  if (d->TracksUri.isLocalFile())
    {
    // Interpret the specifier as a glob
    vtksys::Glob glob;
    glob.FindFiles(stdString(d->TracksUri.toLocalFile()));
    glob.SetRecurse(true);
    const auto& files = glob.GetFiles();
    if (files.empty())
      {
      return false;
      }

    // Read through each track file
    for (size_t i = 0, k = files.size(); i < k; ++i)
      {
      // Construct the track source and track reader
      const auto& trackUri = QUrl::fromLocalFile(qtString(files[i]));
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
    }
  else
    {
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

    // Set track model identifier
    const auto vtkId = static_cast<vtkIdType>(ti.key().SerialNumber);
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
    foreach (const auto& s, in.Trajectory)
      {
      // Get frame metadata
      QScopedPointer<vpFrame> frame;
      if (this->FrameMap)
        {
        frame.reset(new vpFrame);
        if (!this->FrameMap->getFrame(s.TimeStamp.FrameNumber, *frame))
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
        foreach (const auto& p, s.ImageObject)
          {
          const auto x = static_cast<float>(p.X);
          const auto y = static_cast<float>(p.Y);
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

      track->InsertNextPoint(ts, p, s.WorldLocation,
                             shell.size() / 3, shell.data());
      }

    track->Close();
    this->AddTrack(track);
    }

  return true;
}

//-----------------------------------------------------------------------------
QStringList vpVdfTrackIO::GetSupportedFormats() const
{
  return QStringList("NOAA CSV tracks (*.csv)");
}

//-----------------------------------------------------------------------------
QString vpVdfTrackIO::GetDefaultFormat() const
{
  return "csv";
}

//-----------------------------------------------------------------------------
bool vpVdfTrackIO::WriteTracks(
  const QString& filename, vtkVgTrackFilter* filter,
  bool writeSceneElements) const
{
  if (writeSceneElements)
    {
    qCritical() << "ERROR: Can't write scene element tracks to csv";
    return false;
    }

  // Open output file
  QFile file{filename};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
    qCritical().nospace()
      << "Failed to open output track file " << filename
      << ": " << qPrintable(file.errorString());
    return false;
    }

  QTextStream s{&file};
  const auto imageHeight = this->GetImageHeight();

  // Write tracks
  this->TrackModel->InitTrackTraversal();
  while (auto* track = this->TrackModel->GetNextTrack().GetTrack())
    {
    // Skip scene elements
    if (track->GetDisplayFlags() & vtkVgTrack::DF_SceneElement)
      {
      continue;
      }

    // Skip filtered tracks
    if (filter && filter->GetBestClassifier(track) == -1)
      {
      continue;
      }

    const auto trackId = track->GetId();

    // Get track classification
    auto toc = track->GetTOC();
    const auto tt = track->GetType();
    if (tt >= 0)
      {
      // Track type overrides classifiers
      toc.clear();
      toc.emplace(tt, 1.0);
      }

    auto confidence = -1.0;
    for (const auto& c : toc)
      {
      confidence = std::max(confidence, c.second);
      }

    // Iterate over track states
    vtkVgTimeStamp ts;
    track->InitPathTraversal();
    while (track->GetNextPathPt(ts) >= 0)
      {
      const auto& imagePath = this->GetImageFile(ts.GetFrameNumber());
      const auto& imageFile = QFileInfo{imagePath}.fileName();
      const auto& bbox = track->GetHeadBoundingBox(ts);
      s << trackId << ','
        << imageFile << ','
        << ts.GetFrameNumber() << ',';
      if (this->StorageMode == TSM_InvertedImageCoords)
        {
        s <<               bbox.GetBound(0) << ','
          << imageHeight - bbox.GetBound(2) << ','
          <<               bbox.GetBound(1) << ','
          << imageHeight - bbox.GetBound(3) << ',';
        }
      else
        {
        s << bbox.GetBound(0) << ','
          << bbox.GetBound(2) << ','
          << bbox.GetBound(1) << ','
          << bbox.GetBound(3) << ',';
        }
      s << confidence << ','
        << "-1"/*scalar*/;
      for (const auto& c : toc)
        {
        const auto tt = this->TrackTypes->GetType(c.first);
        s << ',' << tt.GetId() << ',' << c.second;
        }
      s << '\n';
      }
    }

  return true;
}
