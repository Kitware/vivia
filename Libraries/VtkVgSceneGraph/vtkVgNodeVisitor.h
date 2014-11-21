/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
