/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTrackPVOFilter.h"

#include "vtkVgTrack.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVgTrackPVOFilter);

//-----------------------------------------------------------------------------
vtkVgTrackPVOFilter::vtkVgTrackPVOFilter()
{
}

//-----------------------------------------------------------------------------
vtkVgTrackPVOFilter::~vtkVgTrackPVOFilter()
{
}

//-----------------------------------------------------------------------------
int vtkVgTrackPVOFilter::GetBestClassifier(vtkVgTrack* track)
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
  for (filterType = vtkVgTrack::Person; filterType <= vtkVgTrack::Other; filterType++)
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
void vtkVgTrackPVOFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
