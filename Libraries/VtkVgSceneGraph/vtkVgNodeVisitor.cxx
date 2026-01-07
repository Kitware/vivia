// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgNodeVisitor.h"

#include "vtkVgGeode.h"
#include "vtkVgGroupNode.h"
#include "vtkVgTransformNode.h"
#include "vtkVgVideoNode.h"
#include "vtkVgPropCollection.h"

//-----------------------------------------------------------------------------
vtkVgNodeVisitor::vtkVgNodeVisitor() : vtkVgNodeVisitorBase()
{
}

//-----------------------------------------------------------------------------
vtkVgNodeVisitor::~vtkVgNodeVisitor()
{
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitor::Visit(vtkVgGeode& geode)
{
  this->Traverse(geode);
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitor::Visit(vtkVgGroupNode& groupNode)
{
  this->Traverse(groupNode);
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitor::Visit(vtkVgTransformNode& transformNode)
{
  this->Traverse(transformNode);
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitor::Visit(vtkVgVideoNode& videoNode)
{
  this->Traverse(videoNode);
}

//-----------------------------------------------------------------------------
void vtkVgNodeVisitor::Visit(vtkVgLeafNodeBase& leafNode)
{
  this->Traverse(leafNode);
}
