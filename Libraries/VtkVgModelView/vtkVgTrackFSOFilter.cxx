/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTrackFSOFilter.h"

#include "vtkVgTrack.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVgTrackFSOFilter);

//-----------------------------------------------------------------------------
vtkVgTrackFSOFilter::vtkVgTrackFSOFilter()
{
}

//-----------------------------------------------------------------------------
vtkVgTrackFSOFilter::~vtkVgTrackFSOFilter()
{
}

//-----------------------------------------------------------------------------
int vtkVgTrackFSOFilter::GetBestClassifier(vtkVgTrack* track)
{
  int bestType = -1;
  double bestScore = -1.0;

  double pvo[3];
  track->GetFSO(pvo);

  // if unclassified, doesn't matter what he filter setting are
  if (pvo[0] == 0 && pvo[1] == 0 && pvo[2] == 0)
    {
    return vtkVgTrack::Unclassified;
    }

  int filterType;
  for (filterType = vtkVgTrack::Fish; filterType <= vtkVgTrack::Other; filterType++)
    {
    if (!this->GetShowType(filterType))
      {
      continue;
      }
    if (pvo[filterType] < this->GetMinProbability(filterType))
      {
      continue;
      }
    double maxProbability = this->GetMaxProbability(filterType);
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
void vtkVgTrackFSOFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
