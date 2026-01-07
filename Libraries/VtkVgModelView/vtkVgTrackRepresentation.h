// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTrackRepresentation_h
#define __vtkVgTrackRepresentation_h

#include "vtkVgTrackRepresentationBase.h"

#include "vtkVgTimeStamp.h"

#include <vgExport.h>

class vtkActor;
class vtkDataArray;
class vtkIdList;
class vtkPoints;
class vtkRenderer;
class vtkTransform;
class vtkUnsignedCharArray;
class vtkVgCellPicker;

class VTKVG_MODELVIEW_EXPORT vtkVgTrackRepresentation
  : public vtkVgTrackRepresentationBase
{
public:
  vtkVgClassMacro(vtkVgTrackRepresentation);
  vtkTypeMacro(vtkVgTrackRepresentation, vtkVgTrackRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgTrackRepresentation* New();

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
  // Update the representation in preparation for rendering.
  virtual void Update();

  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked trackId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account
  // whether the track is currently visible or not.
  virtual vtkIdType Pick(double renX, double renY, vtkRenderer* ren, vtkIdType& pickType);

  // Description:
  // After a successful pick occurs, return the position of the pick (in
  // world coordinates).
  vtkGetVector3Macro(PickPosition, double);

  // Description:
  // Set/Get whether display of tracks should be limited to only those
  // tracks that are "forced" on.  In other words, if this variable is true,
  // tracks that are "on" in the model will not be displayed (unless forced)
  vtkSetMacro(OnlyDisplayForcedTracks, bool);
  vtkGetMacro(OnlyDisplayForcedTracks, bool);
  vtkBooleanMacro(OnlyDisplayForcedTracks, bool);

  // Description:
  // Set/Clear tracks which are "forced" on, regardless of visibility state in
  // the model.
  void ForceShowTracks(vtkIdList* ids);
  void ClearForceShownTracks();

  // Description:
  // Set/Get the Z offset to be applied to the track and expiring track actors
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(ZOffset, double);
  vtkGetMacro(ZOffset, double);

  // Description:
  // Set/Get the line width to be applied to the track actors.
  void  SetActiveTrackLineWidth(float width);
  float GetActiveTrackLineWidth();

  // Description:
  // Set/Get the line width to be applied to the track actors.
  void  SetExpiringTrackLineWidth(float width);
  float GetExpiringTrackLineWidth();

private:
  vtkVgTrackRepresentation(const vtkVgTrackRepresentation&); // Not implemented.
  void operator=(const vtkVgTrackRepresentation&);           // Not implemented.

  vtkVgTrackRepresentation();
  virtual ~vtkVgTrackRepresentation();

  void AddTrackRep(vtkVgTrack* track, vtkVgTimeStamp& currFrame,
                   bool displayRegardlessOfFrame, const vtkVgTrackInfo &info,
                   vtkUnsignedCharArray* activeColors,
                   vtkUnsignedCharArray* expiredColors);

  void SetupPipelineForFiltersAndSelectors();

  void AppendArray(vtkDataArray* base, vtkDataArray* add, void* addPointer);
  vtkActor* TrackActor;
  vtkActor* ExpiringTrackActor;

  vtkSmartPointer<vtkTransform> ActorTransform;
  double ZOffset;

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  class vtkInternal;
  vtkInternal* Internal;

  vtkSmartPointer<vtkVgCellPicker> Picker;
  double PickPosition[3];
  bool OnlyDisplayForcedTracks;

  int Visible;
};

#endif // __vtkVgTrackRepresentation_h
