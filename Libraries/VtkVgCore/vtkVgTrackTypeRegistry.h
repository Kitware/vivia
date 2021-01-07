// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTrackTypeRegistry_h
#define __vtkVgTrackTypeRegistry_h

#include "vtkVgTypeRegistry.h"

#include <vgExport.h>
#include <vgTrackType.h>

class VTKVG_CORE_EXPORT vtkVgTrackTypeRegistry
  : public vtkVgTypeRegistry<vgTrackType>
{
  typedef vtkVgTypeRegistry<vgTrackType> BaseClassType;
public:
  static vtkVgTrackTypeRegistry* New();

  vtkTypeMacro(vtkVgTrackTypeRegistry, BaseClassType);

  // Description:
  // Get index of registered track type or -1 if not found.
  int GetTypeIndex(const char* id) const;
};

#endif // __vtkVgTrackTypeRegistry_h
