// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgSelectionWidget.h"

#include "vtkVgSelectionRepresentation.h"
#include "vtkVgSelectionListRepresentation.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"

vtkStandardNewMacro(vtkVgSelectionWidget);

//-------------------------------------------------------------------------
vtkVgSelectionWidget::vtkVgSelectionWidget()
{
  this->WidgetState = Start;
  this->TimerDuration = 250;

  // Okay, define the events for this widget. Note that we look for extra events
  // (like button press) because without it the hover widget thinks nothing has changed
  // and doesn't begin retiming.
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkWidgetEvent::Select,
                                          this, vtkVgSelectionWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkVgSelectionWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkVgSelectionWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseWheelForwardEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkVgSelectionWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseWheelBackwardEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkVgSelectionWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkVgSelectionWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::TimerEvent,
                                          vtkWidgetEvent::TimedOut,
                                          this, vtkVgSelectionWidget::HoverAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent,
                                          vtkEvent::AnyModifier, 13, 1, "Return",
                                          vtkWidgetEvent::Select,
                                          this, vtkVgSelectionWidget::SelectAction);
}

//-------------------------------------------------------------------------
vtkVgSelectionWidget::~vtkVgSelectionWidget()
{
}

//----------------------------------------------------------------------
void vtkVgSelectionWidget::SetEnabled(int enabling)
{
  if (enabling)   //----------------
    {
    vtkDebugMacro(<< "Enabling widget");

    if (this->Enabled)   //already enabled, just return
      {
      return;
      }

    if (! this->Interactor)
      {
      vtkErrorMacro(<< "The interactor must be set prior to enabling the widget");
      return;
      }

    this->SetCurrentRenderer(this->Interactor->GetRenderWindow()->
                             GetRenderers()->GetFirstRenderer());
    if (!this->CurrentRenderer)
      {
      return;
      }

    // We're ready to enable
    this->Enabled = 1;

    // listen for the events found in the EventTranslator
    this->EventTranslator->AddEventsToInteractor(this->Interactor,
                                                 this->EventCallbackCommand, this->Priority);

    // Start off the timer
    this->TimerId = this->Interactor->CreateRepeatingTimer(this->TimerDuration);
    this->WidgetState = vtkVgSelectionWidget::Timing;

    // Create representation
    this->CreateDefaultRepresentation();
    this->WidgetRep->SetRenderer(this->CurrentRenderer);
    this->WidgetRep->BuildRepresentation();
    this->CurrentRenderer->AddViewProp(this->WidgetRep);

    this->InvokeEvent(vtkCommand::EnableEvent, NULL);
    }

  else //disabling------------------
    {
    vtkDebugMacro(<< "Disabling widget");

    if (! this->Enabled)   //already disabled, just return
      {
      return;
      }

    this->CurrentRenderer->RemoveViewProp(this->WidgetRep);
    this->SetCurrentRenderer(NULL);

    this->Enabled = 0;
    this->Interactor->RemoveObserver(this->EventCallbackCommand);
    this->InvokeEvent(vtkCommand::DisableEvent, NULL);
    }

}

//-------------------------------------------------------------------------
void vtkVgSelectionWidget::SetCursor(int cState)
{
  switch (cState)
    {
  case 0: default:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionWidget::MoveAction(vtkAbstractWidget* w)
{
  vtkVgSelectionWidget* self = reinterpret_cast<vtkVgSelectionWidget*>(w);

  if (self->WidgetState == vtkVgSelectionWidget::Timing)
    {
    self->Interactor->DestroyTimer(self->TimerId);
    }
  else //we have already timed out, on this move we begin retiming
    {
    self->WidgetState = vtkVgSelectionWidget::Timing;

    vtkVgSelectionRepresentation* rep = self->GetSelectionRepresentation();
    rep->AnnotationVisibilityOff();
    self->Render();
    }
  self->TimerId = self->Interactor->CreateRepeatingTimer(self->TimerDuration);
}

//-------------------------------------------------------------------------
void vtkVgSelectionWidget::HoverAction(vtkAbstractWidget* w)
{
  vtkVgSelectionWidget* self = reinterpret_cast<vtkVgSelectionWidget*>(w);
  int timerId = *(reinterpret_cast<int*>(self->CallData));

  // If this is the timer event we are waiting for...
  if (timerId == self->TimerId && self->WidgetState == vtkVgSelectionWidget::Timing)
    {
    self->Interactor->DestroyTimer(self->TimerId);
    self->WidgetState = vtkVgSelectionWidget::TimedOut;

    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];

    vtkVgSelectionRepresentation* rep = self->GetSelectionRepresentation();
    int state = rep->ComputeInteractionState(X, Y);
    if (state == vtkVgSelectionRepresentation::OnItem)
      {
      rep->AnnotationVisibilityOn();
      }
    else
      {
      rep->AnnotationVisibilityOff();
      }

    self->InvokeEvent(vtkCommand::TimerEvent, NULL);
    self->EventCallbackCommand->SetAbortFlag(1); //no one else gets this timer
    self->Render();
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionWidget::SelectAction(vtkAbstractWidget* w)
{
  vtkVgSelectionWidget* self = reinterpret_cast<vtkVgSelectionWidget*>(w);

  // If widget is hovering we grab the selection event
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  vtkVgSelectionRepresentation* rep = self->GetSelectionRepresentation();
  int state = rep->ComputeInteractionState(X, Y);
  if (state == vtkVgSelectionRepresentation::OnItem)
    {
    rep->AnnotationVisibilityOn();
    rep->SelectCurrentItem();
    self->InvokeEvent(vtkCommand::WidgetActivateEvent, NULL);
    self->EventCallbackCommand->SetAbortFlag(1); //no one else gets this event
    self->Render();
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Timer Duration: " << this->TimerDuration << "\n";
}

//----------------------------------------------------------------------
void vtkVgSelectionWidget::CreateDefaultRepresentation()
{
  if (! this->WidgetRep)
    {
    this->WidgetRep = vtkVgSelectionListRepresentation::New();
    }
}
