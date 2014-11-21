/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVQBlastLayoutNode.h"

// VTK includes.
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkBoundingBox.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPropCollection.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// VG includes.
#include "vtkVgSelectionWidget.h"  // will be move to VTK in the future
#include "vtkVgEventRepresentationBase.h"
#include "vtkVgFindNode.h"
#include "vtkVgGeode.h"
#include "vtkVgSelectionListRepresentation.h"
#include "vtkVgTransformNode.h"
#include "vtkVgVideoModel0.h"
#include "vtkVgVideoNode.h"
#include "vtkVgVideoRepresentation0.h"

// STL includes.
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>

vtkStandardNewMacro(vtkVQBlastLayoutNode);


const double vtkVQBlastLayoutNode::MinDelta  = 10.0;

//-----------------------------------------------------------------------------
class vtkVQBlastLayoutNode::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

  struct NewNodeBoundData
    {
    vtkVgNodeBase::SmartPtr Node;
    std::vector<double>     NodeBounds;
    };

  bool                                  Repeat;

  int                                   InstanceColorIndex;

  static int                            ColorIndex;

  static const int                      MaxNumberOfColors = 5;

  static double                         Colors[MaxNumberOfColors][3];

  vtkSmartPointer<vtkMatrix4x4>         CachedMatrix;
  vtkVgVideoNode::SmartPtr              CachedVideoNode;

  std::vector<vtkVgGeode::SmartPtr>     ConnectingNodes;
  std::vector<vtkVgNodeBase::SmartPtr>  SceneNodes;
  std::vector<std::vector<double> >     OriginalNodeBounds;
  std::vector<NewNodeBoundData>         NewNodeBounds;

  std::multimap<long long, vtkVgVideoNode*>
  SortedStack;
  std::multimap<long long, vtkVgVideoNode*>::iterator
  StackPositionIter;

  // NOTE: We could use same sorted list of video nodes in the future.
  std::multimap<long long, vtkVgVideoNode::SmartPtr>
  SortedZSortStack;
  int                                   PrevNumberOfChildren;


  // Description:
  // Helper functions.
  void MakeNonIntersectBounds(double delta[3], vtkBoundingBox& bb1,
                              vtkBoundingBox& bb2, double newNodeBounds[6],
                              double translation[3], bool& newBounds);

  void DoBlast(vtkVgNodeBase::SmartPtr node,
               std::vector<vtkVgTransformNode*>& otherNodes, int index,
               double delta[3], double nodeBounds[6], double translation[3]);

  void DoBlast(vtkVgNodeBase::SmartPtr node,
               std::vector<vtkVgNodeBase::SmartPtr>& otherNodes, double delta[3],
               double nodeBounds[6], double translation[3]);

  void DoBlast(vtkVgNodeBase::SmartPtr node,
               std::vector< std::vector<double> >& otherNodeBounds,
               double delta[3], double nodeBounds[6], double translation[3]);

  void DoBlast(vtkVgNodeBase::SmartPtr node,
               std::vector<NewNodeBoundData>& otherNodeBounds, double delta[3],
               double nodeBounds[6], double translation[3]);


  vtkVgGeode::SmartPtr
  CreateConnectingNodes(
    vtkMatrix4x4* matrix, vtkVgNodeBase* node, vtkVgGroupNode* parent);

  static double* GetColor(int index);

  void   ApplyZSort(vtkIdType videoInstanceId,
                    vtkPropCollection* propCollection, double zValue);
  void   RemoveZSort(vtkIdType videoInstanceId,
                     vtkPropCollection* propCollection);
};

int vtkVQBlastLayoutNode::vtkInternal::ColorIndex = -1.0;

//var. 1 = #FF7400 = rgb(255,116,0)
//var. 2 = #BF7130 = rgb(191,113,48)
//var. 3 = #A64B00 = rgb(166,75,0)
//var. 4 = #FF9640 = rgb(255,150,64)
//var. 5 = #FFB273 = rgb(255,178,115)

double vtkVQBlastLayoutNode::vtkInternal::Colors [MaxNumberOfColors][3] =
{
    {255.0 / 255.0, 116.0 / 255.0, 0.0 / 255.0},
    {191.0 / 255.0, 113.0 / 255.0, 48.0 / 255.0},
    {166.0 / 255.0, 75.0 / 255.0, 0.0 / 255.0},
    {255.0 / 255.0, 150.0 / 255.0, 64.0 / 255.0},
    {255.0 / 255.0, 178.0 / 255.0, 115.0 / 255.0}
};

//-----------------------------------------------------------------------------
vtkVQBlastLayoutNode::vtkInternal::vtkInternal()
{
  this->Repeat          = false;

  this->InstanceColorIndex  = (++this->ColorIndex) % MaxNumberOfColors;

  this->CachedMatrix    = 0;
  this->CachedVideoNode = 0;

  this->PrevNumberOfChildren = -1;

  this->StackPositionIter = this->SortedStack.end();
}

