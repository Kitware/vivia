// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgNodeBase.h"
#include "vtkVgGroupNode.h"
#include "vtkVgNodeVisitorBase.h"

// VTK includes.
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVgNodeBase)

//-----------------------------------------------------------------------------
vtkVgNodeBase::vtkVgNodeBase() : vtkObject(),
  Name(0),
  Dirty(1),
  BoundsDirty(1),
  Visible(1),
  RemoveNode(0),
  FinalMatrix(NULL),
  Parent(NULL),
  NodeReferenceFrame
  (RELATIVE_REFERENCE),
  VisibleNodeMask
  (VISIBLE_NODE_MASK)
{
  this->SetName("");
  this->FinalMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  // Set the bounds to uninitialized state.
  vtkMath::UninitializeBounds(this->Bounds);
}

//-----------------------------------------------------------------------------
vtkVgNodeBase::~vtkVgNodeBase()
{
  this->SetName(NULL);
}

//-----------------------------------------------------------------------------
void vtkVgNodeBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Name: " << ((Name == NULL) ? "" : this->Name) << "\n";
  os << indent << "Visible: " << (this->Visible ? "True" : "False") << "\n";
  os << indent << "Dirty: " << (this->Dirty ? "True" : "False") << "\n";
}

//-----------------------------------------------------------------------------
const char* vtkVgNodeBase::GetName() const
{
  return this->Name;
}

//-----------------------------------------------------------------------------
int vtkVgNodeBase::SetVisible(int flag)
{
  if (this->Visible != flag &&
      (this->GetVisibleNodeMask() & VISIBLE_NODE_MASK))
    {
    this->Visible = flag;
    return 1;
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vtkVgNodeBase::SetBoundsDirty(int flag)
{
  if (this->BoundsDirty != flag)
    {
    this->BoundsDirty = flag;
    if (this->Parent && flag)
      {
      this->Parent->BoundsDirtyOn();
      }

    // \NOTE: Do We need to set this on?
//    this->DirtyOn();
    }
}

//-----------------------------------------------------------------------------
void vtkVgNodeBase::BoundsDirtyOn()
{
  this->SetBoundsDirty(static_cast<int>(1));
}

//-----------------------------------------------------------------------------
void vtkVgNodeBase::BoundsDirtyOff()
{
  this->SetBoundsDirty(static_cast<int>(0));
}

//-----------------------------------------------------------------------------
const vtkMatrix4x4* vtkVgNodeBase::GetFinalMatrix() const
{
  return this->FinalMatrix;
}

//-----------------------------------------------------------------------------
vtkMatrix4x4* vtkVgNodeBase::GetFinalMatrix()
{
  return this->FinalMatrix;
}

//-----------------------------------------------------------------------------
void vtkVgNodeBase::SetParent(vtkVgGroupNode* parent)
{
  if (this->Parent != parent)
    {
    this->Parent = parent;
    this->DirtyOn();
    }
}

//-----------------------------------------------------------------------------
vtkVgGroupNode* vtkVgNodeBase::GetParent()
{
  return this->Parent;
}

//-----------------------------------------------------------------------------
const vtkVgGroupNode* vtkVgNodeBase::GetParent() const
{
  return this->Parent;
}

//-----------------------------------------------------------------------------
void vtkVgNodeBase::GetBounds(double bounds[6])
{
  for (int i = 0; i < 6; ++i)
    {
    bounds[i] = this->Bounds[i];
    }
}

//-----------------------------------------------------------------------------
const double* vtkVgNodeBase::GetBounds() const
{
  return this->Bounds;
}

//-----------------------------------------------------------------------------
double* vtkVgNodeBase::GetBounds()
{
  return this->Bounds;
}

//-----------------------------------------------------------------------------
void vtkVgNodeBase::Update(vtkVgNodeVisitorBase& vtkNotUsed(nodeVisitor))
{
  // Do nothing.
}

//-----------------------------------------------------------------------------
void vtkVgNodeBase::Traverse(vtkVgNodeVisitorBase&  nodeVisitor)
{
  if (nodeVisitor.GetVisitorType() == vtkVgNodeVisitorBase::UPDATE_VISITOR)
    {
    this->Update(nodeVisitor);
    }

  // \NOTE: Don't call DirtyOff here. As that is the responsibility of the derived class.
}
