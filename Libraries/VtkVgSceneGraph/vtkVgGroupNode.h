/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgGroupNode_h
#define __vtkVgGroupNode_h

#include "vtkVgNodeBase.h"

// VTK includes.
#include <vtkSmartPointer.h>

// STL includes.
#include <vector> // STL required.

#include <vgExport.h>

// Forward declarations.
class vtkCollection;

class VTKVG_SCENEGRAPH_EXPORT vtkVgGroupNode : public vtkVgNodeBase
{
public:
  // Description:
  vtkVgClassMacro(vtkVgGroupNode);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgGroupNode, vtkVgNodeBase);

  static vtkVgGroupNode* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // \TODO: We could have index based functions here too.
  void AddChild(vtkVgNodeBase* child);
  int  RemoveChild(vtkVgNodeBase* child);

  // Description:
  // Get a particular child.
  vtkVgNodeBase* GetChild(int index);

  std::vector<vtkVgNodeBase::SmartPtr>
  GetChildren();

  // Description:
  // Get number of children.
  int GetNumberOfChildren() const;

  // Description:
  virtual void MakeAllChildrenDirty();

  // Description:
  virtual void SetParent(vtkVgGroupNode* parent);

  // Description:
  // Overridden functions.
  virtual int SetVisible(int flag);

  virtual void Update(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void Accept(vtkVgNodeVisitorBase&  nodeVisitor);

  virtual void Traverse(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void ComputeBounds();

  virtual void SetRemoveNode(int flag);


protected:
  vtkVgGroupNode();
  virtual ~vtkVgGroupNode();

  void TraverseChildren(vtkVgNodeVisitorBase& nodeVisitor);

  vtkSmartPointer<vtkCollection> Children;
  vtkSmartPointer<vtkCollection> RemoveChildren;

private:
  vtkVgGroupNode(const vtkVgGroupNode&); // Not implemented.
  void operator=(const vtkVgGroupNode&); // Not implemented.
};

#endif // __vtkVgGroupNode_h
