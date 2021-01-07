// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTrackHeadRepresentation_h
#define __vtkVgTrackHeadRepresentation_h

#include "vtkVgTrackRepresentationBase.h"

#include <vgExport.h>

class vtkVgCellPicker;

class vtkActor;
class vtkPoints;
class vtkRenderer;
class vtkTransform;

class VTKVG_MODELVIEW_EXPORT vtkVgTrackHeadRepresentation
  : public vtkVgTrackRepresentationBase
{
public:
  vtkTypeMacro(vtkVgTrackHeadRepresentation, vtkVgTrackRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgTrackHeadRepresentation* New();

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
  virtual vtkIdType Pick(double renX, double renY,
                         vtkRenderer* ren, vtkIdType& pickType);

  // Description
  // Set/Get whether all heads are to be displayed.  If true, then ALL tracks
  // heads will be rendered, regardless of the display state of the
  // corresponding track. If false, only those track heads will be rendered
  // whose corresponding tracks are turned on.
  vtkSetMacro(DisplayAllHeads, bool);
  vtkGetMacro(DisplayAllHeads, bool);

  // Description:
  // Set/Get the Z offset to be applied to the actor
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(ZOffset, double);
  vtkGetMacro(ZOffset, double);

  // Description:
  // Set/Get the line width to be applied to the track actors.
  void  SetLineWidth(float width);
  float GetLineWidth();

  // Description:
  // Set/Get whether to show transparent fill of / for the head
  void SetShowFill(bool showFill);
  vtkGetMacro(ShowFill, bool);

  // Description:
  // Set/Get whether to show transparent fill of / for the head
  void SetFillOpacity(double opacity);
  vtkGetMacro(FillOpacity, double);

private:
  vtkVgTrackHeadRepresentation(const vtkVgTrackHeadRepresentation&); // Not implemented.
  void operator=(const vtkVgTrackHeadRepresentation&);               // Not implemented.

  vtkVgTrackHeadRepresentation();
  virtual ~vtkVgTrackHeadRepresentation();

  vtkActor* HeadActor;
  vtkActor* HeadFillActor;

  bool DisplayAllHeads;
  bool ShowFill;
  double FillOpacity;

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  vtkTransform*         ActorTransform;
  double                ZOffset;

  vtkSmartPointer<vtkVgCellPicker> Picker;

  class vtkInternal;
  vtkInternal* Internal;

  int Visible;
};

#endif // __vtkVgTrackHeadRepresentation_h
