// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgEventRepresentationBase_h
#define __vtkVgEventRepresentationBase_h

#include "vtkVgRepresentationBase.h"

#include <vgExport.h>

class vtkRenderer;

class vtkVgContourOperatorManager;
class vtkVgEventFilter;
class vtkVgEventModel;
class vtkVgEventTypeRegistry;

class VTKVG_MODELVIEW_EXPORT vtkVgEventRepresentationBase
  : public vtkVgRepresentationBase
{
public:
  vtkTypeMacro(vtkVgEventRepresentationBase, vtkVgRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Update all the event actors. Generally called by the application layer.
  virtual void Update() = 0;

  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked eventId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account
  // whether the event is currently visible or not.
  virtual vtkIdType Pick(double renX, double renY, vtkRenderer* ren,
                         vtkIdType& pickType);

  // Description:
  // After a successful pick occurs, return the position of the pick (in
  // world coordinates).
  virtual void GetPickPosition(double pos[]);

  // Description:
  // Set the item on the representation.
  virtual void SetEventModel(vtkVgEventModel* modelItem);
  vtkGetObjectMacro(EventModel, vtkVgEventModel);

  // Description:
  // Set the event filter that this representation will use.
  virtual void SetEventFilter(vtkVgEventFilter* filter);
  vtkGetObjectMacro(EventFilter, vtkVgEventFilter);

  // Description:
  // Assign an event type registry. Must be done before initialization.
  void SetEventTypeRegistry(vtkVgEventTypeRegistry* registry);
  vtkGetObjectMacro(EventTypeRegistry, vtkVgEventTypeRegistry);

  // Description:
  // Set/Get the manager for filters and selectors
  void SetContourOperatorManager(vtkVgContourOperatorManager* manager);
  vtkGetObjectMacro(ContourOperatorManager, vtkVgContourOperatorManager);

  // Description:
  // Invert normalcy indicators.
  virtual void SwapNormalcyCues();

  // Description:
  // Enable / disable normalcy indicators (line width, opacity, icon size, etc.)
  // Off by default.
  vtkSetMacro(UseNormalcyCues, bool);
  vtkGetMacro(UseNormalcyCues, bool);

protected:
  vtkVgEventRepresentationBase();
  virtual ~vtkVgEventRepresentationBase();

  vtkVgEventModel*  EventModel;
  vtkVgEventFilter* EventFilter;
  vtkVgEventTypeRegistry* EventTypeRegistry;

  vtkVgContourOperatorManager* ContourOperatorManager;

  bool UseNormalcyCues;
  bool NormalcyCuesSwapped;

  // Description:
  // Method called (event/observer mechanism with EventModel) when an event is
  // removed.  Essentially giving the representation a chance to react to an
  // event being removed.
  virtual void EventRemoved(vtkObject* vtkNotUsed(caller),
                            unsigned long vtkNotUsed(invokedEventId),
                            void* vtkNotUsed(theEvent))
    {};

private:
  vtkVgEventRepresentationBase(const vtkVgEventRepresentationBase&);
  void operator=(const vtkVgEventRepresentationBase&);
};

#endif // __vtkVgEventRepresentationBase_h
