/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
