/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
