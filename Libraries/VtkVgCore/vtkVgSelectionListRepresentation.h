// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// .NAME vtkVgSelectionListRepresentation - represent a vtkVgSelectionListWidget
// .SECTION Description
// This class is used to represent and render a specific type of
// vtkVgSelectionWidget. In this particular implementation, the selection list
// is a sorted list of rectangular buttons, colored according to a scalar
// value. Associated with each button can be an item ID, and an optional descriptive text
// string and/or thumbnail image.
//
// .SECTION Caveats
// The separation of the widget event handling (e.g., vtkVgSelectionListWidget) from
// the representation (vtkVgSelectionListRepresentation) enables users and developers
// to create new appearances for the widget. It also facilitates parallel
// processing, where the client application handles events, and remote
// representations of the widget are slaves to the client (and do not handle
// events).

// .SECTION See Also
// vtkVgSelectionListWidget vtkTextWidget

#ifndef __vtkVgSelectionListRepresentation_h
#define __vtkVgSelectionListRepresentation_h

#include "vtkVgSelectionRepresentation.h"
#include "vtkCoordinate.h" //Because of the viewport coordinate macro

#include <vgExport.h>

class vtkPoints;
class vtkPlaneSource;
class vtkPolyData;
class vtkPolyDataMapper2D;
class vtkActor2D;
class vtkProperty2D;
class vtkScalarsToColors;
class vtkImageData;
class vtkBalloonRepresentation;
class vtkIdTypeArray;
class vtkVgSelectionMap;
class vtkVgSelectionSet;

class VTKVG_CORE_EXPORT vtkVgSelectionListRepresentation
  : public vtkVgSelectionRepresentation
{
public:
  // Description:
  // Instantiate this class.
  static vtkVgSelectionListRepresentation* New();

  // Description:
  // Define standard methods.
  vtkTypeMacro(vtkVgSelectionListRepresentation, vtkVgSelectionRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify opposite corners of the box defining the boundary of the
  // widget. The ListPosition (lower left corner) is specified in the world
  // coordinate system, and ListSize an offset (in pixels) from the origin.
  void SetListPosition(double x, double y, double z);
  void SetListPosition(double xyz[3]);
  void GetListPosition(double xyz[3]);
  void SetListSize(double xy[2]);
  void SetListSize(double x, double y);
  void GetListSize(double xy[2]);

//BTX
  enum {HorizontalList = 0, VerticalList};
//ETX
  // Description:
  // Control the orientation of the selection list.
  vtkSetClampMacro(Orientation, int, HorizontalList, VerticalList);
  vtkGetMacro(Orientation, int);
  void SetOrientationToHorizontal()
    {this->SetOrientation(HorizontalList);}
  void SetOrientationToVertical()
    {this->SetOrientation(VerticalList);}

  // Description:
  // Specify a lookup table which is used to map a scalar value through to
  // generate an RGBA color.
  virtual void SetLookupTable(vtkScalarsToColors*);
  vtkGetObjectMacro(LookupTable, vtkScalarsToColors);

  // Description:
  // Add an item to the list; update the lists; and initialize the list. A
  // variety of methods are provided. Note that the items are added using an
  // itemId, but are sorted by scalar value. The scalar value is used to
  // generate an item color using the lookup table.
  void AddItem(vtkIdType itemID, double s, const char* str, vtkImageData* image);
  void UpdateItem(vtkIdType itemID, double s, const char* str, vtkImageData* image);
  void RemoveItem(vtkIdType itemID);
  void RemoveAllItems();

  // Description:
  // Satisfy superclass' API. The current item ID is returned.
  virtual vtkIdType GetNumberOfItems();
  virtual vtkIdType GetCurrentItemId() {return this->CurrentItemId;}
  virtual void SelectCurrentItem();
  virtual void UnselectCurrentItem();
  virtual vtkIdType GetSelectedItemId() {return this->SelectedItemId;}
  virtual void SetSelectedItemId(vtkIdType itemID);

  // Description:
  // Keep track in the selection list of the location that it current,
  // or that has been selected.
  virtual vtkIdType GetCurrentLocation() {return this->CurrentLocation;}
  virtual vtkIdType GetSelectedLocation() {return this->SelectedLocation;}
  virtual void SetSelectedLocation(int loc);

  // Description:
  // Grab the actor that defines the marker. This can be used to change its
  // color, etc.
  vtkGetObjectMacro(MarkerActor, vtkActor2D);

  // Description:
  // Specify the balloon offset from the list.
  vtkSetClampMacro(BalloonOffset, int, 0, 100);
  vtkGetMacro(BalloonOffset, int);

  // Description:
  // Retrieve the internal balloon representation. Access to this instance
  // enables the user to control image size and other properties of the
  // balloon.
  vtkGetObjectMacro(Balloon, vtkBalloonRepresentation);

  // Description:
  // Subclasses should implement these methods. See the superclasses'
  // documentation for more information.
  virtual void BuildRepresentation();
  virtual void StartWidgetInteraction(double*) {};
  virtual void WidgetInteraction(double*) {};
  virtual int ComputeInteractionState(int X, int Y, int modify = 0);
  virtual void GetSize(double size[2]); //canonical size

  // Description:
  // These methods are necessary to make this representation behave as
  // a vtkProp.
  virtual void GetActors2D(vtkPropCollection*);
  virtual void ReleaseGraphicsResources(vtkWindow*);
  virtual int RenderOverlay(vtkViewport*);
  virtual int RenderOpaqueGeometry(vtkViewport*);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport*);
  virtual int HasTranslucentPolygonalGeometry();

protected:
  vtkVgSelectionListRepresentation();
  ~vtkVgSelectionListRepresentation();

  // Layout (position of lower left and upper right corners of border)
  vtkCoordinate* PositionCoordinate;
  vtkCoordinate* Position2Coordinate;

  // For mapping to colors
  vtkScalarsToColors* LookupTable;

  // Keep track of start position when moving border
  double StartPosition[2];
  int    Orientation;

  // Classes for managing balloons
  vtkVgSelectionSet* SelectionSet; //PIMPL'd set
  vtkVgSelectionMap* SelectionMap; //PIMPL'd map

  // Represent the selection list. This is just a polydata with cells colored
  // according to the supplied scalars.
  vtkPlaneSource*      Buttons;
  vtkPolyData*         ColoredButtons;
  vtkIdTypeArray*      Ids;
  vtkPolyDataMapper2D* ButtonMapper;
  vtkActor2D*          ButtonActor;

  // The thing that pops up
  vtkBalloonRepresentation* Balloon;
  int BalloonOffset;

  // Mark the selected item
  vtkPolyData*         Marker;
  vtkPoints*           MarkerPoints;
  vtkPolyDataMapper2D* MarkerMapper;
  vtkActor2D*          MarkerActor;
  void                 DrawSelectionMarker();

  // Internal state
  vtkIdType CurrentLocation;
  vtkIdType SelectedLocation;
  vtkIdType CurrentItemId;
  vtkIdType SelectedItemId;

  // Keep track of last build time
  vtkTimeStamp BuildTime;

private:
  vtkVgSelectionListRepresentation(const vtkVgSelectionListRepresentation&);  //Not implemented
  void operator=(const vtkVgSelectionListRepresentation&);  //Not implemented
};

#endif
