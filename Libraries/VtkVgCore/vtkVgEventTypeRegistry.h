// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgEventTypeRegistry_h
#define __vtkVgEventTypeRegistry_h

#include "vtkVgTypeRegistry.h"

#include <vgExport.h>
#include <vgEventType.h>

class VTKVG_CORE_EXPORT vtkVgEventTypeRegistry
  : public vtkVgTypeRegistry<vgEventType>
{
  typedef vtkVgTypeRegistry<vgEventType> BaseClassType;
public:
  static vtkVgEventTypeRegistry* New();

  vtkTypeMacro(vtkVgEventTypeRegistry, BaseClassType);

  vtkVgEventTypeRegistry();

  // Description:
  // Access to registered types.
  const vgEventType& GetTypeById(int id);

  // Description:
  // Remove registered event type with given id.
  void RemoveTypeById(int id);

  // Description:
  // Get index of registered event type or -1 if not found.
  int GetTypeIndex(int id) const;

protected:
  virtual void WarnTypeNotFound(int id);

private:
  vgEventType InvalidType;
};

#endif // __vtkVgEventTypeRegistry_h
