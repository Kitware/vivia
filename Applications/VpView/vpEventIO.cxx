/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
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

//-----------------------------------------------------------------------------
int vpEventIO::GetEventType(const char* typeName)
{
  const auto index = this->GetEventTypeIndex(typeName);
  return this->EventTypes->GetType(index).GetId();
}

//-----------------------------------------------------------------------------
int vpEventIO::GetEventTypeIndex(const char* typeName)
{
  const auto index = this->EventTypes->GetTypeIndex(typeName);

  if (index >= 0)
  {
    return index;
  }

  vgEventType type;
  type.SetName(typeName);
  type.SetId(this->EventTypes->GetNextAvailableId());

  const auto newIndex = this->EventTypes->GetNumberOfTypes();
  this->EventTypes->AddType(type);
  return newIndex;
}
