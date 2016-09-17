/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTrackFilter.h"

#include "vtkVgTrack.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVgTrackFilter);

//-----------------------------------------------------------------------------
vtkVgTrackFilter::vtkVgTrackFilter()
{
  this->FilterShow[0] = this->FilterShow[1] = this->FilterShow[2] = false;
  this->FilterMinProbability[0] = 1.0;
  this->FilterMinProbability[1] = 1.0;
  this->FilterMinProbability[2] = 1.0;
  this->FilterMaxProbability[0] = 0.0;
  this->FilterMaxProbability[1] = 0.0;
  this->FilterMaxProbability[2] = 0.0;
}

//-----------------------------------------------------------------------------
vtkVgTrackFilter::~vtkVgTrackFilter()
{
}

//-----------------------------------------------------------------------------
void vtkVgTrackFilter::SetShowType(int trackType, bool state)
{
  if (trackType < vtkVgTrack::Fish || trackType > vtkVgTrack::Other)
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return;
    }

  if (this->FilterShow[trackType] == state)
    {
    return;
    }

  this->FilterShow[trackType] = state;
  this->Modified();
}

//-----------------------------------------------------------------------------
bool vtkVgTrackFilter::GetShowType(int trackType)
{
  if (trackType < vtkVgTrack::Fish || trackType > vtkVgTrack::Other)
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return false;
    }
  return this->FilterShow[trackType];
}

//-----------------------------------------------------------------------------
void vtkVgTrackFilter::SetMinProbability(int trackType, double threshold)
{
  if (trackType < vtkVgTrack::Fish || trackType > vtkVgTrack::Other)
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return;
    }

  if (this->FilterMinProbability[trackType] == threshold)
    {
    return;
    }

  this->FilterMinProbability[trackType] = threshold;
  this->Modified();
}


//-----------------------------------------------------------------------------
double vtkVgTrackFilter::GetMinProbability(int trackType)
{
  if (trackType < vtkVgTrack::Fish || trackType > vtkVgTrack::Other)
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return 1.0;
    }
  return this->FilterMinProbability[trackType];
}

//-----------------------------------------------------------------------------
void vtkVgTrackFilter::SetMaxProbability(int trackType, double threshold)
{
  if (trackType < vtkVgTrack::Fish || trackType > vtkVgTrack::Other)
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return;
    }

  if (this->FilterMaxProbability[trackType] == threshold)
    {
    return;
    }

  this->FilterMaxProbability[trackType] = threshold;
  this->Modified();
}

//-----------------------------------------------------------------------------
double vtkVgTrackFilter::GetMaxProbability(int trackType)
{
  if (trackType < vtkVgTrack::Fish || trackType > vtkVgTrack::Other)
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return 1.0;
    }
  return this->FilterMaxProbability[trackType];
}

//-----------------------------------------------------------------------------
int vtkVgTrackFilter::GetBestClassifier(vtkVgTrack* track)
{
  int bestType = -1;
  double bestScore = -1.0;

  double pvo[3];
  track->GetPVO(pvo);

  // if unclassified, doesn't matter what he filter setting are
  if (pvo[0] == 0 && pvo[1] == 0 && pvo[2] == 0)
    {
    return vtkVgTrack::Unclassified;
    }

  int filterType;
  for (filterType = vtkVgTrack::Fish; filterType <= vtkVgTrack::Other; filterType++)
    {
    if (!this->FilterShow[filterType])
      {
      continue;
      }
    if (pvo[filterType] < this->FilterMinProbability[filterType])
      {
      continue;
      }
    double maxProbability = this->FilterMaxProbability[filterType];
    if (pvo[filterType] > maxProbability)
      {
      continue;
      }
    if (pvo[filterType] > bestScore)
      {
      bestScore = pvo[filterType];
      bestType = filterType;
      }
    }

  return bestType;
}

//-----------------------------------------------------------------------------
void vtkVgTrackFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
