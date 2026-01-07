// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// VTK includes
#include <vtkCommand.h>

// VisGUI includes
#include "vpGraphModelView.h"

class vpGraphModelRenderCallback : public vtkCommand
{
public:

  static vpGraphModelRenderCallback* New()
    {
    return new vpGraphModelRenderCallback();
    }

  void SetView(vpGraphModelView* view)
    {
    this->View = view;
    }

  virtual void Execute(vtkObject* vtkNotUsed(caller),
                       unsigned long vtkNotUsed(eventId),
                       void* vtkNotUsed(callData))
    {
    this->View->update();
    }

  vpGraphModelView* View;
};
