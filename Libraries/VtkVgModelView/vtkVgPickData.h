// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgPickData_h
#define __vtkVgPickData_h

// C++ includes.
#include <vector>

namespace vtkVgPickData
{
enum enumPickType
{
  EmptyPick = 0,
  PickedIcon = 1,
  PickedTrack = 2,
  PickedImage = 3,
  PickedEvent = 4,
  PickedActivity = 5,
  PickedGraph = 6,
  PickedActor = 7,
  PickedGraphNode = 8,
  PickedGraphEdge = 9
};

struct PickEntity
{
  vtkIdType PickedType;
  vtkIdType PickedId;
};

struct PickData
{
  std::vector<PickEntity> PickedEntities;
};
}

#endif // __vtkVgPickData_h
