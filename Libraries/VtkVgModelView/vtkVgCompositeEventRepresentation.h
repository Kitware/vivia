/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgCompositeEventRepresentation_h
#define __vtkVgCompositeEventRepresentation_h

#include "vtkVgEventRepresentationBase.h"

#include <vgExport.h>

class VTKVG_MODELVIEW_EXPORT vtkVgCompositeEventRepresentation
  : public vtkVgEventRepresentationBase
{
public:
  vtkTypeMacro(vtkVgCompositeEventRepresentation, vtkVgEventRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgCompositeEventRepresentation* New();

  // Description:
  // Return all the objects that can be rendered.
  virtual const vtkPropCollection* GetNewRenderObjects() const;
  virtual const vtkPropCollection* GetActiveRenderObjects() const;
  virtual const vtkPropCollection* GetExpiredRenderObjects() const;

  virtual vtkPropCollection* GetNewRenderObjects();
  virtual vtkPropCollection* GetActiveRenderObjects();
  virtual vtkPropCollection* GetExpiredRenderObjects();

  virtual void ResetTemporaryRenderObjects();

  // Description:
  virtual void SetVisible(int flag);
  virtual int  GetVisible() const { return this->Visible; }

  // Description:
  // Update all the event actors. Generally called by the application layer.
  virtual void Update();

  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked eventId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account
  // whether the event is currently visible or not.
  virtual vtkIdType Pick(double renX, double renY, vtkRenderer* ren, vtkIdType& pickType);

  // Description:
  // Set the item on the representation.
  virtual void SetEventModel(vtkVgEventModel* modelItem);
  vtkGetObjectMacro(EventModel, vtkVgEventModel);

  // Description:
  // Set the event filter that this representation will use.
  virtual void SetEventFilter(vtkVgEventFilter* filter);
  vtkGetObjectMacro(EventFilter, vtkVgEventFilter);

  void AddEventRepresentation(vtkVgEventRepresentationBase* rep);
  void RemoveEventRepresentation(vtkVgEventRepresentationBase* rep);

protected:
  vtkVgCompositeEventRepresentation();
  virtual ~vtkVgCompositeEventRepresentation();

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;
  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  int Visible;

  class vtkInternal;
  vtkInternal* Internal;

private:
  vtkVgCompositeEventRepresentation(const vtkVgCompositeEventRepresentation&);
  void operator=(const vtkVgCompositeEventRepresentation&);
};

#endif // __vtkVgCompositeEventRepresentation_h
