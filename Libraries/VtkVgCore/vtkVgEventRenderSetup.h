// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
