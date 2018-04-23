/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
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
}

//-----------------------------------------------------------------------------
vtkVgTrackFilter::~vtkVgTrackFilter()
{
}

//-----------------------------------------------------------------------------
void vtkVgTrackFilter::SetShowType(int trackType, bool state)
{
  const auto i = this->Filters.find(trackType);
  if (i == this->Filters.end())
    {
    this->Filters.emplace(trackType, FilterSettings{state, 1.0, 0.0});
    this->Modified();
    }
  else if (i->second.Show != state)
    {
    i->second.Show = state;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
bool vtkVgTrackFilter::GetShowType(int trackType)
{
  const auto i = this->Filters.find(trackType);
  if (i == this->Filters.end())
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return false;
    }
  return i->second.Show;
}

//-----------------------------------------------------------------------------
void vtkVgTrackFilter::SetMinProbability(int trackType, double threshold)
{
  const auto i = this->Filters.find(trackType);
  if (i == this->Filters.end())
    {
    this->Filters.emplace(trackType, FilterSettings{false, threshold, 0.0});
    this->Modified();
    }
  else if (i->second.MinProbability != threshold)
    {
    i->second.MinProbability = threshold;
    this->Modified();
    }
}


//-----------------------------------------------------------------------------
double vtkVgTrackFilter::GetMinProbability(int trackType)
{
  const auto i = this->Filters.find(trackType);
  if (i == this->Filters.end())
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return 1.0;
    }
  return i->second.MinProbability;
}

//-----------------------------------------------------------------------------
void vtkVgTrackFilter::SetMaxProbability(int trackType, double threshold)
{
  const auto i = this->Filters.find(trackType);
  if (i == this->Filters.end())
    {
    this->Filters.emplace(trackType, FilterSettings{false, 1.0, threshold});
    this->Modified();
    }
  else if (i->second.MaxProbability != threshold)
    {
    i->second.MaxProbability = threshold;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
double vtkVgTrackFilter::GetMaxProbability(int trackType)
{
  const auto i = this->Filters.find(trackType);
  if (i == this->Filters.end())
    {
    vtkErrorMacro("Invalid track type: " << trackType);
    return 0.0;
    }
  return i->second.MaxProbability;
}

//-----------------------------------------------------------------------------
int vtkVgTrackFilter::GetBestClassifier(vtkVgTrack* track)
{
  int bestType = -1;
  double bestScore = -1.0;

  const auto& toc = track->GetTOC();

  // If unclassified, doesn't matter what the filter setting are
  if (!toc.size())
    {
    return std::numeric_limits<int>::max();
    }

  // Iterate over all filters
  for (const auto& f : this->Filters)
    {
    // Is this type shown?
    if (!f.second.Show)
      {
      continue;
      }

    // Does the track have a classification for this type?
    const auto i = toc.find(f.first);
    if (i == toc.end())
      {
      continue;
      }

    // Is the track's classification score for this type within the filter
    // thresholds?
    if (i->second >= f.second.MinProbability &&
        i->second <= f.second.MaxProbability)
      {
      if (i->second > bestScore)
        {
        bestScore = i->second;
        bestType = f.first;
        }
      }
    }

  return bestType;
}

//-----------------------------------------------------------------------------
void vtkVgTrackFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
