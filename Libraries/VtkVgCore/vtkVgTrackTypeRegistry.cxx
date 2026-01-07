// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgTrackTypeRegistry.h"

#include <vtkObjectFactory.h>

template class vtkVgTypeRegistry<vgTrackType>;

vtkStandardNewMacro(vtkVgTrackTypeRegistry);

//-----------------------------------------------------------------------------
int vtkVgTrackTypeRegistry::GetTypeIndex(const char* id) const
{
  for (int i = 0, end = this->GetNumberOfTypes(); i < end; ++i)
    {
    if (strcmp(this->GetType(i).GetId(), id) == 0)
      {
      return i;
      }
    }
  return -1;
}
