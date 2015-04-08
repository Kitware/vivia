/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgEventRepresentation_h
#define __vtkVgEventRepresentation_h

#include "vtkSmartPointer.h"

#include "vtkVgEventRepresentationBase.h"

#include <vgExport.h>

class vtkCollection;
class vtkPropCollection;
class vtkRenderer;

class vtkVgCellPicker;
class vtkVgEvent;
class vtkVgEventTypeRegistry;
class vtkVgTrackRepresentation;

class VTKVG_MODELVIEW_EXPORT vtkVgEventRepresentation
  : public vtkVgEventRepresentationBase
{
public:
  vtkTypeMacro(vtkVgEventRepresentation, vtkVgEventRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgEventRepresentation* New();

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

  void Initialize();

  // Description:
  // Update all the event actors. Generally called by the application layer.
  virtual void Update();

  // Description:
  // If state = true, randomize the event colors.  Otherwise, randomly
  // color each event.
  void RandomizeEventColors(bool state);

  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked eventId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account
  // whether the event is currently visible or not.
  virtual vtkIdType Pick(double renX, double renY, vtkRenderer* ren, vtkIdType& pickType);

  // Description:
  // After a successful pick occurs, return the position of the pick (in
  // world coordinates).
  vtkGetVector3Macro(PickPosition, double);

  // Description:
  // Invert normalcy indicators (line width, opacity, etc.)
  virtual void SwapNormalcyCues();

  // Description:
  // Optionally provide a track representation, which implicitly
  // causes supporting tracks to be rendered.
  void SetTrackRepresentation(vtkVgTrackRepresentation* rep);
  vtkGetObjectMacro(TrackRepresentation, vtkVgTrackRepresentation);

  // Description:
  // Set/Get the Z offset to be applied to all the events
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(ZOffset, double);
  vtkGetMacro(ZOffset, double);

  // Description:
  // Set/Get the default line width of event tracks.
  // Note: Setting this after initialization will have no effect.
  vtkSetMacro(LineWidth, float);
  vtkGetMacro(LineWidth, float);

  // Description:
  // Set/Get the amount by which to adjust the line width per normalcy bin. The
  // default is 1.0.
  vtkSetMacro(NormalcyLineWidthScale, float);
  vtkGetMacro(NormalcyLineWidthScale, float);

  void UpdateEventTypes();

private:
  vtkVgEventRepresentation(const vtkVgEventRepresentation&); // Not implemented.
  void operator=(const vtkVgEventRepresentation&);           // Not implemented.

  vtkVgEventRepresentation();
  virtual ~vtkVgEventRepresentation();

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  int Visible;

  enum { NumberOfNormalcyBins = 5 };

  struct vtkInternal;
  vtkInternal* Internal;

  vtkSmartPointer<vtkVgCellPicker> Picker;
  double PickPosition[3];

  vtkVgTrackRepresentation* TrackRepresentation;

  double ZOffset;
  float LineWidth;
  float NormalcyLineWidthScale;
};

#endif // __vtkVgEventRepresentation_h
