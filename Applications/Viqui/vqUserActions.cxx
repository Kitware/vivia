// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqUserActions.h"

#include "vqCore.h"

// VG includes.
#include "vtkVgGroupNode.h"
#include "vtkVgSceneManager.h"
#include "vtkVgSpaceConversion.h"
#include "vtkVgVideoModel0.h"
#include "vtkVgVideoNode.h"
#include "vtkVgVideoRepresentation0.h"
#include "vtkVgViewerBase.h"

#include <vtkCamera.h>
#include <vtkRenderer.h>

#include <QList>

//-----------------------------------------------------------------------------
vqUserActions::vqUserActions(vqCore* core)
  : QObject(),
    CoreInstance(core),
    LastActiveVideoNode(0),
    LastZoomedNode(0),
    PreZoomCamera(0),
    IsZoomed(false),
    AutoZoom(true)
{
  this->PreZoomCamera = vtkCamera::New();
}

//-----------------------------------------------------------------------------
vqUserActions::~vqUserActions()
{
  this->PreZoomCamera->Delete();
}

//-----------------------------------------------------------------------------
void vqUserActions::TogglePlayPause(vtkVgVideoNode& node)
{
  vtkVgVideoRepresentationBase0* videoRepresentation =
    node.GetVideoRepresentation();
  if (!videoRepresentation)
    {
    return;
    }

  // toggle between playing and paused
  if (vtkVgVideoModel0* videoModel = videoRepresentation->GetVideoModel())
    {
    if (videoModel->IsPaused())
      {
      videoModel->Play();
      emit this->Play(node);
      }
    else if (videoModel->IsPlaying())
      {
      videoModel->Pause();
      emit this->Pause(node);
      }
    }
}

