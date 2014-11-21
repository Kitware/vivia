/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpEventTypeRegistry_h
#define __vpEventTypeRegistry_h

#include "vtkVgEventTypeRegistry.h"

class vpEventTypeRegistry : public vtkVgEventTypeRegistry
{
public:
  static vpEventTypeRegistry* New();

  vtkTypeMacro(vpEventTypeRegistry, vtkVgEventTypeRegistry);

protected:
  // Provide a more descriptive warning using the VIDTK type name.
  virtual void WarnTypeNotFound(int id);
};

#endif // __vpEventTypeRegistry_h