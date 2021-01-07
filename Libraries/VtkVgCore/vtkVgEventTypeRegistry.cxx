// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
  for (int i = 0, end = this->GetNumberOfTypes(); i < end; ++i)
    {
    if (this->GetType(i).GetId() == id)
      {
      return i;
      }
    }
  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgEventTypeRegistry::WarnTypeNotFound(int id)
{
  vtkErrorMacro("Event type id is not registered: " << id);
}
