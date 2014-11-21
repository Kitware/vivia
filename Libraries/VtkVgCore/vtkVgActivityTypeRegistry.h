/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgActivityTypeRegistry_h
#define __vtkVgActivityTypeRegistry_h

#include "vtkVgTypeRegistry.h"

#include <vgExport.h>
#include <vgActivityType.h>

class VTKVG_CORE_EXPORT vtkVgActivityTypeRegistry
  : public vtkVgTypeRegistry<vgActivityType>
{
  typedef vtkVgTypeRegistry<vgActivityType> BaseClassType;
public:
  static vtkVgActivityTypeRegistry* New();

  vtkTypeMacro(vtkVgActivityTypeRegistry, BaseClassType);
};

#endif // __vtkVgActivityTypeRegistry_h
