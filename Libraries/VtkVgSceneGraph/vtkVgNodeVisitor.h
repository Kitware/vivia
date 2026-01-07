// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgNodeVisitor_h
#define __vtkVgNodeVisitor_h

#include "vtkVgNodeVisitorBase.h"

#include <vgExport.h>

// Forward declarations.
class vtkVgGeode;
class vtkVgGroupNode;
class vtkVgLeafNodeBase;
class vtkVgTransformNode;
class vtkVgVideoNode;

class VTKVG_SCENEGRAPH_EXPORT vtkVgNodeVisitor : public vtkVgNodeVisitorBase
{
public:
  vtkVgNodeVisitor();
  virtual ~vtkVgNodeVisitor();

  // Description:
  // Overridden functions.
  virtual void Visit(vtkVgGeode& geode);
  virtual void Visit(vtkVgGroupNode&  groupNode);
  virtual void Visit(vtkVgTransformNode& transformNode);
  virtual void Visit(vtkVgVideoNode&  videoNode);
  virtual void Visit(vtkVgLeafNodeBase&  leafNode);

protected:

private:
  vtkVgNodeVisitor(const vtkVgNodeVisitor&); // Not implemented.
  void operator= (const vtkVgNodeVisitor&);  // Not implemented.
};

#endif // __vtkVgNodeVisitor_h
