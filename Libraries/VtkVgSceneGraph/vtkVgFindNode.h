/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgFindNode_h
#define __vtkVgFindNode_h

#include <vgExport.h>

// Forward declarations.
class vtkVgGroupNode;
class vtkVgNodeBase;

class VTKVG_SCENEGRAPH_EXPORT vtkVgFindNode
{
public:

  static bool ContainsNode(vtkVgGroupNode* root, vtkVgNodeBase* searchNode);
};

#endif // __vtkVgFindNode_h
