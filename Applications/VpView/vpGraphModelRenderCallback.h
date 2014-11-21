/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
