// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgVideoRepresentation0_h
#define __vtkVgVideoRepresentation0_h

#include "vtkVgVideoRepresentationBase0.h"

// VTK includes.
#include <vtkSmartPointer.h>

#include <vgExport.h>

// Forware declations.
class vtkActor;
class vtkImageActor;
class vtkImageData;
class vtkOutlineFilter;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkPropCollection;
class vtkTransform;
class vtkVgVideoSelectionSource;

class VTKVG_MODELVIEW_EXPORT vtkVgVideoRepresentation0
  : public vtkVgVideoRepresentationBase0
{
public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vtkVgVideoRepresentation0);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgVideoRepresentation0, vtkVgVideoRepresentationBase0);

  static vtkVgVideoRepresentation0* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual const vtkPropCollection* GetNewRenderObjects()      const;
  virtual const vtkPropCollection* GetActiveRenderObjects()   const;
  virtual const vtkPropCollection* GetExpiredRenderObjects()  const;
  virtual void  ResetTemporaryRenderObjects();

  virtual vtkPropCollection* GetNewRenderObjects();
  virtual vtkPropCollection* GetActiveRenderObjects();
  virtual vtkPropCollection* GetExpiredRenderObjects();

  virtual void SetVisible(int flag);
  virtual int  GetVisible() const;

  virtual void Update();

  void SetTrackVisible(int flag);
  vtkGetMacro(TrackVisible, int);
  vtkBooleanMacro(TrackVisible, int);

  void SetEventVisible(int flag);
  vtkGetMacro(EventVisible, int);
  vtkBooleanMacro(EventVisible, int);

  void SetOutlineVisible(int flag);
  int GetOutlineVisible() const;
  vtkBooleanMacro(OutlineVisible, int);

  void SetVideoVisible(int flag);
  int GetVideoVisible() const;
  vtkBooleanMacro(VideoVisible, int);

  void SetSelectionVisible(int flag);
  vtkGetMacro(SelectionVisible, int);
  vtkBooleanMacro(SelectionVisible, int);

  void SetMarkerVisible(int flag);
  vtkGetMacro(MarkerVisible, int);
  vtkBooleanMacro(MarkerVisible, int);

  // Description:
  // Set/Get the color for the outline of the clip (0 to 1).
  void SetOutlineColor(double r, double g, double b);
  void SetOutlineColor(double rgb[3]);
  vtkGetVector3Macro(OutlineColor, double);

  // Description:
  // Set/Get the whether outline will be stippled.
  void SetOutlineStippled(bool enable);
  vtkGetMacro(OutlineStippled, bool);

  // Description:
  // Set/Get the color for the outline of the clip (0 to 1).
  void SetSelectionColor(double r, double g, double b);
  void SetSelectionColor(double rgb[3]);
  vtkGetVector3Macro(SelectionColor, double);

  // Description:
  // Set/Get the Z offset to be applied to the video actor
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(VideoZOffset, double);
  vtkGetMacro(VideoZOffset, double);

  // Description:
  // Set/Get the Z offset to be applied to the outline actor
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(OutlineZOffset, double);
  vtkGetMacro(OutlineZOffset, double);

  // Description:
  // Set/Get the Z offset to be applied to the marker actor
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(MarkerZOffset, double);
  vtkGetMacro(MarkerZOffset, double);

  // Description:
  // Set/Get the Z offset to be applied to the selection actor
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(SelectionZOffset, double);
  vtkGetMacro(SelectionZOffset, double);

  vtkGetMacro(ModelError, bool);

protected:
  // Easy to use vars.
  typedef vtkSmartPointer<vtkActor>          vtkActorRef;
  typedef vtkSmartPointer<vtkImageActor>     vtkImageActorRef;
  typedef vtkSmartPointer<vtkImageData>      vtkImageDataRef;
  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;
  typedef vtkSmartPointer<vtkTransform>      vtkTransformRef;
  typedef vtkSmartPointer<vtkPolyData>       vtkPolyDataRef;
  typedef vtkSmartPointer<vtkPolyDataMapper> vtkPolyDataMapperRef;
  typedef vtkSmartPointer<vtkPoints>         vtkPointsRef;

  vtkVgVideoRepresentation0();
  virtual ~vtkVgVideoRepresentation0();

  // TODO: Get rid of this function once we get support in VTK for picking
  // actors with non-homogeneous transforms.
  void MakeLinear(vtkMatrix4x4* mat);

  virtual void HandleModelError();
  virtual void HandleAnimationCueTickEvent();
  virtual void HandleEndAnimationCueEvent();

  void CreateMarker(vtkImageData* imageData);
  void UpdateMarker(vtkImageData* imageData);

protected:
  vtkVgVideoRepresentation0(const vtkVgVideoRepresentation0&);
  void operator= (const vtkVgVideoRepresentation0&);

  vtkImageActorRef      VideoActor;
  vtkActorRef           OutlineActor;

  vtkActorRef           MarkerActor;
  vtkPolyDataMapperRef  MarkerMapper;
  vtkPolyDataRef        MarkerPolyData;
  vtkPointsRef          MarkerPoints;

  vtkTransformRef       VideoTransform;
  vtkTransformRef       OutlineTransform;
  vtkTransformRef       MarkerTransform;

  double VideoZOffset;
  double OutlineZOffset;
  double MarkerZOffset;
  double SelectionZOffset;

  vtkOutlineFilter*     OutlineFilter;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  int                   TrackVisible;
  int                   EventVisible;
  int                   OutlineVisible;
  int                   VideoVisible;
  int                   SelectionVisible;
  int                   MarkerVisible;

  double                OutlineColor[3];
  double                SelectionColor[3];

  bool                  OutlineStippled;

  vtkImageDataRef       DummyImageData;

  vtkIdType             MarkerPointId;
  double                MarkerLocation[3];

  bool                  ModelError;
};

#endif
