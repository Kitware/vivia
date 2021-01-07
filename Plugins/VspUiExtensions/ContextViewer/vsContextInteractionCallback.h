// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsContextInteractionCallback_h
#define __vsContextInteractionCallback_h

// VisGUI includes.
#include "vsContextViewer.h"

#include <vtkVgInteractorStyleRubberBand2D.h>

// VTK includes.
#include <vtkCamera.h>
#include <vtkCommand.h>

class vsContextInteractionCallback : public vtkCommand
{
public:

  static vsContextInteractionCallback* New()
    {
    return new vsContextInteractionCallback();
    }

  void SetViewer(vsContextViewer* viewer)
    {
    this->ViewerInstance = viewer;
    }

  void UpdateViewer()
    {
    this->ViewerInstance->updateSources();
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
        this->ViewerInstance->interactorStyle()->GetInteraction();

      if (interaction == vtkVgInteractorStyleRubberBand2D::ZOOMING ||
          interaction == vtkVgInteractorStyleRubberBand2D::PANNING)
        {
        this->UpdateViewer();
        }
      }
    else if (eventId == vtkVgInteractorStyleRubberBand2D::ZoomCompleteEvent)
      {
      this->UpdateViewer();
      }
    }

private:
  vsContextViewer* ViewerInstance;
};

#endif // __vsContextInteractionCallback_h