//-----------------------------------------------------------------------------
void vqUserActions::DeselectRecursive(vtkVgNodeBase* root)
{
  if (vtkVgVideoNode* videoNode = dynamic_cast<vtkVgVideoNode*>(root))
    {
    if (vtkVgVideoRepresentation0* rep =
          dynamic_cast<vtkVgVideoRepresentation0*>(
            videoNode->GetVideoRepresentation()))
      {
      rep->SetSelectionVisible(0);
      rep->Update();
      }
    return;
    }

  if (vtkVgGroupNode* groupNode = dynamic_cast<vtkVgGroupNode*>(root))
    {
    int numChildren = groupNode->GetNumberOfChildren();
    for (int i = 0; i < numChildren; ++i)
      {
      if (vtkVgNodeBase* child = groupNode->GetChild(i))
        {
        this->DeselectRecursive(child);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vqUserActions::Select(QList<vtkVgNodeBase*> nodes)
{
  // remember the selection state of the active video
  bool activeVideoAlreadySelected = false;
  if (this->LastActiveVideoNode)
    {
    if (vtkVgVideoRepresentation0* rep =
          dynamic_cast<vtkVgVideoRepresentation0*>(
            this->LastActiveVideoNode->GetVideoRepresentation()))
      {
      activeVideoAlreadySelected = rep->GetSelectionVisible() != 0;
      }
    }

  // deselect all nodes in the graph
  this->DeselectRecursive(
    this->CoreInstance->getContextViewer()->GetSceneRoot());

  // highlight new selections
  foreach (vtkVgNodeBase* node, nodes)
    {
    vtkVgVideoNode* videoNode = dynamic_cast<vtkVgVideoNode*>(node);
    if (videoNode && videoNode->GetHasVideoData())
      {
      if (vtkVgVideoRepresentation0* rep =
            dynamic_cast<vtkVgVideoRepresentation0*>(
              videoNode->GetVideoRepresentation()))
        {
        rep->SetSelectionVisible(1);
        rep->Update();
        }

      // toggle play state if this video is selected and currently active
      if (activeVideoAlreadySelected &&
          videoNode == this->LastActiveVideoNode)
        {
        this->TogglePlayPause(*videoNode);
        }
      }
    }

  this->CoreInstance->postRender();
}

//-----------------------------------------------------------------------------
void vqUserActions::Activate(vtkVgNodeBase& node)
{
  vtkVgVideoNode* videoNode = dynamic_cast<vtkVgVideoNode*>(&node);
  if (!videoNode || !videoNode->GetHasVideoData())
    {
    return;
    }

  // First get the representation.
  vtkVgVideoRepresentationBase0* videoRepresentation =
    videoNode->GetVideoRepresentation();
  if (videoRepresentation)
    {
    if (vtkVgVideoModel0* videoModel = videoRepresentation->GetVideoModel())
      {
      // Play only when video is stopped or its in neutral state.
      if (videoModel->IsStopped() ||
          (!videoModel->IsPlaying() && !videoModel->IsPaused()))
        {
        videoModel->Play();
        emit this->Play(*videoNode);
        }
      else
        {
        if (!videoModel->IsStopped())
          {
          videoModel->Stop();
          emit this->Stop(*videoNode);
          }
        }
      }

    // Stop previously playing video on a different node.
    if (this->LastActiveVideoNode && this->LastActiveVideoNode != videoNode)
      {
      vtkVgVideoRepresentationBase0* videoRepresentation =
        this->LastActiveVideoNode->GetVideoRepresentation();

      vtkVgVideoModel0* videoModel = videoRepresentation->GetVideoModel();

      if (!videoModel->IsStopped())
        {
        videoModel->Stop();
        }
      }
    }

  this->LastActiveVideoNode = videoNode;
}

//-----------------------------------------------------------------------------
void vqUserActions::CalculateBoundsAndCenter(vtkVgNodeBase& node,
                                             double bounds[6],
                                             double center[6])
{
  if (!bounds || !center)
    {
    // Do nothing if null.
    return;
    }

  // Make sure bounds are up to date.
  node.ComputeBounds();
  node.GetBounds(bounds);

  double width =  bounds[1] - bounds[0];
  double height = bounds[3] - bounds[2];

  // pad out the bounds a small amount
  double padScale = 0.1;
  bounds[0] -= width * padScale;
  bounds[1] += width * padScale;
  bounds[2] -= height * padScale;
  bounds[3] += height * padScale;

  // center the extents
  center[0] = 0.5 * (bounds[0] + bounds[1]);
  center[1] = 0.5 * (bounds[2] + bounds[3]);
}

//-----------------------------------------------------------------------------
bool vqUserActions::CheckIfFullyContained2D(double nodeBounds[6],
                                            double worldBounds[6])
{
  // Ignoring Z dimension
  if ((nodeBounds[0] < worldBounds[0]) || (nodeBounds[1] > worldBounds[1])
      || (nodeBounds[2] < worldBounds[2]) || (nodeBounds[3] > worldBounds[3]))
    {
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vqUserActions::CheckIfFullyContained(
  double nodeBounds[6], vtkRenderer* renderer)
{
  double topLeft[3]      = { -1.0,  1.0, 0.0 };
  double bottomRight[3]  = { 1.0, -1.0, 0.0 };

  // View to world conversion
  vtkVgSpaceConversion::ViewToWorldNormalized
  (renderer, topLeft, topLeft);

  vtkVgSpaceConversion::ViewToWorldNormalized(
    renderer, bottomRight, bottomRight);

  double worldBounds[6] =
    {
    topLeft[0], bottomRight[0],
    bottomRight[1], topLeft[1],
    0.0, 0.0
    };

  return this->CheckIfFullyContained2D(nodeBounds, worldBounds);
}

//-----------------------------------------------------------------------------
void vqUserActions::Reset()
{
  this->LastActiveVideoNode = 0;
  this->LastZoomedNode  = 0;
  this->UnZoom();
}

//-----------------------------------------------------------------------------
void vqUserActions::UnZoom()
{
  if (this->IsZoomed)
    {
    this->CoreInstance->getContextViewer()->GetSceneRenderer()
    ->GetActiveCamera()->DeepCopy(this->PreZoomCamera);
    this->IsZoomed = false;
    this->LastZoomedNode = 0;

    emit this->UpdateView();
    }
}

//-----------------------------------------------------------------------------
void vqUserActions::FocusTo(vtkVgVideoNode& node)
{
  this->FocusTo(*static_cast<vtkVgNodeBase*>(&node));
}

//-----------------------------------------------------------------------------
void vqUserActions::FocusTo(vtkVgNodeBase& node)
{
  double center[2];
  double nodeBounds[6];
  double lastFocalPt[3];
  double lastPos[3];

  vtkVgVideoNode* videoNode = vtkVgVideoNode::SafeDownCast(&node);
  if (videoNode)
    {
    vtkVgVideoRepresentation0* videoRep =
      vtkVgVideoRepresentation0::SafeDownCast(
        videoNode->GetVideoRepresentation());
    if (videoRep && videoRep->GetModelError())
      {
      return;
      }
    }
  this->CalculateBoundsAndCenter(node, nodeBounds, center);

  vtkRenderer* renderer =
    this->CoreInstance->getContextViewer()->GetSceneRenderer();

  // Do not center / focus if the node is completely contained
  // within the view area
  if (this->CheckIfFullyContained(nodeBounds, renderer))
    {
    return;
    }

  vtkCamera* camera = renderer->GetActiveCamera();

  camera->GetFocalPoint(lastFocalPt);
  camera->GetPosition(lastPos);

  camera->SetFocalPoint(center[0], center[1], lastFocalPt[2]);
  camera->SetPosition(center[0], center[1], lastPos[2]);

  emit this->UpdateView();
}

//-----------------------------------------------------------------------------
void vqUserActions::CenterAndZoomTo(vtkVgNodeBase& node, bool alwaysZoom)
{
  // Just set view center to node center when auto zoom
  // is disabled
  if (!alwaysZoom && !this->AutoZoom)
    {
    return this->FocusTo(node);
    }

  // Do not zoom if the incoming node is same as the last time
  // or if it is not visible currently.
  if (this->LastZoomedNode == &node || !node.GetVisible())
    {
    return;
    }
  this->LastZoomedNode = &node;

  double center[2];
  double itemBounds[6];
  vtkRenderer* ren = this->CoreInstance->getContextViewer()->GetSceneRenderer();

  // Get the calculated bounds and center.
  this->CalculateBoundsAndCenter(node, itemBounds, center);

  vtkCamera* camera = ren->GetActiveCamera();

  // save pre-zoom camera setup
  if (!this->IsZoomed)
    {
    this->PreZoomCamera->DeepCopy(camera);
    this->IsZoomed = true;
    }

  double lastFocalPt[3], lastPos[3];
  camera->GetFocalPoint(lastFocalPt);
  camera->GetPosition(lastPos);
  camera->SetFocalPoint(center[0], center[1], lastFocalPt[2]);
  camera->SetPosition(center[0], center[1], lastPos[2]);

  // compute the bounds of the item in display coordinates
  double itemMin[] = { itemBounds[0], itemBounds[2], 0.0, 1.0 };
  double itemDisplayMin[4];
  ren->SetWorldPoint(itemMin);
  ren->WorldToDisplay();
  ren->GetDisplayPoint(itemDisplayMin);

  double itemMax[] = { itemBounds[1], itemBounds[3], 0.0, 1.0 };
  double itemDisplayMax[4];
  ren->SetWorldPoint(itemMax);
  ren->WorldToDisplay();
  ren->GetDisplayPoint(itemDisplayMax);

  double width =  itemDisplayMax[0] - itemDisplayMin[0];
  double height = itemDisplayMax[1] - itemDisplayMin[1];

  // zoom to make the item fill the window
  double xZoom = ren->GetSize()[0] / width;
  double yZoom = ren->GetSize()[1] / height;
  camera->Zoom(std::min(xZoom, yZoom));

  emit this->UpdateView();
}

//-----------------------------------------------------------------------------
void vqUserActions::CenterAndZoomTo(vtkVgVideoNode& videoNode, bool alwaysZoom)
{
  this->CenterAndZoomTo(*static_cast<vtkVgNodeBase*>(&videoNode), alwaysZoom);
}

//-----------------------------------------------------------------------------
void vqUserActions::ScrollForward(vtkVgNodeBase& node, bool* eventHandled)
{
  // go to next frame if scrolling forward on a paused and selected video
  if (&node == this->LastActiveVideoNode)
    {
    vtkVgVideoRepresentation0* rep =
      dynamic_cast<vtkVgVideoRepresentation0*>(
        this->LastActiveVideoNode->GetVideoRepresentation());

    if (rep && rep->GetSelectionVisible() && rep->GetVideoModel()->IsPaused())
      {
      emit this->Next(*this->LastActiveVideoNode);
      *eventHandled = true;
      }
    }
}

//-----------------------------------------------------------------------------
void vqUserActions::ScrollBackward(vtkVgNodeBase& node, bool* eventHandled)
{
  // go to previous frame if scrolling backward on a paused and selected video
  if (&node == this->LastActiveVideoNode)
    {
    vtkVgVideoRepresentation0* rep =
      dynamic_cast<vtkVgVideoRepresentation0*>(
        this->LastActiveVideoNode->GetVideoRepresentation());

    if (rep && rep->GetSelectionVisible() && rep->GetVideoModel()->IsPaused())
      {
      emit this->Prev(*this->LastActiveVideoNode);
      *eventHandled = true;
      }
    }
}

//-----------------------------------------------------------------------------
void vqUserActions::SetAutoZoom(bool enable)
{
  this->AutoZoom = enable;
}

//-----------------------------------------------------------------------------
bool vqUserActions::GetAutoZoom() const
{
  return this->AutoZoom;
}
