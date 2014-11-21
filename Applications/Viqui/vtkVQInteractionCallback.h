/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVQInteractionCallback_h
#define __vtkVQInteractionCallback_h

// VTK includes.
#include <vtkCamera.h>
#include <vtkCommand.h>

// VQ includes.
#include "vqCore.h"

// VG includes.

// VG VTK includes.
#include "vtkVgInteractorStyleRubberBand2D.h"

class vtkVQInteractionCallback : public vtkCommand
{
public:

  static vtkVQInteractionCallback* New()
    {
    vtkVQInteractionCallback* cmcb = new vtkVQInteractionCallback();
    return cmcb;
    }

  void SetCore(vqCore* core)
    {
    this->CoreInstance = core;
    }

  void UpdateCore()
    {
    this->CoreInstance->updateSources();
    this->CoreInstance->updateLOD();
    }

  virtual void Execute(vtkObject* caller, unsigned long eventId,
                       void* vtkNotUsed(callData))
    {
    if (!vtkVgInteractorStyleRubberBand2D::SafeDownCast(caller))
      {
      return;
      }

    if (eventId == vtkCommand::InteractionEvent)
      {
      int interaction =
        this->CoreInstance->getContextInteractorStyle()->GetInteraction();

      if (interaction == vtkVgInteractorStyleRubberBand2D::ZOOMING ||
          interaction == vtkVgInteractorStyleRubberBand2D::PANNING)
        {
        this->UpdateCore();
        }
      }
    else if (eventId == vtkVgInteractorStyleRubberBand2D::ZoomCompleteEvent)
      {
      this->UpdateCore();
      }
    }

private:
  vqCore* CoreInstance;
};

#endif // __vtkVQInteractionCallback_h
