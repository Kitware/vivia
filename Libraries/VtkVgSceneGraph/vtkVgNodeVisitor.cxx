/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
