/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpModelIO.h"

#include <assert.h>

//-----------------------------------------------------------------------------
vpModelIO::vpModelIO()
{
}

//-----------------------------------------------------------------------------
void vpModelIO::SetTrackOverrideColor(const vgColor& color)
{
  assert(this->TrackIO);
  this->TrackIO->SetOverrideColor(color);
}

//-----------------------------------------------------------------------------
bool vpModelIO::ReadFrameMetaData(vpFrameMap*, const std::string&)
{
  // To be implemented in derived classes
  return false;
}

//-----------------------------------------------------------------------------
int vpModelIO::GetHomographyCount()
{
  return 0;
}

//-----------------------------------------------------------------------------
const std::vector<std::string>& vpModelIO::GetImageFiles() const
{
  static std::vector<std::string> empty;
  return empty;
}

//-----------------------------------------------------------------------------
bool vpModelIO::ReadTracks(int frameOffset)
{
  assert(this->TrackIO);
  return this->TrackIO->ReadTracks(frameOffset);
}

//-----------------------------------------------------------------------------
bool vpModelIO::ReadTrackTraits()
{
  assert(this->TrackIO);
  return this->TrackIO->ReadTrackTraits();
}

//-----------------------------------------------------------------------------
bool vpModelIO::ReadTrackClassifiers()
{
  assert(this->TrackIO);
  return this->TrackIO->ReadTrackClassifiers();
}

//-----------------------------------------------------------------------------
bool vpModelIO::ImportTracks(int frameOffset, vtkIdType idsOffset,
                             float offsetX, float offsetY)
{
  assert(this->TrackIO);
  return this->TrackIO->ImportTracks(frameOffset, idsOffset, offsetX, offsetY);
}

//-----------------------------------------------------------------------------
bool vpModelIO::WriteTracks(const QString& filename, int frameOffset,
                            QPointF aoiOffset)
{
  assert(this->TrackIO);
  return this->TrackIO->WriteTracks(filename, frameOffset, aoiOffset, false);
}

//-----------------------------------------------------------------------------
bool vpModelIO::ReadEvents()
{
  assert(this->EventIO);
  return this->EventIO->ReadEvents();
}

//-----------------------------------------------------------------------------
bool vpModelIO::ReadEventLinks()
{
  assert(this->EventIO);
  return this->EventIO->ReadEventLinks();
}

//-----------------------------------------------------------------------------
bool vpModelIO::ImportEvents(vtkIdType idsOffset,
                             float offsetX, float offsetY)
{
  assert(this->EventIO);
  return this->EventIO->ImportEvents(idsOffset, offsetX, offsetY);
}

//-----------------------------------------------------------------------------
bool vpModelIO::WriteEvents(const char* filename)
{
  assert(this->EventIO);
  return this->EventIO->WriteEvents(filename);
}

//-----------------------------------------------------------------------------
bool vpModelIO::ReadActivities()
{
  assert(this->ActivityIO);
  return this->ActivityIO->ReadActivities();
}

//-----------------------------------------------------------------------------
bool vpModelIO::ReadFseTracks()
{
  assert(this->FseTrackIO);
  return this->FseTrackIO->ReadTracks(0);
}

//-----------------------------------------------------------------------------
bool vpModelIO::ImportFseTracks(vtkIdType idsOffset,
                                float offsetX, float offsetY)
{
  assert(this->FseTrackIO);
  return this->FseTrackIO->ImportTracks(0, idsOffset, offsetX, offsetY);
}

//-----------------------------------------------------------------------------
bool vpModelIO::WriteFseTracks(const QString& filename, QPointF aoiOffset,
                               bool writeSceneElements)
{
  assert(this->FseTrackIO);
  return this->FseTrackIO->WriteTracks(filename, 0, aoiOffset,
                                       writeSceneElements);
}
