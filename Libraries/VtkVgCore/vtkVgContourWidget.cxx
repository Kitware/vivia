// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgContourWidget.h"

#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkContourRepresentation.h>
#include <vtkEvent.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>

vtkStandardNewMacro(vtkVgContourWidget);

//------------------------------------------------------------------------
vtkVgContourWidget::vtkVgContourWidget()
{
  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent,
                                          vtkEvent::NoModifier, 127, 1,
                                          "Delete",
                                          vtkWidgetEvent::Delete,
                                          this,
                                          vtkVgContourWidget::DeleteAction);

  // vtkContourWidget tries to register two actions with the
  // RightButtonPressEvent, AddFinalPointAction and ScaleContourAction.
  // However, the event translator / callback mapper doesn't seem to support
  // multiple mappings for a single VTK event, so the ScaleContourAction
  // mapping ends up being ignored. Remedy this by removing the existing
  // translations and adding back only the scale action (we don't care about
  // AddFinalPoint).
  this->CallbackMapper->GetEventTranslator()->RemoveTranslation(
    vtkCommand::RightButtonPressEvent);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkWidgetEvent::Scale,
                                          this,
                                          vtkContourWidget::ScaleContourAction);
}

//-------------------------------------------------------------------------
// This function is identical to vtkContourWidget::DeleteAction, except that
// this version correctly sets the abort flag on a handled event so that
// interactor observers further down the chain won't ever see it. We also invoke
// Start/EndInteraction event so that observers who are only listening for
// EndInteractionEvent will still get a notification.
//-------------------------------------------------------------------------
void vtkVgContourWidget::DeleteAction(vtkAbstractWidget* w)
{
  vtkVgContourWidget* self = reinterpret_cast<vtkVgContourWidget*>(w);

  if (self->WidgetState == vtkContourWidget::Start)
    {
    return;
    }

  vtkContourRepresentation* rep =
    reinterpret_cast<vtkContourRepresentation*>(self->WidgetRep);

  if (self->WidgetState == vtkContourWidget::Define)
    {
    if (rep->DeleteLastNode())
      {
      self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
      self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
      self->EventCallbackCommand->SetAbortFlag(1);
      }
    }
  else
    {
    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];
    rep->ActivateNode(X, Y);
    if (rep->DeleteActiveNode())
      {
      self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
      self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
      self->EventCallbackCommand->SetAbortFlag(1);
      }
    rep->ActivateNode(X, Y);
    int numNodes = rep->GetNumberOfNodes();
    if (numNodes < 3)
      {
      rep->ClosedLoopOff();
      if (numNodes < 2)
        {
        self->WidgetState = vtkContourWidget::Define;
        }
      }
    }

  if (rep->GetNeedToRender())
    {
    self->Render();
    rep->NeedToRenderOff();
    }
}
