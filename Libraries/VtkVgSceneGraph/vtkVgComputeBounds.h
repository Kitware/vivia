/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
