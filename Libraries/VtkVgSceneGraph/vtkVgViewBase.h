// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgViewBase_h
#define __vtkVgViewBase_h

// VG includes.
#include "vtkVgSceneManager.h"

#include <vgExport.h>

// Forward declarations.
class vtkRenderer;

class vtkVQLayoutManagerBase;
class vtkVgGroupNode;
class vtkVgSceneManager;
class vtkVgTimeStamp;

class VTKVG_SCENEGRAPH_EXPORT vtkVgViewBase : public vtkVgSceneManager
{
public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vtkVgViewBase);

  // Description:
  // Usual VTK functions.
  static vtkVgViewBase* New();

  vtkTypeMacro(vtkVgViewBase, vtkVgSceneManager);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkVgViewBase();
  virtual ~vtkVgViewBase();

  // Description:
  // Update view and underlying objects.
  virtual void Update(const vtkVgTimeStamp& timestamp);

private:
  vtkVgViewBase(const vtkVgViewBase&);        // Not implemented.
  void operator= (const vtkVgViewBase&);      // Not implemented.
};

#endif // __vtkVgViewBase_h
