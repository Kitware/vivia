/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgRepresentationBase_h
#define __vtkVgRepresentationBase_h

// VTK includes.
#include <vtkObject.h>
#include <vtkPropCollection.h>
#include <vtkSmartPointer.h>

// VG includes.
#include "vtkVgMacros.h"

// C++ includes
#include <limits>

#include <vgExport.h>

// Forward declarations.
class vtkInformationDoubleKey;
class vtkLookupTable;
class vtkMatrix4x4;
class vtkPolyDataCollection;
class vtkRenderer;
class vtkTransform;

class VTKVG_MODELVIEW_EXPORT vtkVgRepresentationBase : public vtkObject
{
public:
  vtkVgClassMacro(vtkVgRepresentationBase);

  // Usual VTK functions.
  vtkTypeMacro(vtkVgRepresentationBase, vtkObject);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This Key is for indicating a Z offset on the prop to be added to any offset
  // the representation controls (see subclasses)
  static vtkInformationDoubleKey* Z_OFFSET();
  static void SetZOffset(vtkProp* prop, double offset);
  static double GetZOffset(vtkProp* prop);
  static bool HasZOffset(vtkProp* prop);

  // Description:
  vtkSetMacro(LayerIndex, int);
  vtkGetMacro(LayerIndex, int);

  // Description:
  // UpdateRequest = 1 indicates that state has changed and
  // requires rendering. This should be set to 0 once
  // render call is invoked.
  vtkSetMacro(UpdateRequest, int);
  vtkGetMacro(UpdateRequest, int);
  vtkBooleanMacro(UpdateRequest, int);

  // Description:
  // Return all the objects that can be rendered.
  virtual const vtkPropCollection* GetNewRenderObjects()     const =  0;
  virtual const vtkPropCollection* GetActiveRenderObjects()  const =  0;
  virtual const vtkPropCollection* GetExpiredRenderObjects() const =  0;

  virtual vtkPropCollection* GetNewRenderObjects()     =  0;
  virtual vtkPropCollection* GetActiveRenderObjects()  =  0;
  virtual vtkPropCollection* GetExpiredRenderObjects() =  0;

  virtual void ResetTemporaryRenderObjects() = 0;

  // Description:
  // Set the matrix for represenation.
  void SetRepresentationMatrix(vtkMatrix4x4* matrix);
  vtkMatrix4x4* GetRepresentationMatrix();
  const vtkMatrix4x4* GetRepresentationMatrix() const;

  // Description:
  // Set look table for scalars to colors conversion
  void SetLookupTable(vtkLookupTable* lookupTable);
  vtkLookupTable* GetLookupTable();
  const vtkLookupTable* GetLookupTable() const;

  // Description:
  // Set/Get visible flag. This makes the representation not to show
  // in any given render frame if flag is set to 0.
  virtual void SetVisible(int flag) = 0;
  virtual int  GetVisible() const   = 0;

  // Description:
  // This is the matrix that could be used to offset the representation
  // from its true location / position. This could be useful for some
  // layout algorithms / scene graphs.
  void ApplyOffset(const vtkMatrix4x4* matrix, bool pre = true);

  // \NOTE: One thing I am not so sure if this update does not require
  // timestamp. May be it does.
  virtual void Update() = 0;

  // Handle the case where an error in a model causes no frame data
  // for the representation.
  virtual void HandleModelError() {;}

  // Handle animation start event
  virtual void HandleStartAnimationCueEvent() {;}

  // Handle animation tick event
  virtual void HandleAnimationCueTickEvent() {;}

  // Handle animation end event
  virtual void HandleEndAnimationCueEvent() {;}

  // Description:
  vtkSetMacro(UseAutoUpdate, bool);
  vtkGetMacro(UseAutoUpdate, bool);
  vtkBooleanMacro(UseAutoUpdate, bool);

  // Description:
  // Set/Get if should use model matrix for render matrix
  // calculation.
  vtkSetMacro(UseModelMatrix, bool);
  vtkGetMacro(UseModelMatrix, bool);
  vtkBooleanMacro(UseModelMatrix, bool);

  virtual unsigned long GetUpdateTime()
    {
    return this->UpdateTime.GetMTime();
    }

  virtual unsigned long GetUpdateRenderObjectsTime()
    {
    return this->UpdateRenderObjectsTime.GetMTime();
    }

  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked trackId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account
  // whether the track is currently visible or not.
  virtual vtkIdType Pick(
    double vtkNotUsed(renX), double vtkNotUsed(renY),
    vtkRenderer* vtkNotUsed(ren), vtkIdType& vtkNotUsed(pickType))
    {
    return -1;
    }

  // Description:
  // After a successful pick occurs, return the position of the pick (in
  // world coordinates).
  virtual void GetPickPosition(double pos[3])
    {
    pos[0] = VTK_DOUBLE_MIN;
    pos[1] = VTK_DOUBLE_MIN;
    pos[2] = VTK_DOUBLE_MIN;
    }

  // Description:
  // Color value modifier.
  vtkSetClampMacro(ColorMultiplier, double, 0.0, 1.0);
  vtkGetMacro(ColorMultiplier, double);

  // Description:
  // Force the representation to display all objects in a single color
  void SetOverrideColor(const double color[3]);

  // Description:
  // Set the display mask that this representation will use.
  vtkSetMacro(DisplayMask, unsigned);
  vtkGetMacro(DisplayMask, unsigned);

protected:
  vtkVgRepresentationBase();
  virtual  ~vtkVgRepresentationBase();

  void SetupActorTransform(
    vtkPolyDataCollection* polyDataCollection,
    vtkMatrix4x4* originalMatrix, vtkTransform* actorTransform);

  vtkSmartPointer<vtkMatrix4x4> RepresentationMatrix;
  vtkSmartPointer<vtkMatrix4x4> OffsetRepresentationMatrix;
  vtkSmartPointer<vtkMatrix4x4> RenderMatrix;

  bool                          UseAutoUpdate;

  int                           LayerIndex;

  int                           UpdateRequest;

  bool                          UseModelMatrix;

  vtkTimeStamp                  UpdateTime;

  // \NOTE: Not useed right now.
  vtkTimeStamp                  UpdateRenderObjectsTime;

  vtkSmartPointer<vtkLookupTable> LookupTable;

  double ColorMultiplier;

  bool HasOverrideColor;
  double OverrideColor[3];

  unsigned DisplayMask;

private:
  vtkVgRepresentationBase(const vtkVgRepresentationBase&); // Not implemented.
  void operator= (const vtkVgRepresentationBase&);        // Not implemented.
};


#endif // __vtkVgRepresentationBase_h
