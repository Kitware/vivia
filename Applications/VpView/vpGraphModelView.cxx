// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpGraphModelView.h"

#include "vpGraphModelRenderCallback.h"
#include "vpMultiGraphModel.h"
#include "vpMultiGraphRepresentation.h"

#include <qtStlUtil.h>

#include <vtkVgInteractorStyleRubberBand2D.h>
#include <vtkVgSpaceConversion.h>

#include <vtkVgQtUtil.h>

#include <vtkCellArray.h>
#include <vtkMatrix3x3.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

//-----------------------------------------------------------------------------
vpGraphModelView::vpGraphModelView() :
  QObject(),
  GraphRepresentation(vtkSmartPointer<vpMultiGraphRepresentation>::New()),
  RenderCallback(vtkSmartPointer<vpGraphModelRenderCallback>::New()),
  LineActor(vtkSmartPointer<vtkActor>::New()),
  LinePoints(vtkSmartPointer<vtkPoints>::New()),
  HeldNode(-1), ConnectParentNode(-1), CurrentSelectionType(ST_None),
  CreateNodeEnabled(false), RenderPending(false), DragStarted(false),
  GraphRepNeedsUpdate(false), PrevRubberBandMode(-1)
{
  this->GraphRepresentation->UseAutoUpdateOff();

  this->RenderCallback->SetView(this);

  this->LinePoints->SetNumberOfPoints(2);

  vtkPolyData* pd = vtkPolyData::New();
  vtkCellArray* ca = vtkCellArray::New();
  vtkPolyDataMapper* pdm = vtkPolyDataMapper::New();

  vtkIdType ids[] = { 0, 1 };
  ca->InsertNextCell(2, ids);
  pd->SetPoints(this->LinePoints);
  pd->SetLines(ca);
  pdm->SetInputData(pd);

  this->LineActor->SetMapper(pdm);
  pd->FastDelete();
  ca->FastDelete();
  pdm->FastDelete();

  this->LineActor->GetProperty()->SetLineWidth(3.0f);
  this->LineActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
  this->LineActor->VisibilityOff();
  this->LineActor->PickableOff();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::initialize(vpMultiGraphModel* model,
                                  vtkRenderer* renderer,
                                  vtkVgInteractorStyleRubberBand2D* istyle)
{
  this->GraphModel = model;
  this->GraphRepresentation->SetGraphModel(model);
  this->GraphRepresentation->SetVisible(0);
  this->Renderer = renderer;
  this->RenderWindow = this->Renderer->GetRenderWindow();
  this->InteractorStyle = istyle;

  this->Renderer->AddViewProp(this->LineActor);

  // Instead of auto-updating the graph representation every time the model is
  // changed, we listen for the update command and use it to set a flag which
  // will lazily update the graph rep only after a render is requested.
  vtkConnect(
    this->GraphModel,
    vtkCommand::UpdateDataEvent,
    this, SLOT(setGraphRepNeedsUpdate()));

  this->GraphRepNeedsUpdate = true;
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setEventRegistry(vtkVgEventTypeRegistry* reg)
{
  this->GraphRepresentation->SetEventRegistry(reg);
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setGraphLayout(int layout,
                                      const vgRange<double>& xExtents,
                                      const vgRange<double>& yExtents)
{
  this->GraphRepresentation->SetLayoutMode(layout, this->Renderer,
                                           xExtents, yExtents);
}

//-----------------------------------------------------------------------------
int vpGraphModelView::getGraphLayout()
{
  return this->GraphRepresentation->GetLayoutMode();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::copyCurrentLayout()
{
  this->GraphRepresentation->ResetCachedNodePositions();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::refreshGraphLayout()
{
  this->GraphRepresentation->RefreshLayout();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::getGraphBounds(vgRange<double>& xExtents,
                                      vgRange<double>& yExtents)
{
  this->update();

  double bounds[6];
  this->Renderer->ComputeVisiblePropBounds(bounds);
  xExtents.lower = bounds[0];
  xExtents.upper = bounds[1];
  yExtents.lower = bounds[2];
  yExtents.upper = bounds[3];
}

//-----------------------------------------------------------------------------
void vpGraphModelView::getWorldPosition(double xDisplay, double yDisplay,
                                        double& xWorld, double& yWorld)
{
  double pt[3] = { xDisplay, yDisplay, 0.0 };
  vtkVgSpaceConversion::DisplayToWorldNormalized(this->Renderer, pt, pt);
  xWorld = pt[0];
  yWorld = pt[1];
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setZOffset(double zOffset)
{
  this->GraphRepresentation->SetZOffset(zOffset);
  this->LineActor->SetPosition(0.0, 0.0, zOffset);
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setVertexSize(double size)
{
  return this->GraphRepresentation->SetVertexSize(size);
}

//-----------------------------------------------------------------------------
double vpGraphModelView::getVertexSize()
{
  return this->GraphRepresentation->GetVertexSize();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setVertexOpacity(double opacity)
{
  return this->GraphRepresentation->SetVertexOpacity(opacity);
}

//-----------------------------------------------------------------------------
double vpGraphModelView::getVertexOpacity()
{
  return this->GraphRepresentation->GetVertexOpacity();
}

//-----------------------------------------------------------------------------
vpMultiGraphModel::NodePositionType vpGraphModelView::getNodePositionType()
{
  return this->GraphRepresentation->GetNodePositionType();
}

//-----------------------------------------------------------------------------
vtkMatrix3x3* vpGraphModelView::getGraphToSpatialMatrix()
{
  return this->GraphRepresentation->GetGraphToSpatialMatrix();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setActive(bool active)
{
  if (active)
    {
    vtkConnect(
      this->RenderWindow->GetInteractor(),
      vtkCommand::LeftButtonPressEvent,
      this, SLOT(leftMousePress()));

    vtkConnect(
      this->RenderWindow->GetInteractor(),
      vtkCommand::LeftButtonReleaseEvent,
      this, SLOT(leftMouseRelease()));

    vtkConnect(
      this->RenderWindow->GetInteractor(),
      vtkCommand::MouseMoveEvent,
      this, SLOT(mouseMove()));

    vtkConnect(
      this->InteractorStyle,
      vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
      this, SLOT(updateSelection()));

    // Connect the delete event observer at a higher-than-default priority so
    // that we can "intercept" the event before other observers see it.
    vtkConnect(
      this->InteractorStyle,
      vtkVgInteractorStyleRubberBand2D::KeyPressEvent_Delete,
      this, SLOT(deleteSelected(vtkObject*, unsigned long, void*, void*,
                                vtkCommand*)),
      0, 1.0);

    // Add an observer for the render start event that will ensure we have been
    // updated before a render occurs. This is needed to prevent any renders
    // happening before we've had a chance to remove expired props.
    this->RenderWindow->AddObserver(vtkCommand::StartEvent,
                                    this->RenderCallback);
    }
  else
    {
    vtkDisconnect(
      this->RenderWindow->GetInteractor(),
      vtkCommand::LeftButtonPressEvent,
      this, SLOT(leftMousePress()));

    vtkDisconnect(
      this->RenderWindow->GetInteractor(),
      vtkCommand::LeftButtonReleaseEvent,
      this, SLOT(leftMouseRelease()));

    vtkDisconnect(
      this->RenderWindow->GetInteractor(),
      vtkCommand::MouseMoveEvent,
      this, SLOT(mouseMove()));

    vtkDisconnect(
      this->InteractorStyle,
      vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
      this, SLOT(updateSelection()));

    vtkDisconnect(
      this->InteractorStyle,
      vtkVgInteractorStyleRubberBand2D::KeyPressEvent_Delete,
      this, SLOT(deleteSelected(vtkObject*, unsigned long, void*, void*,
                                vtkCommand*)));

    this->RenderWindow->RemoveObserver(this->RenderCallback);
    this->enableRubberBand();
    }

  this->GraphRepresentation->SetVisible(active ? 1 : 0);
}

//-----------------------------------------------------------------------------
void vpGraphModelView::update()
{
  if (!this->GraphRepNeedsUpdate)
    {
    return;
    }

  this->GraphRepNeedsUpdate = false;
  this->GraphRepresentation->Update();

  vtkPropCollection* expProps
    = this->GraphRepresentation->GetExpiredRenderObjects();
  expProps->InitTraversal();
  while (vtkProp* prop = expProps->GetNextProp())
    {
    this->Renderer->RemoveViewProp(prop);
    }

  vtkPropCollection* newProps
    = this->GraphRepresentation->GetNewRenderObjects();
  newProps->InitTraversal();
  while (vtkProp* prop = newProps->GetNextProp())
    {
    this->Renderer->AddViewProp(prop);
    }

  this->GraphRepresentation->ResetTemporaryRenderObjects();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::render()
{
  if (!this->RenderPending)
    {
    this->RenderPending = true;
    QMetaObject::invokeMethod(this, "forceRender", Qt::QueuedConnection);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelView::forceRender()
{
  this->RenderPending = false;
  this->Renderer->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::deleteSelected(vtkObject*, unsigned long, void*, void*,
                                      vtkCommand* command)
{
  if (this->CurrentSelectionType == ST_Nodes)
    {
    emit this->deleteSelectedNodesRequested();
    }
  else if (this->CurrentSelectionType == ST_Edges)
    {
    emit this->deleteSelectedEdgesRequested();
    }
  else
    {
    return;
    }

  // Don't let other observers see the event
  command->SetAbortFlag(1);
}

//-----------------------------------------------------------------------------
void vpGraphModelView::selectNodes(vtkIdTypeArray* ids, bool append)
{
  this->CurrentSelectionType = ids ? ST_Nodes : ST_None;
  emit this->nodesSelected(ids, append);
}

//-----------------------------------------------------------------------------
void vpGraphModelView::selectEdges(vtkIdTypeArray* ids, const QString& domain,
                                   bool append)
{
  this->CurrentSelectionType = ids ? ST_Edges : ST_None;
  emit this->edgesSelected(ids, domain, append);
}

//-----------------------------------------------------------------------------
void vpGraphModelView::enableRubberBand()
{
  if (this->PrevRubberBandMode != -1)
    {
    this->InteractorStyle->SetRubberBandMode(this->PrevRubberBandMode);
    this->PrevRubberBandMode = -1;
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelView::disableRubberBand()
{
  this->PrevRubberBandMode = this->InteractorStyle->GetRubberBandMode();
  this->InteractorStyle->SetRubberBandModeToDisabled();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setCreateNodeEnabled(bool enable)
{
  this->CreateNodeEnabled = enable;
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setLightForegroundModeEnabled(bool enable)
{
  double fgColor[3];
  fgColor[0] = fgColor[1] = fgColor[2] = enable ? 1.0 : 0.0;

  this->GraphRepresentation->SetForegroundColor(fgColor);
  this->LineActor->GetProperty()->SetColor(fgColor);
}

//-----------------------------------------------------------------------------
void vpGraphModelView::setCurrentVisibleGraph(const QString& key)
{
  std::string kstr = stdString(key);
  this->GraphRepresentation->SetCurrentVisibleGraph(kstr);
  this->render();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::leftMousePress()
{
  if (this->CreateNodeEnabled)
    {
    int pos[2];
    this->RenderWindow->GetInteractor()->GetEventPosition(pos);

    double pt[3] = { double(pos[0]), double(pos[1]), 0.0 };
    vtkVgSpaceConversion::DisplayToWorldNormalized(this->Renderer, pt, pt);
    emit this->createNodeRequested(pt[0], pt[1]);
    return;
    }

  int pos[2];
  vtkIdType pickType;
  this->RenderWindow->GetInteractor()->GetEventPosition(pos);
  vtkIdType id = this->GraphRepresentation->Pick(pos[0], pos[1],
                                                 this->Renderer, pickType);
  if (id != -1)
    {
    if (pickType == vtkVgPickData::PickedGraphNode)
      {
      double nodeX, nodeY, nodeZ;
      this->GraphModel->GetNodePosition(
        this->GraphRepresentation->GetNodePositionType(), id,
        nodeX, nodeY, nodeZ);

      if (this->RenderWindow->GetInteractor()->GetControlKey())
        {
        // Node connection mode
        this->LinePoints->SetPoint(0, nodeX, nodeY, -0.1);
        this->LinePoints->SetPoint(1, nodeX, nodeY, -0.1);
        this->LinePoints->Modified();
        this->LineActor->VisibilityOn();
        this->ConnectParentNode = static_cast<int>(id);
        this->selectNodes(this->GraphRepresentation->GetPickedIds());
        }
      else
        {
        bool idIsSelected = false;
        vtkIdTypeArray* selected = this->GraphModel->GetSelectedNodes();
        for (vtkIdType i = 0, end = selected->GetNumberOfTuples(); i != end;
             ++i)
          {
          if (id == selected->GetValue(i))
            {
            idIsSelected = true;
            break;
            }
          }

        // If the picked node is part of previous selection, don't clear the
        // selection yet since the user may be starting a drag.
        if (!idIsSelected)
          {
          if (this->RenderWindow->GetInteractor()->GetShiftKey())
            {
            this->selectNodes(this->GraphRepresentation->GetPickedIds(), true);
            }
          else
            {
            this->selectNodes(this->GraphRepresentation->GetPickedIds());
            this->HeldNode = static_cast<int>(id);
            }
          }
        else
          {
          this->HeldNode = static_cast<int>(id);
          }
        }
      // Prevent the interactor style from trying to draw a rubberband rect
      // while we are dragging.
      this->disableRubberBand();
      }
    else // vtkVgPickData::PickedGraphEdge
      {
      std::string domain = this->GraphRepresentation->GetPickedDomain();
      this->selectEdges(
        this->GraphRepresentation->GetPickedIds(), qtString(domain),
        this->RenderWindow->GetInteractor()->GetShiftKey() != 0);
      }
    }
  else
    {
    if (!this->RenderWindow->GetInteractor()->GetShiftKey())
      {
      this->selectNodes(0);
      }
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelView::mouseMove()
{
  int pos1[2];
  int pos2[2];
  this->RenderWindow->GetInteractor()->GetEventPosition(pos1);
  this->RenderWindow->GetInteractor()->GetLastEventPosition(pos2);

  double wpos1[3] = { double(pos1[0]), double(pos1[1]), 0.0 };
  double wpos2[3] = { double(pos2[0]), double(pos2[1]), 0.0 };

  vtkVgSpaceConversion::DisplayToWorldNormalized(this->Renderer,
                                                 wpos1, wpos1);
  vtkVgSpaceConversion::DisplayToWorldNormalized(this->Renderer,
                                                 wpos2, wpos2);

  if (this->ConnectParentNode != -1)
    {
    // Update connection line
    wpos1[2] = -0.1;
    this->LinePoints->SetPoint(1, wpos1);
    this->LinePoints->Modified();
    this->render();
    }
  else if (this->HeldNode != -1 && this->CurrentSelectionType == ST_Nodes)
    {
    double delta[] = { wpos1[0] - wpos2[0], wpos1[1] - wpos2[1] };

    vtkMatrix3x3* graphToSpatial =
      this->GraphRepresentation->GetGraphToSpatialMatrix();

    if (!this->DragStarted)
      {
      this->DragStarted = true;
      emit this->aboutToMoveNode();
      }

    // Move dragged nodes
    vtkIdTypeArray* selected = this->GraphModel->GetSelectedNodes();
    int layout = this->GraphRepresentation->GetLayoutMode();
    for (vtkIdType i = 0, end = selected->GetNumberOfTuples(); i != end; ++i)
      {
      int id = static_cast<int>(selected->GetValue(i));
      bool isSpatialLayout =
        layout == vpMultiGraphRepresentation::Spatial ||
        layout == vpMultiGraphRepresentation::RawSpatial;

      bool isTemporalLayout =
        layout == vpMultiGraphRepresentation::SortByStartTime ||
        layout == vpMultiGraphRepresentation::RawSpatial;

      if ((isSpatialLayout || isTemporalLayout) &&
          this->GraphModel->GetNodeEventId(id) != -1)
        {
        continue;
        }

      double x, y, z;
      vpMultiGraphModel::NodePositionType posType =
        this->GraphRepresentation->GetNodePositionType();
      this->GraphModel->GetNodePosition(posType, id, x, y, z);
      // TODO Fix this
      this->GraphModel->MoveNode(posType, id, x + delta[0], y + delta[1], z,
                                 true);

      // In spatial layout mode, also compute the new position for the node in
      // the overlay (and vice-versa).
      if (layout == vpMultiGraphRepresentation::Spatial)
        {
        double pt[] = { x + delta[0], y + delta[1], 1.0 };
        graphToSpatial->MultiplyPoint(pt, pt);
        this->GraphModel->MoveNode(vpMultiGraphModel::NPT_Spatial, id,
                                   pt[0], pt[1], 0.0);
        }

      if (layout == vpMultiGraphRepresentation::SortByStartTime)
        {
          double position[3];
          this->GraphModel->GetNodePosition(vpMultiGraphModel::NPT_NormalizedTemporal,
                id, position[0], position[1], position[2]);

          vtkVgTimeStamp timestamp = this->GraphRepresentation->ComputeNodeStartTime(
            position[0], position[1], position[2]);

          this->GraphModel->SetNodeStartTime(id, timestamp.GetTime());
          this->GraphModel->SetNodeEndTime(id, timestamp.GetTime());
          this->GraphModel->SetNodeStartFrame(id, timestamp.GetFrameNumber());
          this->GraphModel->SetNodeEndFrame(id, timestamp.GetFrameNumber());
        }
      }

    emit this->nodeMoved();
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelView::leftMouseRelease()
{
  if (this->ConnectParentNode != -1)
    {
    int pos[2];
    this->RenderWindow->GetInteractor()->GetEventPosition(pos);

    vtkIdType pickType;
    vtkIdType id = this->GraphRepresentation->Pick(pos[0], pos[1],
                                                   this->Renderer, pickType);
    // Link to hovered child node
    if (id != -1 && pickType == vtkVgPickData::PickedGraphNode)
      {
      emit this->nodesLinked(this->ConnectParentNode, static_cast<int>(id));
      }
    }
  else if (this->HeldNode != -1)
    {
    // Select *only* the held node now
    vtkSmartPointer<vtkIdTypeArray> ids =
      vtkSmartPointer<vtkIdTypeArray>::New();
    ids->InsertNextValue(this->HeldNode);
    this->selectNodes(ids);
    }

  this->DragStarted = false;
  this->HeldNode = -1;
  this->ConnectParentNode = -1;
  this->LineActor->VisibilityOff();
  this->enableRubberBand();
  this->render();
}

//-----------------------------------------------------------------------------
void vpGraphModelView::updateSelection()
{
  // Pick by bounding rect
  int* startPos = this->InteractorStyle->GetStartPosition();
  int* endPos = this->InteractorStyle->GetEndPosition();

  if (startPos[0] == endPos[0] && startPos[1] == endPos[1])
    {
    return;
    }

  vtkIdType pickType;
  vtkIdType id = this->GraphRepresentation->Pick(startPos[0], startPos[1],
                                                 endPos[0], endPos[1],
                                                 this->Renderer,
                                                 pickType);
  if (id == -1)
    {
    this->selectNodes(0);
    }
  else
    {
    if (pickType == vtkVgPickData::PickedGraphNode)
      {
      this->selectNodes(this->GraphRepresentation->GetPickedIds());
      }
    else // Picked graph edge
      {
      std::string domain = this->GraphRepresentation->GetPickedDomain();
      this->selectEdges(this->GraphRepresentation->GetPickedIds(),
                        qtString(domain));
      }
    }
}
