/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTrackNode.h"

// VisGUI includes.
#include "vtkVgComputeBounds.h"
#include "vtkVgGroupNode.h"
#include "vtkVgNodeVisitorBase.h"
#include "vtkVgTimeStamp.h"

#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>

// VTK includes.
#include <vtkCommand.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

// C/C++ includes
#include <cassert>


vtkStandardNewMacro(vtkVgTrackNode);


//-----------------------------------------------------------------------------
vtkVgTrackNode::vtkVgTrackNode() : vtkVgLeafNodeBase(),
  TrackModel(0x0),
  TrackRepresentation(0x0)
{
}

//-----------------------------------------------------------------------------
vtkVgTrackNode::~vtkVgTrackNode()
{
}

//-----------------------------------------------------------------------------
void vtkVgTrackNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgTrackNode::SetTrackModel(vtkVgTrackModel* trackModel)
{
  if (trackModel && (trackModel != this->TrackModel))
    {
    this->TrackModel = trackModel;

    this->BoundsDirtyOn();

    // \NOTE: This flag is on or else there won't be any data transfer to
    // video representation in \c Update call.
    this->DirtyOn();
    }
}

//-----------------------------------------------------------------------------
const vtkVgTrackModel* vtkVgTrackNode::GetTrackModel() const
{
  return this->TrackModel;
}

//-----------------------------------------------------------------------------
vtkVgTrackModel* vtkVgTrackNode::GetTrackModel()
{
  return this->TrackModel;
}

//-----------------------------------------------------------------------------
void vtkVgTrackNode::SetTrackRepresentation(vtkVgTrackRepresentation* trackRepresentation)
{
  if (trackRepresentation && (trackRepresentation != this->TrackRepresentation))
    {
    this->TrackRepresentation = trackRepresentation;

    this->BoundsDirtyOn();

    // \NOTE: This flag is on or else there won't be any data transfer to
    // video representation in \c Update call.
    this->DirtyOn();
    }
}

//-----------------------------------------------------------------------------
const vtkVgTrackRepresentation* vtkVgTrackNode::GetTrackRepresentation() const
{
  return this->TrackRepresentation;
}

//-----------------------------------------------------------------------------
vtkVgTrackRepresentation* vtkVgTrackNode::GetTrackRepresentation()
{
  return this->TrackRepresentation;
}

//-----------------------------------------------------------------------------
void vtkVgTrackNode::ComputeBounds()
{
  assert(this->TrackRepresentation);

  vtkVgComputeBounds::ComputeVisiblePropBounds(
    this->TrackRepresentation->GetActiveRenderObjects(), this->Bounds);

  if (vtkMath::AreBoundsInitialized(this->Bounds) ||
      this->TrackRepresentation->GetActiveRenderObjects()->GetNumberOfItems() < 1)
    {
    this->BoundsDirtyOff();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackNode::Update(vtkVgNodeVisitorBase& nodeVisitor)
{
  assert(this->TrackModel);
  assert(this->TrackRepresentation);

  // Update the model first.
  this->TrackModel->Update(nodeVisitor.GetTimeStamp());

  if (this->TrackModel->GetNumberOfTracks() > 0)
    {
    this->TrackRepresentation->Update();
    }

  // First update the render objects.
  this->UpdateRenderObjects(nodeVisitor.GetPropCollection());

  if (((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE_REFERENCE) && this->Parent && this->Parent->GetDirty()) ||
      ((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE_REFERENCE) && this->Parent && this->Dirty))
    {
    // We might need a concatenation instead of apply transformation here.
    this->FinalMatrix->DeepCopy(this->Parent->GetFinalMatrix());

    this->TrackRepresentation->SetRepresentationMatrix(this->FinalMatrix);
    this->TrackRepresentation->Update();

    this->BoundsDirtyOn();
    }

  // Check if bounds are dirty.If dirty compute bounds.
  if (this->BoundsDirty)
    {
    this->ComputeBounds();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackNode::UpdateRenderObjects(vtkVgPropCollection* propCollection)
{
  assert(this->TrackModel);
  assert(this->TrackRepresentation);

  if (this->GetRemoveNode())
    {
    if (this->TrackRepresentation)
      {
      this->PrepareForRemoval(propCollection, this->TrackRepresentation);
      }

    this->RemoveNodeOff();
    } // if (this->GetRemoveNode())
  else
    {
    if (this->TrackRepresentation)
      {
      this->PrepareForAddition(propCollection, this->TrackRepresentation);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackNode::Accept(vtkVgNodeVisitorBase& nodeVisitor)
{
  nodeVisitor.Visit(static_cast<vtkVgLeafNodeBase&>(*this));
}

//-----------------------------------------------------------------------------
void vtkVgTrackNode::Traverse(vtkVgNodeVisitorBase& nodeVisitor)
{
  // Then call the base class which will call Update on this.
  this->Superclass::Traverse(nodeVisitor);

  // Check if bounds are dirty.If dirty compute bounds.
  if (this->BoundsDirty)
    {
    this->ComputeBounds();
    }

  this->DirtyOff();
}

