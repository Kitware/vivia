// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
