// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVpInteractionCallback_h
#define __vtkVpInteractionCallback_h

// VTK includes.
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkSetGet.h>

// VG Application includes.
#include <vpViewCore.h>

// VG VTK includes.
#include "vtkVgInteractorStyleRubberBand2D.h"

class vtkVpInteractionCallback : public vtkCommand
{
public:

  static vtkVpInteractionCallback* New()
    {
    return new vtkVpInteractionCallback();
    }

  void UpdateViewCoreInstance()
    {
    this->ViewCoreInstance->updateExtents();
    this->ViewCoreInstance->render(false);
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
        this->ViewCoreInstance->getInteractorStyle()->GetInteraction();

      if (interaction == vtkInteractorStyleRubberBand2D::PANNING ||
          interaction == vtkInteractorStyleRubberBand2D::ZOOMING)
        {
        if (interaction == vtkInteractorStyleRubberBand2D::PANNING)
          {
          this->ViewCoreInstance->setIdOfTrackToFollow(-1);
          }
        this->UpdateViewCoreInstance();
        }
      }
    else if (eventId == vtkVgInteractorStyleRubberBand2D::ZoomCompleteEvent)
      {
      this->ViewCoreInstance->setIdOfTrackToFollow(-1);
      this->UpdateViewCoreInstance();
      }
    }

  vpViewCore* ViewCoreInstance;
};

#endif // __vtkVpInteractionCallback_h
