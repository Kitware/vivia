/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgViewerBase_h
#define __vtkVgViewerBase_h

// VTK includes.
#include <vtkSmartPointer.h>

// VG includes.
#include "vtkVgViewBase.h"

#include <vgExport.h>

class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkVgTimeStamp;

class VTKVG_SCENEGRAPH_EXPORT vtkVgViewerBase : public vtkVgViewBase
{
public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vtkVgViewerBase);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgViewerBase, vtkVgViewBase);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get render window.
  void SetRenderWindow(vtkRenderWindow* renderWindow);
  vtkRenderWindow* GetRenderWindow();
  const vtkRenderWindow* GetRenderWindow() const;

  // Description:
  // Set/Get render window interactor.
  void SetRenderWindowInteractor(vtkRenderWindowInteractor* interactor);
  vtkRenderWindowInteractor* GetRenderWindowInteractor();
  const vtkRenderWindowInteractor* GetRenderWindowInteractor() const;

  // Description:
  // Get current scale. A scale is ratio of camera view width
  // to viewport width.
  virtual double GetCurrentScale();

  // Description:
  // Get current view extents.
  virtual void GetCurrentViewExtents(double* viewExtents) = 0;

  // Description:
  // Given a viewing volume (in world space now)  update the view.
  virtual void UpdateView(double bounds[6]) = 0;

  // Description:
  // Does update view triggers events? If yes which one?
  // -1 means no event is triggered by the update view.
  virtual int UpdateViewTriggersEvent() const = 0;

  // Description:
  // Check if this is the first frame and if it is do something special about it.
  virtual bool IsFirstFrame() = 0;

  // Description:
  // Check if one of the render window has been realized.
  virtual bool IsRealized() = 0;

  // Description:
  // Setup to initialize a render window and threads if any.
  virtual void Realize() = 0;

  // Description:
  // Derived classes need to implement this.
  virtual void Frame(const vtkVgTimeStamp& timestamp) = 0;

  // Description:
  // Executes the main loop.
  virtual void Run() = 0;

  // Description:
  // Force render regardless of state.
  virtual void ForceRender(bool resetCameraOnFirstFrame = true);

  // Description:
  // Reset camera position and other view parameters to
  // their original state.
  virtual void ResetView();

protected:
  vtkVgViewerBase();
  virtual ~vtkVgViewerBase();

  vtkSmartPointer<vtkRenderWindow>            RenderWindow;
  vtkSmartPointer<vtkRenderWindowInteractor>  Interactor;


private:
  vtkVgViewerBase(const vtkVgViewerBase&);
  void operator= (const vtkVgViewerBase&);

};


#endif // __vtkVgViewerBase_h
