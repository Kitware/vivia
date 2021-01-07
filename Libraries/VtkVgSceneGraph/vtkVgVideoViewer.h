// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgVideoViewer_h
#define __vtkVgVideoViewer_h

#include "vtkVgViewer.h"

#include <vgExport.h>

class VTKVG_SCENEGRAPH_EXPORT vtkVgVideoViewer : public vtkVgViewer
{
public:
  // Description:
  // Usual VG macro.
  vtkVgClassMacro(vtkVgVideoViewer);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgVideoViewer, vtkVgViewer);

  static vtkVgVideoViewer* New();

  // Description:
  // Overridden function. Since we don't use
  // VTK render window interactor directly
  // we don't initialize it.
  virtual void Realize();

  // Overridden function. Since we don't use
  // VTK render window interactor directly
  // we don't start VTK event loop.
  virtual void Run();

protected:

  vtkVgVideoViewer();
  virtual ~vtkVgVideoViewer();

private:

  vtkVgVideoViewer(const vtkVgVideoViewer&); // Not implemented.
  void operator=(const vtkVgVideoViewer&); // Not implemented.
};

#endif //__vqViewer_h
