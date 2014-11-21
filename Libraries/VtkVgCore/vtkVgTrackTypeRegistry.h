/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
