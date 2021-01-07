// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpFseIO.h"

#include "vpFseTrackIO.h"

#include <assert.h>

//-----------------------------------------------------------------------------
vpFseIO::vpFseIO() :
  ImageHeight(0)
{}

//-----------------------------------------------------------------------------
vpFseIO::~vpFseIO()
{
}

//-----------------------------------------------------------------------------
void vpFseIO::SetTrackModel(vtkVpTrackModel* trackModel,
                            vpTrackIO::TrackStorageMode storageMode,
                            vpTrackIO::TrackTimeStampMode timeStampMode,
                            vtkVgTrackTypeRegistry* trackTypes,
                            vtkMatrix4x4* geoTransform,
                            vpFrameMap* frameMap)
{
  std::unique_ptr<vpFseTrackIO> io{
    new vpFseTrackIO(trackModel, storageMode, timeStampMode,
                     trackTypes, geoTransform, frameMap)};
  io->SetImageHeight(this->ImageHeight);
  io->SetTracksFileName(this->TracksFilename);
  this->TrackIO = std::move(io);
}

//-----------------------------------------------------------------------------
void vpFseIO::SetEventModel(vtkVgEventModel*,
                            vtkVgEventTypeRegistry*)
{
}

//-----------------------------------------------------------------------------
void vpFseIO::SetActivityModel(vtkVgActivityManager*,
                               vpActivityConfig*)
{
}

//-----------------------------------------------------------------------------
void vpFseIO::SetTracksFileName(const QString& tracksFileName)
{
  this->TracksFilename = tracksFileName;
  if (this->TrackIO)
    {
    auto* const io = static_cast<vpFseTrackIO*>(this->TrackIO.get());
    io->SetTracksFileName(tracksFileName);
    }
}

//-----------------------------------------------------------------------------
void vpFseIO::SetImageHeight(unsigned int imageHeight)
{
  this->ImageHeight = imageHeight;
  if (this->TrackIO)
    {
    auto* const io = static_cast<vpFseTrackIO*>(this->TrackIO.get());
    io->SetImageHeight(imageHeight);
    }
}

//-----------------------------------------------------------------------------
unsigned int vpFseIO::GetImageHeight() const
{
  return this->ImageHeight;
}
