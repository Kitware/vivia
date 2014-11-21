/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgVideoNode.h"

#include "vtkVgEventRepresentationBase.h"
#include "vtkVgGroupNode.h"
#include "vtkVgSceneManager.h"
#include "vtkVgVideoRepresentationBase0.h"
#include "vtkVgVideoModel0.h"
#include "vtkVgPropCollection.h"
#include "vtkVgNodeBase.h"
#include "vtkVgNodeVisitorBase.h"
#include "vtkVgTimeStamp.h"
#include "vtkVgTrackRepresentationBase.h"
#include "vtkVgComputeBounds.h"

// VTK includes.
#include <vtkCommand.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkVgVideoNode);

//-----------------------------------------------------------------------------
vtkVgVideoNode::vtkVgVideoNode() : vtkVgLeafNodeBase(),
  StreamId(NULL),
  MissionId(NULL),
  InstanceId(-1),
  UserScore(0),
  RelevancyScore(0.0),
  PreferenceScore(0.0),
  ColorScalar(0.0),
  Rank(-1),
  IsNormalResult(false),
  IsRefinementResult(false),
  IsStarResult(false),
  HasVideoData(false),
  ActivityType(-1),
  Note(NULL),
  VideoModel(NULL),
  VideoRepresentation(NULL)
{
  this->TimeRange[0] = this->TimeRange[1] = -1.0;
}

//-----------------------------------------------------------------------------
vtkVgVideoNode::~vtkVgVideoNode()
{
  this->SetStreamId(0);
  this->SetMissionId(0);
  this->SetNote(0);
}

//-----------------------------------------------------------------------------
void vtkVgVideoNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgVideoNode::SetVideoRepresentation(vtkVgVideoRepresentationBase0* videoRepresentation)
{
  if (videoRepresentation && (videoRepresentation != this->VideoRepresentation))
    {
    this->VideoRepresentation = videoRepresentation;
    this->VideoModel = this->VideoRepresentation->GetVideoModel();

    if (!this->VideoModel)
      {
      vtkErrorMacro("VideoModel is not set. Expect unusual behavior.")
      }

    this->BoundsDirtyOn();

    // \NOTE: This flag is on or else there won't be any data transfer to
    // video representation in \c Update call.
    this->DirtyOn();
    }
}

//-----------------------------------------------------------------------------
vtkVgVideoRepresentationBase0* vtkVgVideoNode::GetVideoRepresentation()
{
  return this->VideoRepresentation;
}

//-----------------------------------------------------------------------------
const vtkVgVideoRepresentationBase0* vtkVgVideoNode::GetVideoRepresentation() const
{
  return this->VideoRepresentation;
}

//-----------------------------------------------------------------------------
int vtkVgVideoNode::SetVisible(int flag)
{
  // Call the base class function first so that ivar is
  // set to correct value.
  if (!vtkVgNodeBase::SetVisible(flag))
    {
    return 0;
    }

  vtkVgEventRepresentationBase* eventRep =
    this->VideoRepresentation->GetEventRepresentation();
  vtkVgTrackRepresentationBase* trackRep =
    this->VideoRepresentation->GetTrackRepresentation();
  int eventRepVisibilityBefore = eventRep ? eventRep->GetVisible() : 0;
  int trackRepVisibilityBefore = trackRep ? trackRep->GetVisible() : 0;

  this->VideoRepresentation->SetVisible(flag);

  if (flag)
    {
    if (eventRep && !eventRepVisibilityBefore)
      {
      eventRep->Update();
      }
    if (trackRep && !trackRepVisibilityBefore)
      {
      trackRep->Update();
      }
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgVideoNode::Update(vtkVgNodeVisitorBase& nodeVisitor)
{
  // Update the model first.
  this->VideoModel->Update(nodeVisitor.GetTimeStamp());

  // First update the render objects.
  this->UpdateRenderObjects(nodeVisitor.GetPropCollection());

  if (((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE) && this->Parent && this->Parent->GetDirty()) ||
      ((this->NodeReferenceFrame ==
        vtkVgNodeBase::RELATIVE) && this->Parent && this->Dirty))
    {
    // We might need a concatenation instead of apply transformation here.
    this->FinalMatrix->DeepCopy(this->Parent->GetFinalMatrix());

    this->VideoRepresentation->SetRepresentationMatrix(this->FinalMatrix);
    this->VideoRepresentation->Update();

    this->BoundsDirtyOn();
    }

  // Check if bounds are dirty.If dirty compute bounds.
  if (this->BoundsDirty)
    {
    this->ComputeBounds();
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoNode::UpdateRenderObjects(vtkVgPropCollection* propCollection)
{
  if (this->GetRemoveNode())
    {
    this->PrepareForRemoval(propCollection, this->VideoRepresentation);

    if (vtkVgRepresentationBase* trackRep =
          this->VideoRepresentation->GetTrackRepresentation())
      {
      this->PrepareForRemoval(propCollection, trackRep);
      }

    if (vtkVgRepresentationBase* eventRep =
          this->VideoRepresentation->GetEventRepresentation())
      {
      this->PrepareForRemoval(propCollection, eventRep);
      }
    this->RemoveNodeOff();
    } // if (this->GetRemoveNode())
  else
    {
    this->PrepareForAddition(propCollection, this->VideoRepresentation);

    if (vtkVgRepresentationBase* trackRep =
          this->VideoRepresentation->GetTrackRepresentation())
      {
      this->PrepareForAddition(propCollection, trackRep);
      }

    if (vtkVgRepresentationBase* eventRep =
          this->VideoRepresentation->GetEventRepresentation())
      {
      this->PrepareForAddition(propCollection, eventRep);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoNode::Accept(vtkVgNodeVisitorBase& nodeVisitor)
{
  nodeVisitor.Visit(*this);
}

//-----------------------------------------------------------------------------
void vtkVgVideoNode::Traverse(vtkVgNodeVisitorBase& nodeVisitor)
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

//-----------------------------------------------------------------------------
void vtkVgVideoNode::ComputeBounds()
{
  vtkVgComputeBounds::ComputeVisiblePropBounds(
    this->VideoRepresentation->GetActiveRenderObjects(), this->Bounds);

  if (vtkMath::AreBoundsInitialized(this->Bounds) ||
      this->VideoRepresentation->GetActiveRenderObjects()->GetNumberOfItems() < 1)
    {
    this->BoundsDirtyOff();
    }
}
