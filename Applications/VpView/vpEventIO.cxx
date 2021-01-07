// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpEventIO.h"

#include <vtkVgEventModel.h>
#include <vtkVgEventTypeRegistry.h>

#include <assert.h>

//-----------------------------------------------------------------------------
vpEventIO::vpEventIO(vtkVgEventModel* eventModel,
                     vtkVgEventTypeRegistry* eventTypes) :
  EventModel(eventModel),
  EventTypes(eventTypes)
{
  assert(eventModel);
}

//-----------------------------------------------------------------------------
bool vpEventIO::ReadEventLinks()
{
  return false;
}

//-----------------------------------------------------------------------------
bool vpEventIO::ImportEvents(vtkIdType, float, float)
{
  return false;
}
