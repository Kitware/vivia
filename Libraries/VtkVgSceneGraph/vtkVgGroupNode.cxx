// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgGroupNode.h"

// VG includes.
#include "vtkVgComputeBounds.h"
#include "vtkVgNodeVisitorBase.h"

// VTk includes.
#include <vtkCollection.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

#include <string>

vtkStandardNewMacro(vtkVgGroupNode);

//-----------------------------------------------------------------------------
vtkVgGroupNode::vtkVgGroupNode() : vtkVgNodeBase()
{
  this->Children        = vtkSmartPointer<vtkCollection>::New();
  this->RemoveChildren  = vtkSmartPointer<vtkCollection>::New();
}

//-----------------------------------------------------------------------------
vtkVgGroupNode::~vtkVgGroupNode()
{
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::AddChild(vtkVgNodeBase* child)
{
  if (child)
    {
    // Don't add this child twice.
    if (child->GetParent() && (child->GetParent() == this))
      {
      return;
      }

    // If this child has another parent, remove this child from
    // its parent.
    if (child->GetParent())
      {
      child->GetParent()->RemoveChild(child);
      }

    child->SetRemoveNode(0);

    // Set this as new parent.
    child->SetParent(this);

    this->Children->AddItem(child);

    this->BoundsDirtyOn();
    }
}

//-----------------------------------------------------------------------------
int vtkVgGroupNode::RemoveChild(vtkVgNodeBase* child)
{
  if (child)
    {
    // Lazy removal.
    child->SetRemoveNode(1);

    // Set parent to null for children that belong to this node only.
    if (child->GetParent() && child->GetParent() == this)
      {
      child->SetParent(0);
      }

    this->RemoveChildren->AddItem(child);

    this->Children->RemoveItem(child);

    this->BoundsDirtyOn();

    return VTK_OK;
    }

  return VTK_ERROR;
}

//-----------------------------------------------------------------------------
vtkVgNodeBase* vtkVgGroupNode::GetChild(int index)
{
  if (index >= this->GetNumberOfChildren())
    {
    return 0;
    }

  return static_cast<vtkVgNodeBase*>(this->Children->GetItemAsObject(index));
}

//-----------------------------------------------------------------------------
std::vector<vtkVgNodeBase::SmartPtr> vtkVgGroupNode::GetChildren()
{
  std::vector<vtkVgNodeBase::SmartPtr> children;
  int numberOfChildren = this->GetNumberOfChildren();
  if (numberOfChildren)
    {
    this->Children->InitTraversal();

    for (int i = 0; i < numberOfChildren; ++i)
      {
      children.push_back(this->GetChild(i));
      }
    }

  return children;
}

//-----------------------------------------------------------------------------
int vtkVgGroupNode::GetNumberOfChildren() const
{
  return this->Children->GetNumberOfItems();
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::MakeAllChildrenDirty()
{
  int numberOfItems = this->Children->GetNumberOfItems();
  if (numberOfItems)
    {
    this->Children->InitTraversal();

    for (int i = 0; i < numberOfItems; ++i)
      {
      (static_cast<vtkVgNodeBase*>(this->Children->GetNextItemAsObject()))->DirtyOn();
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::SetParent(vtkVgGroupNode* parent)
{
  this->Superclass::SetParent(parent);

  this->DirtyOn();

  this->MakeAllChildrenDirty();
}

//-----------------------------------------------------------------------------
int vtkVgGroupNode::SetVisible(int flag)
{
  if (!this->Superclass::SetVisible(flag))
    {
    return 0;
    }

  int numberOfItems = this->Children->GetNumberOfItems();
  if (numberOfItems)
    {
    this->Children->InitTraversal();

    for (int i = 0; i < numberOfItems; ++i)
      {
      vtkVgNodeBase* node =
        static_cast<vtkVgNodeBase*>(this->Children->GetNextItemAsObject());
      node->SetVisible(flag);
      }
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::Update(vtkVgNodeVisitorBase& nodeVisitor)
{
  this->Superclass::Update(nodeVisitor);

  if (this->GetBoundsDirty())
    {
    this->ComputeBounds();
    }

  if (((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE_REFERENCE) && this->Parent && this->Parent->GetDirty()) ||
      ((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE_REFERENCE) && this->Parent && this->Dirty))
    {
    this->FinalMatrix->DeepCopy(this->Parent->GetFinalMatrix());
    }
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::Accept(vtkVgNodeVisitorBase& nodeVisitor)
{
  nodeVisitor.Visit(*this);
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::Traverse(vtkVgNodeVisitorBase& nodeVisitor)
{
  this->Superclass::Traverse(nodeVisitor);

  this->TraverseChildren(nodeVisitor);

  // Calculate the bounds.
  if (this->GetBoundsDirty())
    {
    this->ComputeBounds();
    }

  // At the end of the traversal reset the dirty state.
  this->DirtyOff();
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::ComputeBounds()
{
  vtkVgComputeBounds::ComputeVisiblePropBounds(this);

  // If bounds are initialized or if this node does not have any children
  // then we won't be calculating the bounds as it will not lead to any
  // real bounds.
  if (vtkMath::AreBoundsInitialized(this->Bounds) || (this->GetNumberOfChildren() < 1))
    {
    this->BoundsDirtyOff();
    }
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::TraverseChildren(vtkVgNodeVisitorBase& nodeVisitor)
{
  int numberOfItems = this->Children->GetNumberOfItems();

  if (numberOfItems)
    {
    this->Children->InitTraversal();

    vtkVgNodeBase* node =
      static_cast<vtkVgNodeBase*>(this->Children->GetNextItemAsObject());
    while (node)
      {
      node->Accept(nodeVisitor);

      node = static_cast<vtkVgNodeBase*>(this->Children->GetNextItemAsObject());
      }
    }

  // Now remove the nodes.
  numberOfItems = this->RemoveChildren->GetNumberOfItems();
  if (numberOfItems)
    {
    this->RemoveChildren->InitTraversal();

    vtkVgNodeBase* node =
      static_cast<vtkVgNodeBase*>(this->RemoveChildren->GetNextItemAsObject());
    while (node)
      {
      // Traverse only if this node has remove node flag set to true.
      if (node->GetRemoveNode())
        {
        node->Accept(nodeVisitor);
        }

      node = static_cast<vtkVgNodeBase*>(this->RemoveChildren->GetNextItemAsObject());
      }

    this->RemoveChildren->RemoveAllItems();
    }
}

//-----------------------------------------------------------------------------
void vtkVgGroupNode::SetRemoveNode(int flag)
{
  if (this->RemoveNode != flag)
    {
    this->RemoveNode = flag;

    int numberOfItems = this->Children->GetNumberOfItems();
    if (numberOfItems)
      {
      this->Children->InitTraversal();

      for (int i = 0; i < numberOfItems; ++i)
        {
        vtkVgNodeBase* node =
          static_cast<vtkVgNodeBase*>(this->Children->GetNextItemAsObject());

        // Don't set the flag if the parent of this node is not this.
        if (node->GetParent() && node->GetParent() != this)
          {
          continue;
          }

        node->SetRemoveNode(flag);
        }
      }
    }
}
