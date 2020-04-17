/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
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
};

#endif // __vtkVgTrackTypeRegistry_h
