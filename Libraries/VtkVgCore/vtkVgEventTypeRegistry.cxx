/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgEventTypeRegistry.h"

#include <vtkObjectFactory.h>

template class vtkVgTypeRegistry<vgEventType>;

vtkStandardNewMacro(vtkVgEventTypeRegistry);

//-----------------------------------------------------------------------------
vtkVgEventTypeRegistry::vtkVgEventTypeRegistry()
{
  this->InvalidType.SetName("Unknown");
}

//-----------------------------------------------------------------------------
const vgEventType& vtkVgEventTypeRegistry::GetTypeById(int id)
{
  int index = this->GetTypeIndex(id);
  if (index == -1)
  {
    this->WarnTypeNotFound(id);
    return this->InvalidType;
  }
  return this->GetType(index);
}

//-----------------------------------------------------------------------------
void vtkVgEventTypeRegistry::RemoveTypeById(int id)
{
  int index = this->GetTypeIndex(id);
  if (index == -1)
  {
    this->WarnTypeNotFound(id);
    return;
  }
  return this->RemoveType(index);
}

//-----------------------------------------------------------------------------
int vtkVgEventTypeRegistry::GetTypeIndex(int id) const
{
  const auto end = this->GetNumberOfTypes();
  for (decltype(+end) i = 0; i < end; ++i)
  {
    if (this->GetType(i).GetId() == id)
    {
      return i;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
int vtkVgEventTypeRegistry::GetNextAvailableId() const
{
  int result = 0;
  const auto end = this->GetNumberOfTypes();
  for (decltype(+end) i = 0; i < end; ++i)
  {
    result = std::max(result, this->GetType(i).GetId());
  }
  return ++result;
}

//-----------------------------------------------------------------------------
void vtkVgEventTypeRegistry::WarnTypeNotFound(int id)
{
  vtkErrorMacro("Event type id is not registered: " << id);
}
