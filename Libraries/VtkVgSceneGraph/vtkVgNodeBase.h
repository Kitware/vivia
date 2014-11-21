/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgNodeBase_h
#define __vtkVgNodeBase_h

// VG includes.
#include "vtkVgMacros.h"

// VTK includes.
#include <vtkObject.h>
#include <vtkSmartPointer.h>

#include <vgExport.h>

// Forward declarations.
class vtkVgTimeStamp;
class vtkVgGroupNode;
class vtkVgNodeVisitorBase;

class vtkMatrix4x4;

class VTKVG_SCENEGRAPH_EXPORT vtkVgNodeBase : public vtkObject
{
public:
  typedef unsigned int NodeMaskType;

  // Description:
  // Defines reference frame of this wrt its parent.
  enum ReferenceFrame
    {
    ABSOLUTE,
    RELATIVE
    };

  // Description:
  // Easy to use types.
  vtkVgClassMacro(vtkVgNodeBase);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgNodeBase, vtkObject);

  static vtkVgNodeBase* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get name for the node.
  vtkSetStringMacro(Name);
  const char* GetName() const;

  // Description:
  // Set/Get visibility flag.
  virtual int SetVisible(int flag);
  vtkGetMacro(Visible, int);
  vtkBooleanMacro(Visible, int);

  // Description:
  // Flag that indicates is state of this node is dirty.
  vtkSetMacro(Dirty, int);
  vtkGetMacro(Dirty, int);
  vtkBooleanMacro(Dirty, int);

  // Description:
  // Flag that indicates is state of this node is dirty.
  void SetBoundsDirty(int flag);
  vtkGetMacro(BoundsDirty, int);
  virtual void BoundsDirtyOn();
  virtual void BoundsDirtyOff();

  // Description:
  // Return the accumulated matrix (top down). This
  // matrix could be used for rendering.
  // \NOTE: Changing the value of the matrix does not make the
  // state dirty. Hence use carefully.
  virtual const vtkMatrix4x4* GetFinalMatrix() const;
  virtual vtkMatrix4x4* GetFinalMatrix();

  // Description:
  // Set/Get this node's parent.
  void SetParent(vtkVgGroupNode* parent);
  vtkVgGroupNode* GetParent();
  const vtkVgGroupNode* GetParent() const;

  // Description:
  // Set/Get reference frame.
  vtkSetMacro(NodeReferenceFrame, int);
  vtkGetMacro(NodeReferenceFrame, int);

  // Description:
  // Set/Get state if this node is set to be removed from its parent.
  vtkSetMacro(RemoveNode, int);
  vtkGetMacro(RemoveNode, int);
  vtkBooleanMacro(RemoveNode, int);

  // Description:
  // Set/Get node bounds.
  vtkSetVector6Macro(Bounds, double);
  virtual void GetBounds(double bounds[6]);
  virtual const double* GetBounds() const;
  virtual double* GetBounds();

  // Description:
  // Set/Get node visibility node mask.
  vtkSetMacro(VisibleNodeMask, NodeMaskType);
  vtkGetMacro(VisibleNodeMask, NodeMaskType);

  // Description:
  // Update this node and use \param timeStamp information if needed.
  virtual void Update(vtkVgNodeVisitorBase& nodeVisitor);

  // Description:
  // Start traversing the node using \param nodeVisitor.
  virtual void Accept(vtkVgNodeVisitorBase&  vtkNotUsed(nodeVisitor)) {;}

  // Description:
  // Traverse this node using \param nodeVisitor.
  virtual void Traverse(vtkVgNodeVisitorBase&  nodeVisitor);

  // Description:
  // Computer bounds of this node.
  virtual void ComputeBounds() {;}

  static const NodeMaskType     VISIBLE_NODE_MASK    = 0xffffffff;

  // Not used right now.
  static const NodeMaskType     TRAVERSAL_NODE_MASK  = 0xeeeeeeee;

  // Not used right now.
  static const NodeMaskType     UPDATE_NODE_MASK     = 0xdddddddd;

protected:
  vtkVgNodeBase();
  virtual ~vtkVgNodeBase();

  // Description:
  char*                         Name;

  // Description:
  // Check if the node is dirty. This means that node and its children (if applicable)
  // needs traversal and are dirty as well. Once traversed this should set to false.
  int                           Dirty;

  // Description:
  // Check if the bounds of this node is dirty. If dirty calculate new bounds.
  int                           BoundsDirty;

  // Description:
  // Check if this node and if applicable its children will be visible.
  int                           Visible;

  int                           RemoveNode;

  // Description:
  // Complete matrix transform for this node. If the parent is dirty
  // this needs to be recalculated.
  vtkSmartPointer<vtkMatrix4x4> FinalMatrix;

  // Description:
  vtkVgGroupNode*               Parent;

  // Description:
  int                           NodeReferenceFrame;

  double                        Bounds[6];  \

  NodeMaskType                  VisibleNodeMask;


private:

  // Description:
  vtkVgNodeBase(const vtkVgNodeBase&);    // Not implemented.
  void operator=(const vtkVgNodeBase&);   // Not implemented.
};

#endif // __vtkVgNodeBase_h
