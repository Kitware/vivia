/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTransformNode.h"
#include "vtkVgNodeVisitorBase.h"

// VTK includes.
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

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

  // \NOTE:
  // If the reference frame is RELATIVE and if this node has a parent and if either
  // its parent state or its state is dirty then only calculate the \c FinalMatrix
  // using its parent \c FinalMatrix.
  if (((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE) && this->Parent && this->Parent->GetDirty()) ||
      ((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE) && this->Parent && this->Dirty))
    {
    this->FinalMatrix->Identity();

    if (!this->PreMultiply)
      {
      vtkMatrix4x4::Multiply4x4(this->Parent->GetFinalMatrix(), this->Matrix,
                                this->FinalMatrix);
      }
    else
      {
      vtkMatrix4x4::Multiply4x4(this->Matrix, this->Parent->GetFinalMatrix(),
                                this->FinalMatrix);
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
