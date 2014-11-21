/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgAnnotationActor - a composite 2D actor for displaying simple text annotations

#ifndef __vtkVgAnnotationActor_h
#define __vtkVgAnnotationActor_h

#include "vtkActor.h"

#include <vgExport.h>

class vtkActor2D;
class vtkPoints;
class vtkProperty2D;
class vtkTextActor;
class vtkTextMapper;
class vtkTextProperty;

class VTKVG_CORE_EXPORT vtkVgAnnotationActor : public vtkActor
{
public:
  vtkTypeMacro(vtkVgAnnotationActor, vtkActor);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate the class.
  static vtkVgAnnotationActor* New();

  // Description:
  // Specify/retrieve the text to display in the annotation.
  vtkGetStringMacro(Text);
  vtkSetStringMacro(Text);

  // Description:
  // Set/get the text property
  vtkTextProperty* GetTextProperty();

  // Description:
  // Set/get the frame property
  vtkProperty2D*   GetFrameProperty();

  // Description:
  // Access to the component actors
  vtkActor2D* GetFrameActor() { return this->FrameActor; }
  vtkActor2D* GetTextActor() { return this->TextActor; }

  // Description:
  // Clamp annotation to edges of viewport?
  vtkSetMacro(ClampToViewport, bool);
  vtkGetMacro(ClampToViewport, bool);
  vtkBooleanMacro(ClampToViewport, bool);

  // Description:
  // Specify the display offset of the annotation.
  vtkSetVector2Macro(Offset, int);
  vtkGetVector2Macro(Offset, int);

  // Description:
  // Automatically set the x offset so that the text will be centered on the
  // anchor point.
  vtkSetMacro(AutoCenterX, bool);
  vtkGetMacro(AutoCenterX, bool);
  vtkBooleanMacro(AutoCenterX, bool);

  // Description:
  // Automatically set the y offset so that the text will be centered on the
  // anchor point.
  vtkSetMacro(AutoCenterY, bool);
  vtkGetMacro(AutoCenterY, bool);
  vtkBooleanMacro(AutoCenterY, bool);

  // Description:
  // Methods required by vtkProp superclass.
  virtual void ReleaseGraphicsResources(vtkWindow* w);

  virtual int RenderOpaqueGeometry(vtkViewport* viewport);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport*) {return 0;};
  virtual int RenderOverlay(vtkViewport* viewport);
  virtual int HasTranslucentPolygonalGeometry();

  virtual void GetActors2D(vtkPropCollection* pc);

  // Returning a null bounds prevents the renderer from culling this actor
  virtual double* GetBounds() { return 0; }

protected:
  vtkVgAnnotationActor();
  ~vtkVgAnnotationActor();

  // Gets the final transformed world position of the actor.
  void GetTransformedPosition(double pos[3]);

  char* Text;

  int Offset[2];

  bool AutoCenterX;
  bool AutoCenterY;

  vtkTextMapper*   TextMapper;
  vtkActor2D*      TextActor;

  vtkPoints*       FramePoints;
  vtkActor2D*      FrameActor;

  vtkTimeStamp    UpdateTime;

  bool ClampToViewport;
  bool ClampedToBorder;

  // bounds of frame in pixels relative to the actor
  int Left, Right;
  int Bottom, Top;

private:
  vtkVgAnnotationActor(const vtkVgAnnotationActor&);  //Not implemented
  void operator=(const vtkVgAnnotationActor&);      //Not implemented

  void Update(vtkViewport* v);
  inline int CalcArrowLength(int dist);
};

#endif
