// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// .NAME vtkVgSelectionRepresentation - represent a vtkVgSelectionWidget
// .SECTION Description
// This class is used to represent and render a vtkVgSelectionWidget. It
// defines an abstract API for selecting an item from a list of items.
//
// The class is must be subclassed so that specialized representations can be
// created.  The class defines an API and a default implementation that the
// vtkVgSelectionRepresentation interacts with to render itself in the scene.

// .SECTION Caveats
// The separation of the widget event handling (e.g., vtkVgSelectionWidget) from
// the representation (vtkVgSelectionRepresentation) enables users and developers
// to create new appearances for the widget. It also facilitates parallel
// processing, where the client application handles events, and remote
// representations of the widget are slaves to the client (and do not handle
// events).

// .SECTION See Also
// vtkVgSelectionWidget

#ifndef __vtkVgSelectionRepresentation_h
#define __vtkVgSelectionRepresentation_h

#include "vtkWidgetRepresentation.h"

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgSelectionRepresentation
  : public vtkWidgetRepresentation
{
public:
  // Description:
  // Define standard methods.
  vtkTypeMacro(vtkVgSelectionRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Retrieve the coordinate of selection after the representation is
  // selected.
  vtkGetVectorMacro(SelectionPoint, double, 2);

  // Description:
  // Methods to traverse the list of items.
  virtual vtkIdType GetNumberOfItems() = 0;

  // Description:
  // Retrieve the item id if the interaction state is OnItem.  The meaning
  // of the id value will vary depending on the subclasses' implementation.
  virtual vtkIdType GetCurrentItemId() = 0;

  // Description:
  // Specify whether any supplemental annotation is on or off.
  vtkSetMacro(AnnotationVisibility, int);
  vtkGetMacro(AnnotationVisibility, int);
  vtkBooleanMacro(AnnotationVisibility, int);

  // Description:
  // Mark the current item selected, or unselect. Retrieve the current
  // selected item.
  virtual void SelectCurrentItem() {}
  virtual void UnselectCurrentItem() {}
  virtual vtkIdType GetSelectedItemId() {return -1;}

  // Description:
  // Specify the minimum size for the selection marker (if any). Specified
  // in pixels.
  vtkSetClampMacro(MinimumSelectionMarkerSize, int, 1, 1000);
  vtkGetMacro(MinimumSelectionMarkerSize, int);

//BTX
  // Description:
  // Define the various states that the representation can be in.
  enum _InteractionState
    {
    Outside = 0,
    OnBorder,
    OnItem
    };
//ETX

protected:
  vtkVgSelectionRepresentation();
  ~vtkVgSelectionRepresentation();

  // The point of last selection
  double SelectionPoint[2];

  // Annotation visibility
  int AnnotationVisibility;

  // Marker size
  int MinimumSelectionMarkerSize;

private:
  vtkVgSelectionRepresentation(const vtkVgSelectionRepresentation&);  //Not implemented
  void operator=(const vtkVgSelectionRepresentation&);  //Not implemented

};

#endif
