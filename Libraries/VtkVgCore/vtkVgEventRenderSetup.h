/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgEventRenderSetup_h
#define __vtkVgEventRenderSetup_h

// VTK includes.
#include <vtkObject.h>

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgEventRenderSetup : public vtkObject
{
public:

  // Description:
  // Standard VTK functions.
  static vtkVgRenderSetup* New();

  vtkTypeMacro(vtkVgRenderSetup, vtkObject);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  void SetColor(int type, const double& r, const double& g, const double& b);
  void GetColor(int type, const double& r, const double& g, const double& b);

protected:
  // Description:
  // Constructor / Destructor.
  vtkVgEventRenderSetup();
  ~vtkVgEventRenderSetup();

private:
  vtkVgEventRenderSetup(const vtkVgEventRenderSetup&);  // Not implemented.
  void operator=(const vtkVgEventRenderSetup&);         // Not implemented.

  class vtkInternal;
  vtkVgEventRenderSetup::vtkInternal* Internal;
};


#endif // __vtkVgEventRenderSetup_h
