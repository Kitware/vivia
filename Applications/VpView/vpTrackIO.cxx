/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpTrackIO.h"

#include "vtkVpTrackModel.h"

#include <vgColor.h>

#include <vtkVgTrackModel.h>
#include <vtkVgTrackTypeRegistry.h>

#include <vtkMatrix4x4.h>

#include <algorithm>

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
vpTrackIO::vpTrackIO(vtkVpTrackModel* trackModel,
                     TrackStorageMode storageMode,
                     TrackTimeStampMode timeStampMode,
                     vtkVgTrackTypeRegistry* trackTypes,
                     vtkMatrix4x4* geoTransform,
                     vpFileDataSource* imageDataSource,
                     vpFrameMap* frameMap) :
  TrackModel(trackModel),
  TrackTypes(trackTypes),
  StorageMode(storageMode),
  TimeStampMode(timeStampMode),
  GeoTransform(geoTransform),
  ImageDataSource(imageDataSource),
  FrameMap(frameMap)
{
  assert(trackModel);
}

//-----------------------------------------------------------------------------
vpTrackIO::~vpTrackIO()
{}

//-----------------------------------------------------------------------------
void vpTrackIO::SetOverrideColor(const vgColor& color)
{
  this->OverrideColor = color;
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

//-----------------------------------------------------------------------------
void vpTrackIO::AddTrack(vtkVgTrack* track)
{
  // Set track color
  double color[3];
  if (this->OverrideColor.isValid())
    {
    this->OverrideColor.fillArray(color);
    }
  else
    {
    int typeIndex = track->GetType();
    if (typeIndex != -1)
      {
      // If the track has a valid type, use that to look up a color
      const vgTrackType& type = this->TrackTypes->GetType(typeIndex);
      type.GetColor(color[0], color[1], color[2]);
      }
    else
      {
      this->GetDefaultTrackColor(track->GetId(), color);
      }
    }
  track->SetColor(color);

  // Add track to track model and release our ownership
  this->TrackModel->AddTrack(track);
  track->FastDelete();
}

//-----------------------------------------------------------------------------
int vpTrackIO::GetTrackTypeIndex(const char* typeName)
{
  const auto index = this->TrackTypes->GetTypeIndex(typeName);

  if (index >= 0)
    {
    return index;
    }

  vgTrackType type;
  type.SetId(typeName);
  type.SetColor(0.5, 0.5, 0.0);

  this->TrackTypes->AddType(type);
  return this->TrackTypes->GetTypeIndex(typeName);
}
