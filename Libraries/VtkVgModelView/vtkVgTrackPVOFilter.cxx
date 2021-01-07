// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
