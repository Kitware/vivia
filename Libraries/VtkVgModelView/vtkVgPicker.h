/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgPicker - picker for visGUI application
// .SECTION Description
// This class takes advantage of the visgui data structures in order to pick
// icons, tracks, events and the locations on the image. Basically it
// interfaces with the VG Manager classes to perform the pick. It returns
// information particular to the visgui application.
//
// vtkPicker fires events during the picking process.  These events are
// StartPickEvent, PickEvent, and EndPickEvent which are invoked prior to
// picking, when something is picked, and after all picking candidates have
// been tested. Note that during the pick process the PickEvent of vtkProp
// (and its subclasses such as vtkActor) is fired prior to the PickEvent of
// vtkPicker.

#ifndef __vtkVgPicker_h
#define __vtkVgPicker_h

#include "vtkAbstractPicker.h"
#include "vtkSmartPointer.h"

// C++ includes
#include <vector>

#include <vgExport.h>

class vtkRenderer;
class vtkVgCellPicker;
class vtkVgIconManager;
class vtkVgTrackRepresentationBase;
class vtkVgEventRepresentationBase;
class vtkVgRepresentationBase;
class vtkVgActivityManager;

class vtkActor;
class vtkImageActor;

//-----------------------------------------------------------------------------
class VTKVG_MODELVIEW_EXPORT vtkVgPicker : public vtkAbstractPicker
{
public:
  // Description:
  // Standard VTK functions.
  static vtkVgPicker* New();
  vtkTypeMacro(vtkVgPicker, vtkAbstractPicker);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Perform pick operation with selection point provided. Normally the first
  // two values for the selection point are x-y pixel coordinate, and the
  // third value is =0. Return non-zero if something was successfully
  // picked. Note that return value==PickedIcon means an icon was picked. If
  // return value==PickedTrack then a track was picked. If return
  // value==PickedImage then the background image was picked. If return
  // value==PickedEvent then an event was picked. If return
  // value==PickedActivity then an activity was picked.
  virtual int
  Pick(double selectionX, double selectionY, double selectionZ, vtkRenderer* renderer);

  // Description:
  // Specify the icon manager to pick.
  void SetIconManager(vtkVgIconManager* im);
  vtkGetObjectMacro(IconManager, vtkVgIconManager);

  // Description:
  // Add representation to pick.
  void AddRepresentation(vtkVgRepresentationBase* representation);

  // Description:
  // Specify the activity manager to pick.
  void SetActivityManager(vtkVgActivityManager* tm);
  vtkGetObjectMacro(ActivityManager, vtkVgActivityManager);

  // Description:
  // Specify the image actor corresponding to the display.
  void SetImageActor(vtkImageActor* ia);
  vtkGetObjectMacro(ImageActor, vtkImageActor);

  // Description:
  // Specify the actor to pick.
  void SetActor(vtkActor* actor);
  vtkGetObjectMacro(Actor, vtkActor);

  // Description:
  // After a successful icon pick (i.e., Pick() returns PickedIcon), retrieve
  // the icon id of the picked icon. This can be used to retrieve other
  // information related to the icon.
  vtkGetMacro(IconId, vtkIdType);

  // Description:
  // After a successful pick (i.e., Pick() returns PickedTrack),
  // retrieve the id of the picked entity.
  vtkGetMacro(PickedId, vtkIdType);


  // Description:
  // After a successful track pick (i.e., Pick() returns PickedActivity),
  // retrieve the activity id of the picked activity. This can be used to
  // retrieve other information related to the activity.
  vtkGetMacro(ActivityIndex, vtkIdType);

  // Description:
  // Set/Get verbosity to on or off. If on information
  // regarding picked entity will be spit out using
  // standard output stream.
  vtkSetMacro(Verbose, int);
  vtkGetMacro(Verbose, int)
  vtkBooleanMacro(Verbose, int);

protected:
  vtkVgPicker();
  ~vtkVgPicker();

  int                       Verbose;

  vtkIdType                 PickedId;

  vtkVgIconManager*          IconManager;
  vtkIdType                 IconId;

  vtkVgActivityManager*      ActivityManager;
  vtkIdType                 ActivityIndex;

  vtkImageActor*             ImageActor;

  vtkActor*                  Actor;

  std::vector<vtkSmartPointer<vtkVgRepresentationBase> >
  Representations;


private:
  vtkVgPicker(const vtkVgPicker&);  // Not implemented.
  void operator=(const vtkVgPicker&);  // Not implemented.

  // Internal cell picker for fallback
  vtkSmartPointer<vtkVgCellPicker> Picker;

};

#endif
