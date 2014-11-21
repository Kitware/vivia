/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgActivity_h
#define __vtkVgActivity_h

#include "vtkObject.h"

#include "vtkVgTimeStamp.h"

#include <vgExport.h>

class vtkIdList;
class vtkPoints;
class vtkVgEvent;
class vtkVgIconManager;
class vtkRenderer;
class vtkVgLabeledRegion;

class VTKVG_CORE_EXPORT vtkVgActivity : public vtkObject
{
public:

  // Description:
  // Standard VTK functions.
  static vtkVgActivity* New();
  vtkTypeMacro(vtkVgActivity, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(Type, int);
  vtkGetMacro(Type, int);

  vtkSetMacro(Id, int);
  vtkGetMacro(Id, int);

  vtkSetMacro(IconIndex, int);
  vtkGetMacro(IconIndex, int);

  vtkGetObjectMacro(ActivityDisplayIds, vtkIdList);

  vtkGetObjectMacro(Actor, vtkVgLabeledRegion);

  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  void SetVisibility(bool visible);

  // Description:
  // Update activity to indicated frame. Returns true if a new actor was added.
  bool SetCurrentDisplayFrame(const vtkVgTimeStamp& timeStamp);
  vtkVgTimeStamp GetCurrentDisplayFrame()
    {
    return this->CurrentDisplayFrame;
    }

  // Description:
  // Specify renderer viewport that the highlight actor will render into.
  void SetRenderer(vtkRenderer* ren);
  vtkRenderer* GetRenderer();

  // Description:
  // Specify an icon manager to coordinate with.
  void SetIconManager(vtkVgIconManager* iconManager);
  vtkGetObjectMacro(IconManager, vtkVgIconManager);

  void SetOverlayOpacity(double opacity);

  // Description:
  // Add a new event into the list of events related with the activity.
  void AddEvent(vtkVgEvent* theEvent);

  // Description:
  // Remove an event from the activity.
  void RemoveEvent(unsigned int index);

  unsigned int GetNumberOfEvents();

  // Description:
  // After adding all events or changing the representation type, this function
  // does the heavy lifting of preparing the activity for display
  void PrepareRepresentation();

  // Description:
  // Given a valid index, fill in track and its related parameters.
  // \TODO: This interface might change in the future.
  vtkVgEvent* GetEvent(unsigned int index);

  // Description:
  // Get start and end frame for the activity
  void GetActivityFrameExtents(vtkVgTimeStamp& startFrame,
                               vtkVgTimeStamp& endFrame)
    {
    startFrame = this->ActivityFrameExtents[0];
    endFrame = this->ActivityFrameExtents[1];
    }

  // Description:
  // Set/Get the color to use for rendering non-icon part of activity
  vtkSetVector3Macro(Color, double);
  vtkGetVector3Macro(Color, double);

  // Description:
  // Set/Get the bounds of the activity (to be used for "Box" display)
  // (xmin, xmax, ymin, ymax)
  vtkSetVector4Macro(SpatialBounds, double);
  vtkGetVector4Macro(SpatialBounds, double);

  // Description:
  // Set/Get whether to use "Bounds" mode for diaplying the activity.
  // For BoundsMode, we just show a "box" based on the SpatialBounds.
  vtkBooleanMacro(BoundsMode, bool);
  vtkSetMacro(BoundsMode, bool);
  vtkGetMacro(BoundsMode, bool);

  // Description:
  // Set/Get the color to use for rendering non-icon part of activity
  vtkSetVector3Macro(SecondaryColor, double);
  vtkGetVector3Macro(SecondaryColor, double);

  vtkSetMacro(MultiColorEnabled, bool);
  vtkGetMacro(MultiColorEnabled, bool);

  // Description:
  // Update activity actor with most recent color settings
  void ApplyColors();

  // Description:
  // Set/Get the status of the activity (adjudicated, excluded, or otherwise)
  vtkSetMacro(Status, int);
  vtkGetMacro(Status, int);

  // Description:
  // Set/Get the saliency of the activity.
  vtkSetMacro(Saliency, double);
  vtkGetMacro(Saliency, double);

  // Description:
  // Set/Get the probability of the activity.
  vtkSetMacro(Probability, double);
  vtkGetMacro(Probability, double);

  // Description:
  // Show the icon and text description of this activity.
  void SetShowLabel(bool enable);

  // Description:
  // Always display the activity, regardless of current frame.
  vtkSetMacro(ShowAlways, bool);
  vtkGetMacro(ShowAlways, bool);

  void SetExpirationOffset(const vtkVgTimeStamp& timeStamp);
  vtkVgTimeStamp GetExpirationOffset()
    {
    return this->ExpirationOffset;
    }

private:
  vtkVgActivity(const vtkVgActivity&); // Not implemented.
  void operator=(const vtkVgActivity&);  // Not implemented.

  // Description:
  // Constructor / Destructor.
  vtkVgActivity();
  ~vtkVgActivity();

  void UpdateFrameExtentsAndBounds();

  char* Name;

  int Type;
  int Id;
  int Status;
  int IconIndex;

  double Saliency;
  double Probability;

  vtkVgTimeStamp ActivityFrameExtents[2];
  double SpatialBounds[4];

  vtkIdList* ActivityDisplayIds;
  vtkVgTimeStamp CurrentDisplayFrame;
  vtkVgTimeStamp ExpirationOffset;

  vtkVgLabeledRegion* Actor;
  vtkVgIconManager* IconManager;

  double OverlayOpacity;

  double Color[3];
  double SecondaryColor[3];

  bool MultiColorEnabled;
  bool ShowAlways;
  bool BoundsMode;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgActivity_h
