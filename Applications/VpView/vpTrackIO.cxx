/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpTrackIO.h"

#include <vtkVgTrackModel.h>
#include <vtkVgTrackTypeRegistry.h>

#include <vtkMatrix4x4.h>

#include <assert.h>

enum { NumDefaultTrackColors = 12 };
static const unsigned char DefaultTrackColors[NumDefaultTrackColors][3] =
{
  { 255, 255,   0 }, // Yellow
  {   0, 255, 255 }, // Cyan
  { 178,  34,  34 }, // Firebrick
  { 106,  90, 205 }, // Slate Blue
  { 255,   0,   0 }, // Red
  {   0, 100,   0 }, // Dark Green
  { 188, 143, 143 }, // Rosy Brown
  {   0,   0, 255 }, // Blue
  { 127, 255,   0 }, // Chartreuse
  { 208,  32, 144 }, // Violet Red
  { 240, 230, 140 }, // Khaki
  { 255,  69,   0 }  // Orange Red
};

//-----------------------------------------------------------------------------
vpTrackIO::vpTrackIO(vtkVgTrackModel* trackModel,
                     TrackStorageMode storageMode,
                     TrackTimeStampMode timeStampMode,
                     vtkVgTrackTypeRegistry* trackTypes,
                     vtkMatrix4x4* geoTransform,
                     vpFrameMap* frameMap) :
  TrackModel(trackModel),
  TrackTypes(trackTypes),
  StorageMode(storageMode),
  TimeStampMode(timeStampMode),
  GeoTransform(geoTransform),
  FrameMap(frameMap),
  HasOverrideColor(false)
{
  assert(trackModel);
}

//-----------------------------------------------------------------------------
vpTrackIO::~vpTrackIO()
{}

//-----------------------------------------------------------------------------
void vpTrackIO::SetOverrideColor(const double color[3])
{
  if (!color)
    {
    this->HasOverrideColor = false;
    return;
    }

  this->HasOverrideColor = true;
  this->OverrideColor[0] = color[0];
  this->OverrideColor[1] = color[1];
  this->OverrideColor[2] = color[2];
}

//-----------------------------------------------------------------------------
bool vpTrackIO::ReadTrackTraits()
{
  return false;
}

//-----------------------------------------------------------------------------
bool vpTrackIO::ImportTracks(vtkIdType, float, float)
{
  return false;
}

//-----------------------------------------------------------------------------
bool vpTrackIO::GetNextValidTrackFrame(vtkVgTrack*, unsigned int,
                                       vtkVgTimeStamp&) const
{
  return false;
}

//-----------------------------------------------------------------------------
bool vpTrackIO::GetPrevValidTrackFrame(vtkVgTrack*, unsigned int,
                                       vtkVgTimeStamp&) const
{
  return false;
}

//-----------------------------------------------------------------------------
vtkIdType vpTrackIO::GetModelTrackId(unsigned int sourceId) const
{
  return static_cast<vtkIdType>(sourceId);
}

//-----------------------------------------------------------------------------
void vpTrackIO::GetDefaultTrackColor(int trackId, double (&color)[3])
{
  // Switch track colors every million ids.
  int colorIdx = std::min(trackId / 1000000, NumDefaultTrackColors - 1);
  color[0] = DefaultTrackColors[colorIdx][0] / 255.0;
  color[1] = DefaultTrackColors[colorIdx][1] / 255.0;
  color[2] = DefaultTrackColors[colorIdx][2] / 255.0;
}
