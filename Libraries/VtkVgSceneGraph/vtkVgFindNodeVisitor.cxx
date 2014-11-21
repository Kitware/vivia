/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// VG includes.
#include "vtkVgFindNodeVisitor.h"
#include "vtkVgVideoNode.h"
#include "vtkVgGeode.h"
#include "vtkVgVideoRepresentationBase0.h"

// VTK includes.
#include <vtkPropCollection.h>
#include <vtkProp3D.h>

//-----------------------------------------------------------------------------
vtkVgFindNodeVisitor::vtkVgFindNodeVisitor() : vtkVgNodeVisitor(),
  UsingProp3D(NULL),
  Node(NULL)
{
}

//-----------------------------------------------------------------------------
vtkVgFindNodeVisitor::~vtkVgFindNodeVisitor()
{
  this->SetVisitorType(vtkVgNodeVisitorBase::NODE_VISITOR);
}

//-----------------------------------------------------------------------------
void vtkVgFindNodeVisitor::SetUsingProp3D(vtkProp3D* prop3D)
{
  if (prop3D && (this->UsingProp3D != prop3D))
    {
    this->UsingProp3D = prop3D;

    this->ModifiedTimeStamp.Modified();
    }
}

//-----------------------------------------------------------------------------
vtkProp3D* vtkVgFindNodeVisitor::GetUsingProp3D()
{
  return this->UsingProp3D;
}

//-----------------------------------------------------------------------------
const vtkProp3D* vtkVgFindNodeVisitor::GetUsingProp3D() const
{
  return this->UsingProp3D;
}

//-----------------------------------------------------------------------------
vtkVgNodeBase* vtkVgFindNodeVisitor::GetNode()
{
  return this->Node;
}

//-----------------------------------------------------------------------------
void vtkVgFindNodeVisitor::Visit(vtkVgVideoNode& videoNode)
{
  if (!this->UsingProp3D)
    {
    return;
    }

  vtkVgNodeVisitorBase::Visit(videoNode);

  vtkVgVideoRepresentationBase0* videoRep = videoNode.GetVideoRepresentation();
  vtkPropCollection* propCollection = videoRep->GetActiveRenderObjects();
  if (propCollection)
    {
    propCollection->InitTraversal();

    int numberOfItems = propCollection->GetNumberOfItems();

    for (int i = 0; i < numberOfItems; ++i)
      {
      if (vtkProp3D::SafeDownCast(propCollection->GetNextProp()) ==
          this->UsingProp3D)
        {
        this->Node = static_cast<vtkVgNodeBase*>(&videoNode);
        break;
        }
      }
    }

  this->Traverse(videoNode);
}

//-----------------------------------------------------------------------------
void vtkVgFindNodeVisitor::Visit(vtkVgGeode& geode)
{
  if (!this->UsingProp3D)
    {
    return;
    }

  vtkVgNodeVisitorBase::Visit(geode);

  vtkPropCollection* propCollection = geode.GetActiveDrawables();
  if (propCollection)
    {
    propCollection->InitTraversal();

    int numberOfItems = propCollection->GetNumberOfItems();

    for (int i = 0; i < numberOfItems; ++i)
      {
      // Put a loop in here.
      if (vtkProp3D::SafeDownCast(propCollection->GetNextProp()) ==
          this->UsingProp3D)
        {
        this->Node = static_cast<vtkVgNodeBase*>(&geode);
        break;
        }
      }
    }

  this->Traverse(geode);
}

//-----------------------------------------------------------------------------
void vtkVgFindNodeVisitor::Visit(vtkVgGroupNode& groupNode)
{
  vtkVgNodeVisitor::Visit(groupNode);
}

//-----------------------------------------------------------------------------
void vtkVgFindNodeVisitor::Visit(vtkVgTransformNode& transformNode)
{
  vtkVgNodeVisitor::Visit(transformNode);
}

//-----------------------------------------------------------------------------
void vtkVgFindNodeVisitor::Visit(vtkVgLeafNodeBase& leafNode)
{
  vtkVgNodeVisitor::Visit(leafNode);
}

