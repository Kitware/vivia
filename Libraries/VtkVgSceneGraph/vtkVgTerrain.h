// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTerrain_h
#define __vtkVgTerrain_h

// VG includes.
#include "vtkVgGeode.h"

#include <vgExport.h>

class VTKVG_SCENEGRAPH_EXPORT vtkVgTerrain : public vtkVgGeode
{
public:
  // Description:
  // Define easy to use types
  vtkVgClassMacro(vtkVgTerrain);

  // Description:
  // Usual VTK methods
  vtkTypeMacro(vtkVgTerrain, vtkVgGeode);

  static vtkVgTerrain* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkVgTerrain();
  virtual ~vtkVgTerrain();

private:
  vtkVgTerrain(const vtkVgTerrain&);   // Not implemented.
  void operator=(const vtkVgTerrain&); // Not implemented.
};

#endif // __vtkVgTerrain_h
