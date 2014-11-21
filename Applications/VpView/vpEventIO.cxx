/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
