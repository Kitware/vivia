// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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