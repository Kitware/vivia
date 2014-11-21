/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
