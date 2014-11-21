/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgViewer_h
#define __vtkVgViewer_h

// VG includes.
#include "vtkVgViewerBase.h"

// VTK includes.
#include <vtkSmartPointer.h>

#include <vgExport.h>

class VTKVG_SCENEGRAPH_EXPORT vtkVgViewer : public vtkVgViewerBase
{
public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vtkVgViewer);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgViewer, vtkVgViewerBase);

  static vtkVgViewer* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Overridden functions.
  virtual bool IsFirstFrame();

  virtual bool IsRealized();

  virtual void Realize();

  virtual void Frame(const vtkVgTimeStamp& timestamp);

  virtual void ForceRender(bool resetCameraOnFirstFrame = true);

  virtual void Run();

  virtual void GetCurrentViewExtents(double* viewExtents);

  virtual void UpdateView(double bounds[6]);

  virtual int UpdateViewTriggersEvent() const;


protected:
  vtkVgViewer();
  virtual ~vtkVgViewer();

  bool    FirstFrame;
  bool    Realized;


private:
  vtkVgViewer(const vtkVgViewer&);    // Not implemented.
  void operator=(const vtkVgViewer&); // Not implemented.
};


#endif // __vtkVgViewer_h
