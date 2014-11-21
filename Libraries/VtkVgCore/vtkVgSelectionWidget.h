/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgSelectionWidget - general widget to select items from a list
// .SECTION Description
// This class is used to select items from a list, where the list may be
// structured (e.q., a linear list or rectangular matrix) or unstructured
// (e.g., an arbitray scene). Generally the widget is paired with an
// appropriate representation (i.e., subclass of vtkVgSelectionRepresentation).
// The representation defines the visual appearance of the widget and coordinates
// with the widget to manage the selection process.
//
// .SECTION Event Bindings
// By default, the widget responds to the following VTK events (i.e., it
// watches the vtkRenderWindowInteractor for these events):
// <pre>
// On the interior of the widget:
//   LeftButtonPressEvent - select an item in the list that the mouse pointer is over.
//   KeyPress <Enter> - select the item in the list that the mouse pointer is over
// On the boundary of the widget:
//   LeftButtonPressEvent - select boundary, can be used to move/resize the widget
//                          (depending on the representation).
//   LeftButtonReleaseEvent - deselect boundary
//   MouseMoveEvent - move/resize widget depending on which portion of the
//                    boundary was selected.
// Anywhere on the widget:
//   MiddleButtonPressEvent - move the widget
// </pre>
//
// Note that the event bindings described above can be changed using this
// class's vtkWidgetEventTranslator. This class translates VTK events
// into the vtkVgSelectionWidget's widget events:
// <pre>
//   vtkWidgetEvent::Select -- the boundary or interior of the widget has been selected
//   vtkWidgetEvent::EndSelect -- the selection process has completed
//   vtkWidgetEvent::Translate -- the widget is to be translated
//   vtkWidgetEvent::TimedOut -- the widget timer has expired
// </pre>
//
// In turn, when these widget events are processed, this widget invokes the
// following VTK events on itself (which observers can listen for):
// <pre>
//   vtkCommand::WidgetActivationEvent (on vtkWidgetEvent::Select when selecting interior)
//   vtkCommand::StartInteractionEvent (on vtkWidgetEvent::Select when selecting
//      the boundary)
//   vtkCommand::EndInteractionEvent (on vtkWidgetEvent::EndSelect)
//   vtkCommand::InteractionEvent (on vtkWidgetEvent::Move)
// </pre>

// .SECTION See Also
// vtkInteractorObserver vtkVgSelectionRepresentation


#ifndef __vtkVgSelectionWidget_h
#define __vtkVgSelectionWidget_h

#include "vtkAbstractWidget.h"

#include <vgExport.h>

class vtkVgSelectionRepresentation;


class VTKVG_CORE_EXPORT vtkVgSelectionWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Method to instantiate class.
  static vtkVgSelectionWidget* New();

  // Description;
  // Standard methods for class.
  vtkTypeMacro(vtkVgSelectionWidget, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The method for activiating and deactiviating this widget. This method
  // must be overridden because it performs special timer-related operations.
  virtual void SetEnabled(int);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkVgSelectionRepresentation* r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

  // Description:
  // Return the representation as a vtkBorderRepresentation.
  vtkVgSelectionRepresentation* GetSelectionRepresentation()
    {return reinterpret_cast<vtkVgSelectionRepresentation*>(this->WidgetRep);}

  // Description:
  // Specify the hovering interval (in milliseconds). If after moving the
  // mouse the pointer stays over a vtkProp for this duration, then a
  // vtkTimerEvent::TimerEvent is invoked.
  vtkSetClampMacro(TimerDuration, int, 1, 100000);
  vtkGetMacro(TimerDuration, int);

  // Description:
  // Create the default widget representation if one is not set.
  virtual void CreateDefaultRepresentation();

protected:
  vtkVgSelectionWidget();
  ~vtkVgSelectionWidget();

  // The state of the widget
//BTX
  enum {Start = 0, Timing, TimedOut};
//ETX
  int WidgetState;

  // Callback interface to execute events
  static void MoveAction(vtkAbstractWidget*);
  static void HoverAction(vtkAbstractWidget*);
  static void SelectAction(vtkAbstractWidget*);

  // Description:
  // Helper methods for creating and destroying timers.
  int TimerId;
  int TimerDuration;

  // helper methods for cursoe management
  virtual void SetCursor(int State);

private:
  vtkVgSelectionWidget(const vtkVgSelectionWidget&);  //Not implemented
  void operator=(const vtkVgSelectionWidget&);  //Not implemented
};

#endif
