// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
