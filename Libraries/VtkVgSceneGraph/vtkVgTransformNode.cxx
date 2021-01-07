// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgTransformNode.h"

// VisGUI includes.
#include "vtkVgNodeVisitorBase.h"

// VTK includes.
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

// C++ includes.
#include <sstream>

vtkStandardNewMacro(vtkVgTransformNode);

//-----------------------------------------------------------------------------
vtkVgTransformNode::vtkVgTransformNode() : vtkVgGroupNode()
{
  this->PreMultiply = false;
  this->Matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->Matrix->Identity();
}

//-----------------------------------------------------------------------------
vtkVgTransformNode::~vtkVgTransformNode()
{
}

//-----------------------------------------------------------------------------
void vtkVgTransformNode::PrintSelf(ostream& vtkNotUsed(os),
                                   vtkIndent vtkNotUsed(indent))
{
}

//-----------------------------------------------------------------------------
void vtkVgTransformNode::SetMatrix(vtkMatrix4x4* matrix, bool preMultiply)
{
  // \TODO: Visit this code again.
  if (matrix && matrix != this->Matrix)
    {
    this->Matrix = matrix;
    this->PreMultiply = preMultiply;
    this->DirtyOn();

    this->MakeAllChildrenDirty();
    }
}

//-----------------------------------------------------------------------------
vtkMatrix4x4* vtkVgTransformNode::GetMatrix()
{
  return this->Matrix;
}

//-----------------------------------------------------------------------------
const vtkMatrix4x4* vtkVgTransformNode::GetMatrix() const
{
  return this->Matrix;
}

//-----------------------------------------------------------------------------
void vtkVgTransformNode::Update(vtkVgNodeVisitorBase& vtkNotUsed(nodeVisitor))
{
  if (this->GetBoundsDirty())
    {
    this->ComputeBounds();
    }

  // If the reference frame is RELATIVE_REFERENCE and if this node has
  // a parent and if either its parent state or its state is dirty then
  // only calculate the \c FinalMatrix using its parent \c FinalMatrix.
  if (((this->NodeReferenceFrame == vtkVgNodeBase::RELATIVE_REFERENCE) &&
       this->Parent && this->Parent->GetDirty()) ||
      ((this->NodeReferenceFrame == vtkVgNodeBase::RELATIVE_REFERENCE) &&
       this->Parent && this->Dirty))
    {
    this->FinalMatrix->Identity();

    if (!this->PreMultiply)
      {
      vtkMatrix4x4::Multiply4x4(
        this->Parent->GetFinalMatrix(), this->Matrix, this->FinalMatrix);
      }
    else
      {
      vtkMatrix4x4::Multiply4x4(
        this->Matrix, this->Parent->GetFinalMatrix(), this->FinalMatrix);
      }
    }
  else if (this->Dirty)
    {
    this->FinalMatrix->DeepCopy(this->Matrix);
    }
}

//-----------------------------------------------------------------------------
void vtkVgTransformNode::Accept(vtkVgNodeVisitorBase& nodeVisitor)
{
  nodeVisitor.Visit(*this);
}

//-----------------------------------------------------------------------------
void vtkVgTransformNode::Traverse(vtkVgNodeVisitorBase& nodeVisitor)
{
  this->Superclass::Traverse(nodeVisitor);
}
