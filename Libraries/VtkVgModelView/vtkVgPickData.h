/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
