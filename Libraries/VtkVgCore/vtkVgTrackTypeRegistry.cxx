/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
