// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgComputeBounds_h
#define __vtkVgComputeBounds_h

#include <vgExport.h>

// Forward declarations.
class vtkPropCollection;
class vtkVgGroupNode;

class VTKVG_SCENEGRAPH_EXPORT vtkVgComputeBounds
{
public:

  static void ComputeVisiblePropBounds(vtkPropCollection* props,
                                       double bounds[6]);

  static void ComputeVisiblePropBounds(vtkVgGroupNode* groupNode);

};

#endif // __vtkVgComputeBounds_h