//-----------------------------------------------------------------------------
vtkVQBlastLayoutNode::vtkInternal::~vtkInternal()
{
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::vtkInternal::MakeNonIntersectBounds(double delta[3],
    vtkBoundingBox& bb1,
    vtkBoundingBox& bb2,
    double newNodeBounds[6],
    double translation[3],
    bool& newBounds)

{
  newBounds = false;

  while (bb1.Intersects(bb2))
    {
    newNodeBounds[0] += delta[0];
    newNodeBounds[1] += delta[0];
    newNodeBounds[2] += delta[1];
    newNodeBounds[3] += delta[1];

    bb1.SetBounds(&newNodeBounds[0]);

    translation[0] += delta[0];
    translation[1] += delta[1];

    newBounds = true;
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::vtkInternal::DoBlast(
  vtkVgNodeBase::SmartPtr vtkNotUsed(node),
  std::vector<vtkVgTransformNode*>& otherNodes,
  int index, double delta[3], double nodeBounds[6], double translation[3])
{
  // Check if this node is colliding against other nodes.
  // If it is move the node until its not colliding.
  int numberOfOtherNodes = static_cast<int>(otherNodes.size());

  vtkBoundingBox bb1;
  bb1.SetBounds(nodeBounds[0], nodeBounds[1], nodeBounds[2], nodeBounds[3], 0, 0);

  for (int j = (index + 1); j < numberOfOtherNodes; ++j)
    {
    bool newBounds = false;

    vtkBoundingBox bb2;
    double* bounds = otherNodes[j]->GetBounds();
    bb2.SetBounds(bounds[0], bounds[1], bounds[2], bounds[3], 0, 0);

    this->MakeNonIntersectBounds(delta,
                                 bb1,
                                 bb2,
                                 nodeBounds,
                                 translation,
                                 newBounds);

    if (newBounds)
      {
      this->Repeat = true;
      bb1.SetBounds(nodeBounds);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::vtkInternal::DoBlast(
  vtkVgNodeBase::SmartPtr vtkNotUsed(node),
  std::vector<vtkVgNodeBase::SmartPtr>& otherNodes,
  double delta[3], double nodeBounds[6], double translation[3])
{
  // Check if this node is colliding against other nodes.
  // If it is move the node until its not colliding.
  int numberOfOtherNodes = static_cast<int>(otherNodes.size());

  vtkBoundingBox bb1;
  bb1.SetBounds(nodeBounds[0], nodeBounds[1], nodeBounds[2], nodeBounds[3], 0, 0);

  for (int j = 0; j < numberOfOtherNodes; ++j)
    {
    bool newBounds = false;

    vtkBoundingBox bb2;
    double* bounds = otherNodes[j]->GetBounds();
    bb2.SetBounds(bounds[0], bounds[1], bounds[2], bounds[3], 0, 0);

    this->MakeNonIntersectBounds(delta,
                                 bb1,
                                 bb2,
                                 nodeBounds,
                                 translation,
                                 newBounds);

    if (newBounds)
      {
      this->Repeat = true;
      bb1.SetBounds(nodeBounds);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::vtkInternal::DoBlast(
  vtkVgNodeBase::SmartPtr vtkNotUsed(node),
  std::vector< std::vector<double> >& otherNodeBounds,
  double delta[3], double nodeBounds[3], double translation[3])
{
  // Check if this node is colliding against other nodes.
  // If it is move the node until its not colliding.
  int numberOfOtherBounds = static_cast<int>(otherNodeBounds.size());

  vtkBoundingBox bb1;
  bb1.SetBounds(nodeBounds[0], nodeBounds[1], nodeBounds[2], nodeBounds[3], 0, 0);

  for (int j = 0; j < numberOfOtherBounds; ++j)
    {
    bool newBounds = false;

    vtkBoundingBox bb2;
    bb2.SetBounds(otherNodeBounds[j][0], otherNodeBounds[j][1],
                  otherNodeBounds[j][2], otherNodeBounds[j][3], 0, 0);

    this->MakeNonIntersectBounds(delta,
                                 bb1,
                                 bb2,
                                 nodeBounds,
                                 translation,
                                 newBounds);

    if (newBounds)
      {
      this->Repeat = true;
      bb1.SetBounds(nodeBounds);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::vtkInternal::DoBlast(vtkVgNodeBase::SmartPtr node,
                                                std::vector<NewNodeBoundData>& otherNodeBounds, double delta[3],
                                                double nodeBounds[3], double translation[3])
{
  // Check if this node is colliding against other nodes.
  // If it is move the node until its not colliding.
  int numberOfOtherBounds = static_cast<int>(otherNodeBounds.size());

  vtkBoundingBox bb1;
  bb1.SetBounds(nodeBounds[0], nodeBounds[1], nodeBounds[2], nodeBounds[3], 0, 0);

//  bool addNewBound = false;

  for (int j = 0; j < numberOfOtherBounds; ++j)
    {
    if (node == otherNodeBounds[j].Node)
      {
      continue;
      }

    bool newBounds = false;

    vtkBoundingBox bb2;
    bb2.SetBounds(otherNodeBounds[j].NodeBounds[0], otherNodeBounds[j].NodeBounds[1],
                  otherNodeBounds[j].NodeBounds[2], otherNodeBounds[j].NodeBounds[3], 0, 0);

    this->MakeNonIntersectBounds(delta,
                                 bb1,
                                 bb2,
                                 nodeBounds,
                                 translation,
                                 newBounds);

    if (newBounds)
      {
      this->Repeat = true;
      bb1.SetBounds(nodeBounds);
      }
    }
}

//-----------------------------------------------------------------------------
vtkVgGeode::SmartPtr vtkVQBlastLayoutNode::vtkInternal::CreateConnectingNodes(
  vtkMatrix4x4* transformationMatrix, vtkVgNodeBase* node,
  vtkVgGroupNode* vtkNotUsed(parent))
{
  vtkVgGeode::SmartPtr                lineGeode(vtkVgGeode::SmartPtr::New());
  vtkSmartPointer<vtkActor>           lineActor(vtkSmartPointer<vtkActor>::New());
  vtkSmartPointer<vtkPolyDataMapper>  lineMapper(vtkSmartPointer<vtkPolyDataMapper>::New());
  vtkSmartPointer<vtkPolyData>        linePolyData(vtkSmartPointer<vtkPolyData>::New());
  vtkSmartPointer<vtkCellArray>       lineCells(vtkSmartPointer<vtkCellArray>::New());
  vtkSmartPointer<vtkPoints>          linePoints(vtkSmartPointer<vtkPoints>::New());

  double* nodeBounds = node->GetBounds();
  double point1[4] = {(nodeBounds[1] + nodeBounds[0]) / 2.0,
                      (nodeBounds[3] + nodeBounds[2]) / 2.0,
                      0.0,
                      1.0
                     };
  double point2[4];
  transformationMatrix->MultiplyPoint(point1, point2);
  linePoints->InsertNextPoint(point1);
  linePoints->InsertNextPoint(point2);

  vtkIdType ptIds[2] = {0, 1};
  lineCells->InsertNextCell(2, ptIds);

  linePolyData->SetPoints(linePoints);
  linePolyData->SetLines(lineCells);
  lineMapper->SetInputData(linePolyData);
  lineActor->SetMapper(lineMapper);

  lineActor->GetProperty()->SetLineWidth(2.0);
  lineActor->GetProperty()->SetColor(this->GetColor(this->InstanceColorIndex));

  lineGeode->AddDrawable(lineActor);
  lineGeode->SetName("LineGeode");

  this->ConnectingNodes.push_back(lineGeode);

  return lineGeode;
}

//-----------------------------------------------------------------------------
double* vtkVQBlastLayoutNode::vtkInternal::GetColor(int index)
{
  if (index >= MaxNumberOfColors)
    {
    return 0;
    }

  return Colors[index];
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::vtkInternal::ApplyZSort(
  vtkIdType vtkNotUsed(videoInstanceId),
  vtkPropCollection* propCollection,
  double zValue)
{
  if (propCollection)
    {
    propCollection->InitTraversal();

    vtkProp3D* prop3d;
    while ((prop3d = vtkProp3D::SafeDownCast(propCollection->GetNextProp())))
      {
      vtkVgRepresentationBase::SetZOffset(prop3d, zValue);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::vtkInternal::RemoveZSort(
  vtkIdType vtkNotUsed(videoInstanceId), vtkPropCollection* propCollection)
{
  if (propCollection)
    {
    propCollection->InitTraversal();

    vtkProp3D* prop3d;
    while ((prop3d = vtkProp3D::SafeDownCast(propCollection->GetNextProp())))
      {
      vtkVgRepresentationBase::SetZOffset(prop3d, 0);
      }
    }
}

//-----------------------------------------------------------------------------
vtkVQBlastLayoutNode::vtkVQBlastLayoutNode() : vtkVgGroupNode(),
  LayoutMode(Z_SORT),
  Blast(1),
  ZSort(1),
  Stack(1),
  NodeFocused(0),
  ZSortOffsetFactor(100.0)
{
  this->Implementation = new vtkInternal();

  this->StackSelectionWidget = vtkVgSelectionWidget::New();
  this->StackSelectionWidget->EnabledOff(); // make sure off initially
  vtkVgSelectionListRepresentation* rep = vtkVgSelectionListRepresentation::New();
  rep->GetMarkerActor()->GetProperty()->SetColor(1.0, 0.08, 0.58);
  rep->SetOrientationToVertical();
  this->StackSelectionWidget->SetRepresentation(rep);
  rep->FastDelete();
}

//-----------------------------------------------------------------------------
vtkVQBlastLayoutNode::~vtkVQBlastLayoutNode()
{
  delete this->Implementation;
  this->Implementation = 0;
  this->StackSelectionWidget->Delete();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::PrintSelf(ostream& vtkNotUsed(os),
                                     vtkIndent vtkNotUsed(indent))
{
  // \TODO: Implement this.
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::SetLayoutMode(int mode)
{
  // Check if incoming mode is valid.
  if (mode != Z_SORT &&
      mode != STACK  &&
      mode != BLAST)
    {
    // Do nothing.
    return;
    }

  if (this->LayoutMode == STACK)
    {
    this->UndoStackLayout();

    // DoStackOn()...
    this->StackOff();
    }
  else if (this->LayoutMode == Z_SORT)
    {
    this->UndoZSortLayout();

    this->ZSortOff();
    }
  else
    {
    this->UndoBlastLayout();

    this->BlastOff();
    }

  if (mode == STACK)
    {
    this->StackOn();
    }
  else if (mode == Z_SORT)
    {
    this->ZSortOn();
    }
  else
    {
    this->BlastOn();
    }

  this->LayoutMode = mode;
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::AddSceneNode(vtkVgNodeBase* node)
{
  if (!node)
    {
    return;
    }

  // Check if the node exist as a children to this layout node.
  if (!vtkVgFindNode::ContainsNode(this, node))
    {
    std::vector<double> bounds(6, 0.0);
    node->ComputeBounds();
    node->GetBounds(&bounds[0]);

    this->Implementation->SceneNodes.push_back(node);
    this->Implementation->OriginalNodeBounds.push_back(bounds);
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::FocusNode(vtkVgVideoNode* node)
{
  if (!node)
    {
    return;
    }

  // NOTE: I believe we don't need to rely on this now.
  if (!node->GetVideoRepresentation()->GetVideoModel()->IsPlaying())
    {
    return;
    }

  if (vtkVgTransformNode* parent =
        static_cast<vtkVgTransformNode*>(node->GetParent()))
    {
    this->Implementation->CachedVideoNode = node;
    this->Implementation->CachedMatrix = parent->GetMatrix();
    parent->SetMatrix(vtkSmartPointer<vtkMatrix4x4>::New());

    this->NodeFocused = 1;
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::RestoreFocusedNodePosition()
{
  if (vtkVgVideoNode* prevPickedNode = this->Implementation->CachedVideoNode)
    {
    if (prevPickedNode->GetVideoRepresentation()->GetVideoModel()->IsPlaying())
      {
      std::cerr << "Video still playing. Inconsistent state." << std::endl;
      return;
      }

    if (vtkVgTransformNode* parent =
          static_cast<vtkVgTransformNode*>(prevPickedNode->GetParent()))
      {
      parent->SetMatrix(this->Implementation->CachedMatrix, true);
      this->NodeFocused = 0;
      }
    this->Implementation->CachedVideoNode = 0;
    this->Implementation->CachedMatrix = 0;

    this->SetChildrenVisible(1);
    this->UpdateStackLayout();
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::SetChildrenVisible(int visible)
{
  int numberOfChildren = this->GetNumberOfChildren();
  for (int i = 0; i < numberOfChildren; ++i)
    {
    if (vtkVgNodeBase* node = this->GetChild(i))
      {
      node->SetVisible(visible);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::OnPlayStop(vtkVgVideoNode& node)
{
  // Do nothing if the incoming node is not visible, this could be the case
  // where the node is not visible on context but is shown in video player.
  if (!node.GetVisible())
    {
    return;
    }

  vtkVgVideoNode* videoNode = dynamic_cast<vtkVgVideoNode*>(&node);
  if (!videoNode)
    {
    return;
    }

  vtkVgVideoNode* prevPickedNode = this->Implementation->CachedVideoNode;

  bool isContained = false;

  isContained = vtkVgFindNode::ContainsNode(this, videoNode);

  // if the selected node belongs to someone else and we had picked a node previously
  // return after storing the state.
  if ((!isContained && prevPickedNode))
    {
    this->RestoreFocusedNodePosition();

    return;
    }

  // If we have picked the same node twice, restore the state and return.
  if (prevPickedNode && prevPickedNode == videoNode)
    {
    this->RestoreFocusedNodePosition();

    return;
    }

  // Node picked is one of its own children but different than last one.
  if (isContained && prevPickedNode && prevPickedNode != videoNode)
    {
    this->RestoreFocusedNodePosition();
    }

  // Swap the focused node if we are not re-picking the same node as before
  if (prevPickedNode && prevPickedNode != videoNode)
    {
    if (vtkVgTransformNode* parent =
          static_cast<vtkVgTransformNode*>(videoNode->GetParent()))
      {
      this->FocusNode(videoNode);

      if (this->NodeFocused)
        {
        parent->SetVisible(1);

        if (vtkVgNodeBase* prevParent = prevPickedNode->GetParent())
          {
          prevParent->SetVisible(0);
          }
        }
      }
    return;
    }

  // If we are picking for the first time.
  if (isContained)
    {
    // focus picked node
    if (vtkVgTransformNode* parent =
          static_cast<vtkVgTransformNode*>(videoNode->GetParent()))
      {
      this->FocusNode(videoNode);
      if (this->NodeFocused)
        {
        this->SetChildrenVisible(0);
        parent->SetVisible(1);
        }
      }
    }
  else
    {
    // If the incoming node does not belong to this and is playing then
    // we have to make sure that we make this layout visible again
    // but then since it will make all the children visible and hence
    // will break the stack layout we would have to call update on stack layout.
    if (!videoNode->GetVideoRepresentation()->GetVideoModel()->IsPlaying())
      {
      this->SetVisible(1);
      this->UpdateStackLayout();
      }
    else
      {
      // Else make this node not visible.
      this->SetVisible(0);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::Update(vtkVgNodeVisitorBase& nodeVisitor)
{
  this->Superclass::Update(nodeVisitor);

  if ((this->LayoutMode == BLAST) && this->Blast)
    {
    this->SetLayoutToBlast(nodeVisitor);

    this->BlastOff();
    }
  else if (this->LayoutMode == Z_SORT && this->ZSort)
    {
    this->SetLayoutToZSort(nodeVisitor);

    this->ZSortOff();
    }
  else if (this->LayoutMode == STACK && this->Stack)
    {
    this->SetLayoutToStack(nodeVisitor);

    this->StackOff();
    }
  else
    {
    // Do nothing.
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::Traverse(vtkVgNodeVisitorBase& nodeVisitor)
{
  this->Superclass::Traverse(nodeVisitor);
}

//-----------------------------------------------------------------------------
bool vtkVQBlastLayoutNode::IsCoincidental(double bounds1[6], double bounds2[6])
{
  const double EPS = 0.00000000001;
  if (fabs(bounds1[0] - bounds2[0]) <= EPS &&
      fabs(bounds1[1] - bounds2[1]) <= EPS &&
      fabs(bounds1[2] - bounds2[2]) <= EPS &&
      fabs(bounds1[3] - bounds2[3]) <= EPS)
    {
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::SetLayoutToBlast(vtkVgNodeVisitorBase& nodeVisitor)
{
// First find all the video with exact same bound center as this node.
  std::vector<vtkVgTransformNode*> coincidentalNodes;
  std::vector<vtkVgTransformNode*> overlappingNodes;

  this->Implementation->NewNodeBounds.clear();
  this->Implementation->Repeat = false;
  this->Implementation->ConnectingNodes.clear();

  int numberOfChildren = this->GetNumberOfChildren();

  this->Superclass::TraverseChildren(nodeVisitor);

  if (this->GetBoundsDirty())
    {
    this->ComputeBounds();
    }

  for (int i = 0; i < numberOfChildren; ++i)
    {
    vtkVgTransformNode* transformNode1 =
      dynamic_cast<vtkVgTransformNode*>(this->GetChild(i));

    // If the video transform or its video is not visible ignore it.
    if (!transformNode1->GetVisible() || (transformNode1->GetChild(0)
                                          && !transformNode1->GetChild(0)->GetVisible()))
      {
      continue;
      }

    double* bounds1 = transformNode1->GetBounds();

    for (int j = (i + 1); j < this->GetNumberOfChildren(); ++j)
      {
      vtkVgTransformNode* transformNode2 =
        dynamic_cast<vtkVgTransformNode*>(this->GetChild(j));

      double* bounds2 = transformNode2->GetBounds();

      if (this->IsCoincidental(bounds1, bounds2))
        {
        if (std::find(coincidentalNodes.begin(), coincidentalNodes.end(),
                      transformNode1) == coincidentalNodes.end())
          {
          coincidentalNodes.push_back(transformNode1);
          }
        if (std::find(coincidentalNodes.begin(),
                      coincidentalNodes.end(), transformNode2) == coincidentalNodes.end())
          {
          coincidentalNodes.push_back(transformNode2);
          }
        }
      }

    if (std::find(coincidentalNodes.begin(),
                  coincidentalNodes.end(), transformNode1) == coincidentalNodes.end())
      {
      overlappingNodes.push_back(transformNode1);
      }
    }
// Now we have exact overlapping nodes and just overlapping nodes,
// we need to find a random distance seed.
  const double someFactor = 1.2;

  double blastDistnace = MinDelta;

  int totalNumberOfChildren = this->GetNumberOfChildren();
  for (int ll = 0; ll < totalNumberOfChildren; ++ll)
    {
    vtkVgTransformNode::SmartPtr videoTransform =
      dynamic_cast<vtkVgTransformNode*>(this->GetChild(ll));
    if (videoTransform && videoTransform->GetNumberOfChildren())
      {
      double* nodeBounds = videoTransform->GetChild(0)->GetBounds();
      double distance = (nodeBounds[1] - nodeBounds[0]) * someFactor;

      if (distance > 0.0)
        {
        blastDistnace = distance;
        break;
        }
      }
    }

  double coinAngleSeparation = 0.0;

  if (!coincidentalNodes.empty())
    {
    coinAngleSeparation =
      360.0 / static_cast<double>(coincidentalNodes.size());
    }

  double coinAngle = 0.0;

  double ovrAngleSeparation = 0.0;

  if (!overlappingNodes.empty())
    {
    ovrAngleSeparation = 360.0 / static_cast<double>(overlappingNodes.size());
    }

  double ovrAngle = 0.0;

// First deal with the just overlapping nodes.
  int numberOfOverlappingNodes  = static_cast<int>(overlappingNodes.size());
  int numberOfCoincidentalNodes = static_cast<int>(coincidentalNodes.size());

// NOTE: Commented out for now.
// Get center of this blast node.
//double *overallBounds = this->GetBounds();
//double overallCenter[3] =
//  {
//  overallBounds[1] - overallBounds[0] / 2.0,
//  overallBounds[3] - overallBounds[2] / 2.0,
//  overallBounds[5] - overallBounds[4] / 2.0,
//  };

// Deal with just overlapping nodes.
//-------------------------------------------------------------------------
  for (int i = 0; i < numberOfOverlappingNodes; ++i)
    {
    double translation[3] = {0.0, 0.0, 0.0};
    double* bounds = overlappingNodes[i]->GetBounds();

    std::vector<double> newBounds(bounds, bounds + 6);

    // NOTE: Commented out for now.
//  double nodeCenter[3] =
//    {
//    bounds[1] - bounds[0] / 2.0,
//    bounds[3] - bounds[2] / 2.0,
//    bounds[5] - bounds[4] / 2.0,
//    };

//  double direction[3] = {
//    nodeCenter[0] - overallCenter[0],
//    nodeCenter[1] - overallCenter[1],
//    0.0};
//  vtkMath::Normalize(direction);

//  double delta[3] = {
//    blastDistnace * direction[0],
//    blastDistnace * direction[1],
//    0.0};

    double delta [3] =
      {
      blastDistnace * cos(ovrAngle *  3.14 / 180.0),
      blastDistnace * sin(ovrAngle *  3.14 / 180.0),
      0.0
      };

    // This is to make sure that during blast even the last node moves out.
    this->Implementation->OriginalNodeBounds.push_back(std::vector<double>(bounds, bounds + 6));
    \
    do
      {
      this->Implementation->Repeat = false;

      // Do blast against overlapping nodes.
      this->Implementation->DoBlast(overlappingNodes[i], overlappingNodes,
                                    i, delta, &newBounds[0], translation);

      // Do blast against exactOverlapping  nodes.
      if (coincidentalNodes.size() > 0)
        {
        std::vector<vtkVgTransformNode*> anyCoincidentalNode(
          1, coincidentalNodes[0]);
        this->Implementation->DoBlast(overlappingNodes[i], anyCoincidentalNode,
                                      -1, delta, &newBounds[0], translation);
        }

      // Do blast against scene  nodes.
      this->Implementation->DoBlast(overlappingNodes[i],
                                    this->Implementation->SceneNodes, delta, &newBounds[0], translation);

      // Do blast against original bounds.
      this->Implementation->DoBlast(overlappingNodes[i],
                                    this->Implementation->OriginalNodeBounds, delta, &newBounds[0], translation);

      // Do blast against the new node bounds if any.
      this->Implementation->DoBlast(overlappingNodes[i],
                                    this->Implementation->NewNodeBounds, delta, &newBounds[0], translation);

      }
    while (this->Implementation->Repeat);

    if (fabs(translation[0]) > 0.0 || fabs(translation [1]) > 0.0)
      {
      vtkInternal::NewNodeBoundData newNodeBoundData;
      newNodeBoundData.Node = overlappingNodes[i];
      newNodeBoundData.NodeBounds = newBounds;
      this->Implementation->NewNodeBounds.push_back(newNodeBoundData);

      vtkSmartPointer<vtkMatrix4x4> translationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      translationMatrix->SetElement(0, 3, translation[0]);
      translationMatrix->SetElement(1, 3, translation[1]);

      overlappingNodes[i]->SetMatrix(translationMatrix, true);

      vtkSmartPointer<vtkMatrix4x4> invertMat = vtkSmartPointer<vtkMatrix4x4>::New();
      invertMat->DeepCopy(overlappingNodes[i]->GetFinalMatrix());
      invertMat->Invert();

      vtkVgTransformNode::SmartPtr lineTransformNode(vtkVgTransformNode::SmartPtr::New());
      lineTransformNode->SetMatrix(invertMat);
      lineTransformNode->SetName("lineTransformNode");
      lineTransformNode->AddChild(this->Implementation->CreateConnectingNodes(
                                    translationMatrix, overlappingNodes[i], this));
      this->AddChild(lineTransformNode);
      }

    ovrAngle += ovrAngleSeparation;
    }

// Now deal with coincidental nodes.
// ---------------------------------------------------------------------------------
  for (int i = 0; i < numberOfCoincidentalNodes; ++i)
    {
    double translation[3] = {0.0, 0.0, 0.0};
    double* bounds = coincidentalNodes[i]->GetBounds();
    std::vector<double> newBounds(bounds, bounds + 6);
    double delta [3] =
      {
      blastDistnace * cos(coinAngle *  3.14 / 180.0),
      blastDistnace * sin(coinAngle *  3.14 / 180.0),
      0.0
      };

    // This is to make sure that during blast even the last node moves out.
    this->Implementation->OriginalNodeBounds.push_back(std::vector<double>(bounds, bounds + 6));

    do
      {
      this->Implementation->Repeat = false;


      // Do blast against the coincidental nodes.
      this->Implementation->DoBlast(coincidentalNodes[i], coincidentalNodes, i,
                                    delta, &newBounds[0], translation);

      // Do blast against the just overlapping nodes.
      this->Implementation->DoBlast(coincidentalNodes[i], overlappingNodes, -1,
                                    delta, &newBounds[0], translation);

      // Do blast against the scene nodes.
      // If these are coincidental with this node
      // then we have to make sure we test blast
      // against this node itself.
      if (IsCoincidental(bounds, this->GetBounds()))
        {
        this->Implementation->OriginalNodeBounds.push_back(std::vector<double>(bounds, (bounds + 6)));
        }
      this->Implementation->DoBlast(coincidentalNodes[i],
                                    this->Implementation->SceneNodes, delta, &newBounds[0], translation);

      // Do blast against the original node bounds.
      this->Implementation->DoBlast(coincidentalNodes[i],
                                    this->Implementation->OriginalNodeBounds, delta, &newBounds[0], translation);

      // Do blast against the new node bounds if any.
      this->Implementation->DoBlast(coincidentalNodes[i],
                                    this->Implementation->NewNodeBounds, delta, &newBounds[0], translation);

      }
    while (this->Implementation->Repeat);

    if (fabs(translation[0]) > 0.0 || fabs(translation [1]) > 0.0)
      {
      vtkInternal::NewNodeBoundData newNodeBoundData;
      newNodeBoundData.Node = coincidentalNodes[i];
      newNodeBoundData.NodeBounds = newBounds;
      this->Implementation->NewNodeBounds.push_back(newNodeBoundData);

      vtkSmartPointer<vtkMatrix4x4> translationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      translationMatrix->SetElement(0, 3, translation[0]);
      translationMatrix->SetElement(1, 3, translation[1]);

      coincidentalNodes[i]->SetMatrix(translationMatrix, true);

      vtkSmartPointer<vtkMatrix4x4> invertMat = vtkSmartPointer<vtkMatrix4x4>::New();
      invertMat->DeepCopy(coincidentalNodes[i]->GetFinalMatrix());
      invertMat->Invert();

      vtkVgTransformNode::SmartPtr lineTransformNode(vtkVgTransformNode::SmartPtr::New());
      lineTransformNode->SetMatrix(invertMat);
      lineTransformNode->SetName("lineTransformNode");
      lineTransformNode->AddChild(this->Implementation->CreateConnectingNodes(
                                    translationMatrix, coincidentalNodes[i], this));
      this->AddChild(lineTransformNode);
      }

    coinAngle += coinAngleSeparation;
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::OnNextInStack(vtkVgNodeBase& node)
{
  // no further procesing required if not in stack mode OR the node passed in
  // doesn't match the one on top (then must not be in this stack)
  if (this->LayoutMode != STACK ||
      this->Implementation->StackPositionIter == this->Implementation->SortedStack.end() ||
      this->Implementation->StackPositionIter->second != &node)
    {
    return;
    }

  if (this->NodeFocused)
    {
    return;
    }

  this->MoveToNextVisibleNodeInStack();

  // update bar location
  this->UpdateStackRepresentationBarLocation();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::SetLayoutToZSort(
  vtkVgNodeVisitorBase& vtkNotUsed(nodeVisitor))
{
  int numberOfChildren = this->GetNumberOfChildren();

  // If this is the first time or if there has been a change in the number of children.
  // (though in the current case it will build the stack again and again as we are adding
  // addition nodes for the blast).
  if (this->Implementation->PrevNumberOfChildren == -1 ||
      numberOfChildren != this->Implementation->PrevNumberOfChildren)
    {
    this->Implementation->SortedZSortStack.clear();

    for (int i = 0; i < numberOfChildren; ++i)
      {
      // Get the trasnform node.
      vtkVgTransformNode::SmartPtr transformNode =
        vtkVgTransformNode::SafeDownCast(this->GetChild(i));
      if (transformNode)
        {
        vtkVgVideoNode::SmartPtr currentVideoNode =
          vtkVgVideoNode::SafeDownCast(transformNode->GetChild(0));

        if (currentVideoNode)
          {
          this->Implementation->SortedZSortStack.insert(std::pair<double, vtkVgVideoNode::SmartPtr>(
                                                          currentVideoNode->GetRank(), currentVideoNode));
          }
        }
      }

    this->Implementation->PrevNumberOfChildren = numberOfChildren;
    }

  double zValue = 1.0;

  // Now apply zsort.
  std::multimap<long long, vtkVgVideoNode::SmartPtr>::reverse_iterator stackIter;
  for (stackIter = this->Implementation->SortedZSortStack.rbegin();
       stackIter != this->Implementation->SortedZSortStack.rend(); stackIter++)
    {
    vtkVgVideoRepresentation0::SmartPtr videoRepresentation =
      vtkVgVideoRepresentation0::SafeDownCast(stackIter->second->GetVideoRepresentation());

    if (videoRepresentation)
      {
      this->Implementation->ApplyZSort(stackIter->second->GetInstanceId(),
                                       videoRepresentation->GetActiveRenderObjects(), zValue);

      vtkVgEventRepresentationBase* eventRepresentation =
        videoRepresentation->GetEventRepresentation();
      if (eventRepresentation)
        {
        this->Implementation->ApplyZSort(stackIter->second->GetInstanceId(),
                                         eventRepresentation->GetActiveRenderObjects(), zValue);
        }

      zValue += 1.0;

      // Since model has not changed we need to manually force update representations.
      videoRepresentation->Update();
      }
    }
}

///////////////////////////////////////////////////////////////////////////////
void vtkVQBlastLayoutNode::UndoZSortLayout()
{
  // Now undo zsort.
  std::multimap<long long, vtkVgVideoNode::SmartPtr>::iterator stackIter;
  for (stackIter = this->Implementation->SortedZSortStack.begin();
       stackIter != this->Implementation->SortedZSortStack.end(); stackIter++)
    {
    vtkVgVideoRepresentation0::SmartPtr videoRepresentation =
      vtkVgVideoRepresentation0::SafeDownCast(stackIter->second->GetVideoRepresentation());

    if (videoRepresentation)
      {
      this->Implementation->RemoveZSort(stackIter->second->GetInstanceId(),
                                        videoRepresentation->GetActiveRenderObjects());

      vtkVgEventRepresentationBase* eventRepresentation =
        videoRepresentation->GetEventRepresentation();
      if (eventRepresentation)
        {
        this->Implementation->RemoveZSort(stackIter->second->GetInstanceId(),
                                          eventRepresentation->GetActiveRenderObjects());
        }

      videoRepresentation->Update();
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::SetLayoutToStack(
  vtkVgNodeVisitorBase& vtkNotUsed(nodeVisitor))
{
  // Z Sort likely to use this as well, but for now just create for our needs
  if (this->Implementation->SortedStack.size() == 0)
    {
    int numberOfChildren = this->GetNumberOfChildren();
    for (int i = 0; i < numberOfChildren; ++i)
      {
      // Get the trasnform node.
      vtkVgTransformNode* transformNode =
        vtkVgTransformNode::SafeDownCast(this->GetChild(i));
      if (transformNode)
        {
        vtkVgVideoNode* currentVideoNode =
          vtkVgVideoNode::SafeDownCast(transformNode->GetChild(0));

        if (currentVideoNode)
          {
          this->Implementation->SortedStack.insert(std::pair<double, vtkVgVideoNode*>(
                                                     currentVideoNode->GetRank(), currentVideoNode));
          }
        }
      }
    }

  bool firstVisible = true;
  std::multimap<long long, vtkVgVideoNode*>::iterator stackIter;
  for (stackIter = this->Implementation->SortedStack.begin();
       stackIter != this->Implementation->SortedStack.end(); stackIter++)
    {
    if (stackIter->second->GetVisibleNodeMask() == VISIBLE_NODE_MASK)
      {
      if (firstVisible)
        {
        this->Implementation->StackPositionIter = stackIter;
        // should already be visible, but just in case
        stackIter->second->SetVisible(1);
        emit this->VisibilityChanged(*stackIter->second);
        firstVisible = false;
        }
      else
        {
        stackIter->second->SetVisible(0);
        }
      }
    }
  this->BuildStackRepresentationBar();
  this->UpdateStackRepresentationBarLocation();

  this->StackSelectionWidget->EnabledOn();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::UndoStackLayout()
{
  // make all the nodes visible
  // (VISIBLE_NODE_MASK will prevent us from turning on those user turned off)
  std::multimap<long long, vtkVgVideoNode*>::iterator stackIter;
  for (stackIter = this->Implementation->SortedStack.begin();
       stackIter != this->Implementation->SortedStack.end(); stackIter++)
    {
    stackIter->second->SetVisible(1);
    }

  this->StackSelectionWidget->EnabledOff();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::UndoBlastLayout()
{
  int numberOfChildren = this->GetNumberOfChildren();

  // Remove blast connection lines.
  std::vector<vtkVgTransformNode::SmartPtr> lineTransforms;
  for (int i = 0; i < numberOfChildren; ++i)
    {
    vtkVgTransformNode::SmartPtr transformNode =
      vtkVgTransformNode::SafeDownCast(this->GetChild(i));

    if (transformNode)
      {
      // Set the matrix to identity.
      transformNode->SetMatrix(vtkSmartPointer<vtkMatrix4x4>::New(), true);

      vtkVgGeode::SmartPtr lineGeode = vtkVgGeode::SafeDownCast(transformNode->GetChild(0));
      if (lineGeode)
        {
        lineTransforms.push_back(transformNode);
        }
      }
    }

  int numberOfLineTransforms = static_cast<int>(lineTransforms.size());
  for (int i = 0; i < numberOfLineTransforms; ++i)
    {
    this->RemoveChild(lineTransforms[i]);
    }
  lineTransforms.clear();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::UpdateStackLayout()
{
  if (this->LayoutMode != STACK)
    {
    return;
    }
  // in case items in the stack were turned on that aren't the top,
  // make sure evrything is "not visible" other than the top of our stack
  std::multimap<long long, vtkVgVideoNode*>::iterator stackIter;
  for (stackIter = this->Implementation->SortedStack.begin();
       stackIter != this->Implementation->SortedStack.end(); stackIter++)
    {
    if (stackIter != this->Implementation->StackPositionIter)
      {
      stackIter->second->SetVisible(0);
      }
    }

  if (this->Implementation->StackPositionIter != this->Implementation->SortedStack.end() &&
      this->Implementation->StackPositionIter->second->GetVisibleNodeMask() == VISIBLE_NODE_MASK)
    {
    return; // current top of stack is still visible; nothing to do
    }
  // current top of stack must no longer be visible, so find next visible item
  this->MoveToNextVisibleNodeInStack();

  // ok, not most efficient, but let's just rebuild the bar, even if it may
  // not have changed
  this->BuildStackRepresentationBar();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::MoveToNextVisibleNodeInStack()
{
  std::multimap<long long, vtkVgVideoNode*>::iterator startIter;
  if (this->Implementation->StackPositionIter != this->Implementation->SortedStack.end())
    {
    // make current invisble
    this->Implementation->StackPositionIter->second->SetVisible(0);
    startIter = ++this->Implementation->StackPositionIter;
    this->Implementation->StackPositionIter = this->Implementation->SortedStack.end();
    }
  else
    {
    startIter = this->Implementation->SortedStack.begin();
    }

  // look for next possible clip to make visible
  std::multimap<long long, vtkVgVideoNode*>::iterator stackIter = startIter;
  for (; stackIter != this->Implementation->SortedStack.end(); stackIter++)
    {
    if (stackIter->second->GetVisibleNodeMask() == VISIBLE_NODE_MASK)
      {
      stackIter->second->SetVisible(1);
      this->Implementation->StackPositionIter = stackIter;
      this->UpdateStackCurrentItemIndicator();
      emit this->VisibilityChanged(*stackIter->second);
      return;
      }
    }

  // loop around and check the rest of the list (from begin), but only if we
  // didn't already begin from begin.
  if (startIter != this->Implementation->SortedStack.begin())
    {
    for (stackIter = this->Implementation->SortedStack.begin();
         stackIter != startIter; stackIter++)
      {
      if (stackIter->second->GetVisibleNodeMask() == VISIBLE_NODE_MASK)
        {
        stackIter->second->SetVisible(1);
        emit this->VisibilityChanged(*stackIter->second);
        this->Implementation->StackPositionIter = stackIter;
        break;
        }
      }
    }
  this->UpdateStackCurrentItemIndicator();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::BuildStackRepresentationBar()
{
  if (this->LayoutMode != STACK)
    {
    return;
    }

  vtkVgSelectionListRepresentation* stackListRep =
    vtkVgSelectionListRepresentation::SafeDownCast(
      this->StackSelectionWidget->GetRepresentation());
  stackListRep->RemoveAllItems();
  std::multimap<long long, vtkVgVideoNode*>::const_iterator stackIter;
  for (stackIter = this->Implementation->SortedStack.begin();
       stackIter != this->Implementation->SortedStack.end(); stackIter++)
    {
    if (stackIter->second->GetVisibleNodeMask() == VISIBLE_NODE_MASK)
      {
      std::ostringstream oss;
      oss << stackIter->second->GetInstanceId();
      stackListRep->AddItem(
        stackIter->second->GetInstanceId(),
        stackIter->second->GetColorScalar(), oss.str().c_str(), NULL);
      }
    }
  this->UpdateStackCurrentItemIndicator();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::UpdateStackRepresentationBarLocation()
{
  if (this->LayoutMode != STACK)
    {
    return;
    }

  vtkVgVideoNode* placementNode = 0;
  if (this->Implementation->StackPositionIter !=
      this->Implementation->SortedStack.end())
    {
    placementNode = this->Implementation->StackPositionIter->second;
    }
  else if (this->Implementation->SortedStack.size() > 0)
    {
    placementNode = this->Implementation->SortedStack.begin()->second;
    }

  if (placementNode)
    {
    if (placementNode->GetBoundsDirty())
      {
      placementNode->ComputeBounds();
      }
    double* bounds = placementNode->GetBounds();

    vtkVgSelectionListRepresentation* stackListRep =
      vtkVgSelectionListRepresentation::SafeDownCast(
        this->StackSelectionWidget->GetRepresentation());

    vtkRenderer* renderer = this->StackSelectionWidget->GetInteractor()->
                            GetRenderWindow()->GetRenderers()->GetFirstRenderer();

    // vertical (axis aligned) bar (a bit) to the right of the clip,
    // same height as the clip
    renderer->SetWorldPoint(bounds[0], bounds[2], bounds[4], 1.0);
    renderer->WorldToDisplay();
    double minY = renderer->GetDisplayPoint()[1];
    renderer->SetWorldPoint(bounds[0], bounds[3], bounds[4], 1.0);
    renderer->WorldToDisplay();
    double maxY = renderer->GetDisplayPoint()[1];

    double barAspectRatio = 20;
    double height = maxY - minY;
    double width = height / barAspectRatio;
    stackListRep->SetListPosition(
      bounds[1] + (bounds[1] - bounds[0]) / 20.0, bounds[2], 100);
    stackListRep->SetListSize(width, height);
    stackListRep->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::MoveToSelectedStackItem()
{
  vtkIdType instanceID =
    this->StackSelectionWidget->GetSelectionRepresentation()->GetCurrentItemId();

  // if we have a visible one (not necessarily?), hide it
  if (this->Implementation->StackPositionIter != this->Implementation->SortedStack.end())
    {
    // make current invisble
    this->Implementation->StackPositionIter->second->SetVisible(0);
    this->Implementation->StackPositionIter = this->Implementation->SortedStack.end();
    }

  // if we can find the node that has this id, switch to it;  assumption is that
  // the list only shows visble clips, thus don't need to check VISIBLE_NODE_MASK
  std::multimap<long long, vtkVgVideoNode*>::iterator stackIter;
  for (stackIter = this->Implementation->SortedStack.begin();
       stackIter != this->Implementation->SortedStack.end(); stackIter++)
    {
    if (stackIter->second->GetInstanceId() == instanceID)
      {
      stackIter->second->SetVisible(1);
      emit this->VisibilityChanged(*stackIter->second);
      this->Implementation->StackPositionIter = stackIter;
      break;
      }
    }
  this->UpdateStackRepresentationBarLocation();
}

//-----------------------------------------------------------------------------
void vtkVQBlastLayoutNode::UpdateStackCurrentItemIndicator()
{
  if (this->Implementation->StackPositionIter != this->Implementation->SortedStack.end())
    {
    vtkVgSelectionListRepresentation::SafeDownCast(
      this->StackSelectionWidget->GetRepresentation())->SetSelectedItemId(
        this->Implementation->StackPositionIter->second->GetInstanceId());
    }
}
