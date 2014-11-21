/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpMultiGraphModel.h"

#include "vpEventConfig.h"
#include "vpPrimitiveConfig.h"

// VisGUI includes
#include <qtStlUtil.h>

#include <vtkCharArray.h>
#include <vtkCommand.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVariantArray.h>

#include <sstream>

vtkStandardNewMacro(vpMultiGraphModel);

//-----------------------------------------------------------------------------
class vpMultiGraphModel::vtkInternal
{
public:
  typedef std::map<std::string, int> GraphIsDirectedMap;
  typedef vtkSmartPointer<vtkMutableUndirectedGraph> vtkMutableUndirectedGraphPtr;
  typedef vtkSmartPointer<vtkMutableDirectedGraph> vtkMutableDirectedGraphPtr;
  typedef vtkSmartPointer<vtkPoints> vtkPointsPtr;
  typedef std::map<std::string, vtkMutableUndirectedGraphPtr> UndirectedGraphs;
  typedef std::map<std::string, vtkMutableDirectedGraphPtr> DirectedGraphs;

  vtkInternal(vpMultiGraphModel* parent)
    {
    this->Parent = parent;
    this->CurrNodeId = 0;
    }

  void AddNodes(vtkGraph* graph)
    {
    if (!graph)
      {
      return;
      }

    vtkIdType numberOfVertices = this->MasterGraph->GetNumberOfVertices();

    vtkMutableUndirectedGraphPtr uGraph =
      vtkMutableUndirectedGraph::SafeDownCast(graph);

    if (uGraph)
      {
      for (vtkIdType i = 0; i < numberOfVertices; ++i)
        {
        // \NOTE Every existing graph should have same vertices (id and count)
        uGraph->AddVertex();
        }

      uGraph->SetPoints(this->MasterPoints);
      return;
      }

    vtkMutableDirectedGraphPtr dGraph =
      vtkMutableDirectedGraph::SafeDownCast(graph);

    if (dGraph)
      {
      for (vtkIdType i = 0; i < numberOfVertices; ++i)
        {
        // \NOTE Every existing graph should have same vertices (id and count)
        dGraph->AddVertex();
        }

      dGraph->SetPoints(this->MasterPoints);
      return;
      }
    }

  void AddEdgeArrays(vtkGraph* graph)
    {
    if (!graph)
      {
      return;
      }

    vtkSmartPointer<vtkVariantArray> parentAttrs =
      vtkSmartPointer<vtkVariantArray>::New();

    parentAttrs->SetName("EdgeParentAttrs");

    vtkSmartPointer<vtkVariantArray> childAttrs =
      vtkSmartPointer<vtkVariantArray>::New();

    childAttrs->SetName("EdgeChildAttrs");

    vtkSmartPointer<vtkVariantArray> pedigreeIds =
      vtkSmartPointer<vtkVariantArray>::New();

    vtkSmartPointer<vtkCharArray> selected =
      vtkSmartPointer<vtkCharArray>::New();
    selected->SetName("Selected");

    graph->GetEdgeData()->AddArray(parentAttrs);
    graph->GetEdgeData()->AddArray(childAttrs);
    graph->GetEdgeData()->AddArray(selected);
    graph->GetEdgeData()->SetPedigreeIds(pedigreeIds);
    }

  void AddEdgeAttrs(vtkGraph* graph,
                    vtkIntArray* parentAttrs,
                    vtkIntArray* childAttrs)
    {
    if (vtkVariantArray* parentArr =
          vtkVariantArray::SafeDownCast(
            graph->GetEdgeData()->GetAbstractArray("EdgeParentAttrs")))
      {
      parentArr->InsertNextValue(parentAttrs);
      }

    if (vtkVariantArray* childArr =
          vtkVariantArray::SafeDownCast(
            graph->GetEdgeData()->GetAbstractArray("EdgeChildAttrs")))
      {
      childArr->InsertNextValue(childAttrs);
      }
    }

  void SetEdgeAttrs(const std::string& domain, int id,
                    vtkIntArray* attrs, const char* name)
    {
    vtkGraph* graph = this->Parent->GetGraph(domain);
    if (!graph)
      {
      return;
      }

    vtkIdType index =
      graph->GetEdgeData()->GetPedigreeIds()->LookupValue(id);

    if (index == -1)
      {
      return;
      }

    if (vtkVariantArray* array =
          vtkVariantArray::SafeDownCast(
            graph->GetEdgeData()->GetAbstractArray(name)))
      {
      array->SetValue(index, attrs);
      }
    }

  vtkIntArray* GetEdgeAttrs(const std::string& domain, int id,
                            const char* name)
    {
    vtkGraph* graph = this->Parent->GetGraph(domain);
    if (!graph)
      {
      return 0;
      }

    vtkIdType index =
      graph->GetEdgeData()->GetPedigreeIds()->LookupValue(id);

    if (index == -1)
      {
      return 0;
      }

    if (vtkVariantArray* array =
          vtkVariantArray::SafeDownCast(
            graph->GetEdgeData()->GetAbstractArray(name)))
      {
      return vtkIntArray::SafeDownCast(array->GetValue(index).ToArray());
      }

    return 0;
    }

  vtkDoubleArray* GetPositionArray(NodePositionType positionType)
    {
    switch (positionType)
      {
      case NPT_Spatial: return this->SpatialPositions;
      case NPT_NormalizedSpatial: return this->NormalizedSpatialPositions;
      case NPT_NormalizedTemporal: return this->NormalizedTemporalPositions;
      default: return this->CachedPositions;
      }
    }

  void AddPositionArrays(vtkGraph* graph)
    {
    vtkDataSetAttributes* vdata = graph->GetVertexData();
    vdata->AddArray(this->CachedPositions);
    vdata->AddArray(this->SpatialPositions);
    vdata->AddArray(this->NormalizedSpatialPositions);
    vdata->AddArray(this->NormalizedTemporalPositions);
    vdata->AddArray(this->StartPositions);
    vdata->AddArray(this->EndPositions);
    }

  void RemovePositionArrays(vtkGraph* graph)
    {
    vtkDataSetAttributes* vdata = graph->GetVertexData();
    vdata->RemoveArray(this->CachedPositions->GetName());
    vdata->RemoveArray(this->SpatialPositions->GetName());
    vdata->RemoveArray(this->NormalizedSpatialPositions->GetName());
    vdata->RemoveArray(this->NormalizedTemporalPositions->GetName());
    vdata->RemoveArray(this->StartPositions->GetName());
    vdata->RemoveArray(this->EndPositions->GetName());
    }

  vpMultiGraphModel* Parent;

  vtkMutableUndirectedGraphPtr MasterGraph;
  vtkPointsPtr MasterPoints;

  GraphIsDirectedMap GraphIsDirectedInfo;

  UndirectedGraphs DomainUndirectedGraphs;
  DirectedGraphs DomainDirectedGraphs;

  std::map<std::string, int> CurrentEdgeIds;

  // Node attributes
  vtkSmartPointer<vtkStringArray> NodeTypes;
  vtkSmartPointer<vtkStringArray> NodeLabels;
  vtkSmartPointer<vtkIntArray> NodeDefaultEdgeAttrs;
  vtkSmartPointer<vtkCharArray> NodeSelected;

  vtkSmartPointer<vtkUnsignedIntArray> GroupIds;

  vtkSmartPointer<vtkDoubleArray> StartTime;
  vtkSmartPointer<vtkDoubleArray> EndTime;
  vtkSmartPointer<vtkUnsignedIntArray> StartFrame;
  vtkSmartPointer<vtkUnsignedIntArray> EndFrame;
  vtkSmartPointer<vtkIdTypeArray> EventId;
  vtkSmartPointer<vtkUnsignedIntArray> NodeVirtualType;

  vtkSmartPointer<vtkDoubleArray> CachedPositions;
  vtkSmartPointer<vtkDoubleArray> SpatialPositions;
  vtkSmartPointer<vtkDoubleArray> NormalizedSpatialPositions;
  vtkSmartPointer<vtkDoubleArray> NormalizedTemporalPositions;
  vtkSmartPointer<vtkDoubleArray> StartPositions;
  vtkSmartPointer<vtkDoubleArray> EndPositions;

  // For holding the selected ids after a GetSelected* call
  vtkSmartPointer<vtkIdTypeArray> SelectedNodesHelper;
  vtkSmartPointer<vtkIdTypeArray> SelectedEdgesHelper;

  int CurrNodeId;
};

//-----------------------------------------------------------------------------
const std::string vpMultiGraphModel::NoneDomain = "None";

//-----------------------------------------------------------------------------
vpMultiGraphModel::vpMultiGraphModel() :
  vtkVgModelBase(),
  LockPositionOfNode(false),
  Internal(new vtkInternal(this))
{
  this->Internal->MasterGraph =
    vtkSmartPointer<vtkMutableUndirectedGraph>::New();
  this->Internal->MasterPoints =
    vtkSmartPointer<vtkPoints>::New();

  vtkSmartPointer<vtkVariantArray> nodePedigreeIds =
    vtkSmartPointer<vtkVariantArray>::New();

  this->Internal->MasterGraph->GetVertexData()->SetPedigreeIds(
    nodePedigreeIds);

  this->Internal->MasterGraph->SetPoints(
    this->Internal->MasterPoints);

  // Master graph is an undirected graph
  this->Internal->DomainUndirectedGraphs[vpMultiGraphModel::NoneDomain] =
    this->Internal->MasterGraph;

  this->Internal->NodeTypes = vtkSmartPointer<vtkStringArray>::New();
  this->Internal->NodeTypes->SetName("NodeTypes");

  this->Internal->NodeLabels = vtkSmartPointer<vtkStringArray>::New();
  this->Internal->NodeLabels->SetName("NodeLabels");

  this->Internal->NodeDefaultEdgeAttrs = vtkSmartPointer<vtkIntArray>::New();
  this->Internal->NodeDefaultEdgeAttrs->SetName("NodeDefaultEdgeAttrs");

  this->Internal->NodeSelected = vtkSmartPointer<vtkCharArray>::New();
  this->Internal->NodeSelected->SetName("Selected");

  this->Internal->GroupIds = vtkSmartPointer<vtkUnsignedIntArray>::New();
  this->Internal->GroupIds->SetName("GroupIds");

  this->Internal->StartTime = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->StartTime->SetName("StartTime");

  this->Internal->EndTime = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->EndTime->SetName("EndTime");

  this->Internal->StartFrame = vtkSmartPointer<vtkUnsignedIntArray>::New();
  this->Internal->StartFrame->SetName("StartFrame");

  this->Internal->EndFrame = vtkSmartPointer<vtkUnsignedIntArray>::New();
  this->Internal->EndFrame->SetName("EndFrame");

  this->Internal->EventId = vtkSmartPointer<vtkIdTypeArray>::New();
  this->Internal->EventId->SetName("EventId");

  this->Internal->NodeVirtualType = vtkSmartPointer<vtkUnsignedIntArray>::New();
  this->Internal->NodeVirtualType->SetName("NodeVirtualType");

  this->Internal->CachedPositions = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->CachedPositions->SetNumberOfComponents(3);
  this->Internal->CachedPositions->SetName("CachedPositions");

  this->Internal->SpatialPositions = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->SpatialPositions->SetNumberOfComponents(3);
  this->Internal->SpatialPositions->SetName("SpatialPositions");

  this->Internal->NormalizedSpatialPositions = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->NormalizedSpatialPositions->SetNumberOfComponents(3);
  this->Internal->NormalizedSpatialPositions->SetName("NormalizedSpatialPositions");

  this->Internal->NormalizedTemporalPositions = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->NormalizedTemporalPositions->SetNumberOfComponents(3);
  this->Internal->NormalizedTemporalPositions->SetName("NormalizedTemporalPositions");

  this->Internal->StartPositions = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->StartPositions->SetNumberOfComponents(2);
  this->Internal->StartPositions->SetName("StartPositions");

  this->Internal->EndPositions = vtkSmartPointer<vtkDoubleArray>::New();
  this->Internal->EndPositions->SetNumberOfComponents(2);
  this->Internal->EndPositions->SetName("EndPositions");

  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeTypes);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeLabels);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeDefaultEdgeAttrs);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeSelected);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->StartTime);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->EndTime);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->StartFrame);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->EndFrame);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->EventId);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeVirtualType);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->GroupIds);

  this->Internal->AddPositionArrays(this->Internal->MasterGraph);

  this->Internal->SelectedNodesHelper = vtkSmartPointer<vtkIdTypeArray>::New();
  this->Internal->SelectedEdgesHelper = vtkSmartPointer<vtkIdTypeArray>::New();
}

//-----------------------------------------------------------------------------
vpMultiGraphModel::~vpMultiGraphModel()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::Reset()
{
  this->Internal->MasterGraph->Initialize();

  this->Internal->MasterGraph->GetVertexData()->SetPedigreeIds(
    vtkSmartPointer<vtkVariantArray>::New());

  this->Internal->MasterPoints->SetNumberOfPoints(0);
  this->Internal->MasterGraph->SetPoints(this->Internal->MasterPoints);

  this->Internal->CurrNodeId = 0;
  this->Internal->CurrentEdgeIds.clear();

  this->Internal->NodeLabels->SetNumberOfTuples(0);
  this->Internal->NodeTypes->SetNumberOfTuples(0);
  this->Internal->NodeDefaultEdgeAttrs->SetNumberOfTuples(0);
  this->Internal->NodeSelected->SetNumberOfTuples(0);
  this->Internal->StartTime->SetNumberOfTuples(0);
  this->Internal->EndTime->SetNumberOfTuples(0);
  this->Internal->StartFrame->SetNumberOfTuples(0);
  this->Internal->EndFrame->SetNumberOfTuples(0);
  this->Internal->EventId->SetNumberOfTuples(0);
  this->Internal->NodeVirtualType->SetNumberOfTuples(0);
  this->Internal->GroupIds->SetNumberOfTuples(0);

  this->Internal->CachedPositions->SetNumberOfTuples(0);
  this->Internal->SpatialPositions->SetNumberOfTuples(0);
  this->Internal->NormalizedSpatialPositions->SetNumberOfTuples(0);
  this->Internal->NormalizedTemporalPositions->SetNumberOfTuples(0);

  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeTypes);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeLabels);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeDefaultEdgeAttrs);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->NodeSelected);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->StartTime);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->EndTime);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->StartFrame);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->EndFrame);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->EventId);
  this->Internal->MasterGraph->GetVertexData()->AddArray(
    this->Internal->GroupIds);

  this->Internal->AddPositionArrays(this->Internal->MasterGraph);

  this->Internal->DomainDirectedGraphs.clear();
  this->Internal->DomainUndirectedGraphs.clear();

  this->Internal->DomainUndirectedGraphs[vpMultiGraphModel::NoneDomain] =
    this->Internal->MasterGraph;

  this->Modified();
  this->InvokeEvent(vtkCommand::UpdateDataEvent);
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::SetGraphIsDirected(const std::string& domain, int val)
{
  // Cannot change graph type if already created
  if (this->Internal->DomainDirectedGraphs.find(domain) !=
      this->Internal->DomainDirectedGraphs.end())
    {
    return VTK_ERROR;
    }

  if (this->Internal->DomainUndirectedGraphs.find(domain) !=
      this->Internal->DomainUndirectedGraphs.end())
    {
    return VTK_ERROR;
    }

  val = val != 0 ? 1 : 0;
  this->Internal->GraphIsDirectedInfo[domain] = val;

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::GetGraphIsDirected(const std::string& domain) const
{
  return this->Internal->GraphIsDirectedInfo[domain];
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::CreateNode(const std::string& nodeType,
                                  double x, double y, double z,
                                  int defaultEdgeAttr)
{
  int id = this->Internal->CurrNodeId;
  std::ostringstream ostr;
  ostr << nodeType << '_' << id;

  double eventPositions[4] = {x, y, x, y};
  return this->CreateNode(id, nodeType,  ostr.str(), x, y, z, eventPositions,
                          defaultEdgeAttr);
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::CreateNode(const std::string& nodeType,
                                  const std::string& nodeLabel,
                                  double x, double y, double z,
                                  double (&eventPositions)[4],
                                  int defaultEdgeAttr,
                                  unsigned int groupId,
                                  double st, double et,
                                  unsigned int sf, unsigned int ef,
                                  double spatialX, double spatialY,
                                  vtkIdType eventId)
{
  return this->CreateNode(this->Internal->CurrNodeId, nodeType, nodeLabel,
                          x, y, z, eventPositions, defaultEdgeAttr, groupId,
                          st, et, sf, ef, spatialX, spatialY, eventId);
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::CreateNode(int id,
                                  const std::string& nodeType,
                                  const std::string& nodeLabel,
                                  double x, double y, double z,
                                  double (&eventPositions)[4],
                                  int defaultEdgeAttr,
                                  unsigned int groupId,
                                  double st, double et,
                                  unsigned int sf, unsigned int ef,
                                  double spatialX, double spatialY,
                                  vtkIdType eventId)
{
  int typeId = vpEventConfig::GetIdFromString(nodeType.c_str());
  if (typeId == -1)
    {
    // Type string not valid
    return -1;
    }

  this->Internal->CurrNodeId = std::max(id + 1, this->Internal->CurrNodeId);

  this->Internal->MasterGraph->AddVertex(id);

  this->Internal->MasterPoints->InsertNextPoint(x, y, z);
  this->Internal->MasterPoints->Modified();

  this->Internal->NodeDefaultEdgeAttrs->InsertNextValue(defaultEdgeAttr);
  this->Internal->NodeLabels->InsertNextValue(nodeLabel.c_str());
  this->Internal->NodeTypes->InsertNextValue(nodeType.c_str());
  this->Internal->NodeSelected->InsertNextValue(0);
  this->Internal->GroupIds->InsertNextValue(groupId);
  this->Internal->StartTime->InsertNextValue(st);
  this->Internal->EndTime->InsertNextValue(et);
  this->Internal->StartFrame->InsertNextValue(sf);
  this->Internal->EndFrame->InsertNextValue(ef);
  this->Internal->EventId->InsertNextValue(eventId);
  this->Internal->NodeVirtualType->InsertNextValue(eventId != -1 ? 0 : 1);
  this->Internal->CachedPositions->InsertNextTuple3(x, y, z);
  this->Internal->SpatialPositions->InsertNextTuple3(spatialX, spatialY, 0.0);
  this->Internal->NormalizedSpatialPositions->InsertNextTuple3(0.0, 0.0, 0.0);
  this->Internal->NormalizedTemporalPositions->InsertNextTuple3(0.0, 0.0, 0.0);
  this->Internal->StartPositions->InsertNextTuple2(
        eventPositions[0], eventPositions[1]);
  this->Internal->EndPositions->InsertNextTuple2(
        eventPositions[2], eventPositions[3]);

  // Add vertex to undirected graphs
  vtkInternal::UndirectedGraphs::iterator udGraphItr =
    this->Internal->DomainUndirectedGraphs.begin();

  for (; udGraphItr != this->Internal->DomainUndirectedGraphs.end();
       ++udGraphItr)
    {
    if (udGraphItr->second == this->Internal->MasterGraph)
      {
      continue;
      }

    udGraphItr->second->AddVertex();
    }

  // Add vertex to directed graphs
  vtkInternal::DirectedGraphs::iterator dGraphItr =
    this->Internal->DomainDirectedGraphs.begin();

  for (; dGraphItr != this->Internal->DomainDirectedGraphs.end();
       ++dGraphItr)
    {
    dGraphItr->second->AddVertex();
    }

  this->Modified();

  // Notify others
  this->InvokeEvent(vtkCommand::UpdateDataEvent);

  return id;
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::CreateEdge(
  int parent, int child,
  const std::string& domain, vtkIntArray* parentAttrs, vtkIntArray* childAttrs)
{
  return this->CreateEdge(this->Internal->CurrentEdgeIds[domain],
                          parent, child, domain,
                          parentAttrs, childAttrs);
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::CreateEdge(
  int edgeId, int parent, int child,
  const std::string& domain, vtkIntArray* parentAttrs, vtkIntArray* childAttrs)
{
  if (domain.compare(vpMultiGraphModel::NoneDomain) == 0)
    {
    return -1;
    }

  // Parent and child are pedigree ids.
  vtkIdType pv = this->Internal->MasterGraph->FindVertex(parent);
  vtkIdType cv = this->Internal->MasterGraph->FindVertex(child);

  if (pv == -1 || cv == -1)
    {
    return -1;
    }

  vpPrimitiveConfig primitiveConfig;
  vpPrimitiveConfig::vpPrimitiveType type =
    primitiveConfig.getPrimitiveTypeByName(qtString(domain));

  if (type.Id == -1)
    {
    return -1;
    }

  int& currId = this->Internal->CurrentEdgeIds[domain];
  currId = std::max(edgeId + 1, currId);

  vtkGraph* graph = 0;

  if (type.Directed)
    {
    vtkInternal::DirectedGraphs::iterator dGraphItr =
      this->Internal->DomainDirectedGraphs.find(domain);
    if (dGraphItr != this->Internal->DomainDirectedGraphs.end())
      {
      dGraphItr->second->AddEdge(pv, cv);
      graph = dGraphItr->second;
      }
    else
      {
      vtkSmartPointer<vtkMutableDirectedGraph> newGraph =
        vtkSmartPointer<vtkMutableDirectedGraph>::New();

      this->Internal->AddNodes(newGraph);
      this->Internal->AddPositionArrays(newGraph);
      this->Internal->AddEdgeArrays(newGraph);

      this->Internal->DomainDirectedGraphs.insert(
        std::pair<std::string, vtkSmartPointer<vtkMutableDirectedGraph> >(
          domain, newGraph));

      newGraph->AddEdge(pv, cv);
      graph = newGraph;
      }
    }
  else
    {
    vtkInternal::UndirectedGraphs::iterator udGraphItr =
      this->Internal->DomainUndirectedGraphs.find(domain);
    if (udGraphItr != this->Internal->DomainUndirectedGraphs.end())
      {
      udGraphItr->second->AddEdge(pv, cv);
      graph = udGraphItr->second;
      }
    else
      {
      vtkSmartPointer<vtkMutableUndirectedGraph> newGraph =
        vtkSmartPointer<vtkMutableUndirectedGraph>::New();

      this->Internal->AddNodes(newGraph);
      this->Internal->AddPositionArrays(newGraph);
      this->Internal->AddEdgeArrays(newGraph);

      this->Internal->DomainUndirectedGraphs.insert(
        std::pair<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >(
          domain, newGraph));

      newGraph->AddEdge(pv, cv);
      graph = newGraph;
      }
    }

  vtkAbstractArray* ids = graph->GetEdgeData()->GetPedigreeIds();
  ids->InsertVariantValue(ids->GetMaxId() + 1, edgeId);

  vtkAbstractArray* sel = graph->GetEdgeData()->GetArray("Selected");
  vtkCharArray::SafeDownCast(sel)->InsertNextValue(0);

  vtkSmartPointer<vtkIntArray> parentAttrsSp = parentAttrs;
  vtkSmartPointer<vtkIntArray> childAttrsSp = childAttrs;

  // Supply default attributes if none were supplied
  if (!parentAttrs)
    {
    int val = this->Internal->NodeDefaultEdgeAttrs->GetValue(pv);
    if (val != -1)
      {
      parentAttrsSp = vtkSmartPointer<vtkIntArray>::New();
      parentAttrsSp->InsertNextValue(val);
      }
    }

  if (!childAttrs)
    {
    int val = this->Internal->NodeDefaultEdgeAttrs->GetValue(cv);
    if (val != -1)
      {
      childAttrsSp = vtkSmartPointer<vtkIntArray>::New();
      childAttrsSp->InsertNextValue(val);
      }
    }

  this->Internal->AddEdgeAttrs(graph, parentAttrsSp, childAttrsSp);

  // Notify others
  this->InvokeEvent(vtkCommand::UpdateDataEvent);

  return edgeId;
}

//-----------------------------------------------------------------------------
vtkGraph* vpMultiGraphModel::CreateGraph(const std::string &domain)
{
  vtkGraph* graph = NULL;

  vpPrimitiveConfig primitiveConfig;
  vpPrimitiveConfig::vpPrimitiveType type =
    primitiveConfig.getPrimitiveTypeByName(qtString(domain));

  if (type.Id == -1)
    {
    return graph;
    }

  if (type.Directed)
    {
    vtkSmartPointer<vtkMutableDirectedGraph> newGraph =
      vtkSmartPointer<vtkMutableDirectedGraph>::New();

    this->Internal->AddNodes(newGraph);
    this->Internal->AddPositionArrays(newGraph);
    this->Internal->AddEdgeArrays(newGraph);

    this->Internal->DomainDirectedGraphs.insert(
      std::pair<std::string, vtkSmartPointer<vtkMutableDirectedGraph> >(
        domain, newGraph));

    graph = newGraph;
    }
  else
    {
    vtkSmartPointer<vtkMutableUndirectedGraph> newGraph =
      vtkSmartPointer<vtkMutableUndirectedGraph>::New();

    this->Internal->AddNodes(newGraph);
    this->Internal->AddPositionArrays(newGraph);
    this->Internal->AddEdgeArrays(newGraph);

    this->Internal->DomainUndirectedGraphs.insert(
      std::pair<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >(
        domain, newGraph));

    graph = newGraph;
    }

  return graph;
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::MoveNode(NodePositionType positionType,
                                 int id, double x, double y, double z,
                                 bool overrideLock)
{
  if (this->LockPositionOfNode && !overrideLock)
    {
    return;
    }

  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return;
    }

  vtkDoubleArray* array = this->Internal->GetPositionArray(positionType);
  array->SetTuple3(index, x, y, z);

  if (positionType == vpMultiGraphModel::NPT_Spatial &&
      this->GetNodeEventId(id) == -1)
      {
        this->SetStartPosition(id, x, y);
        this->SetEndPosition(id, x, y);
      }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::GetNodePosition(NodePositionType positionType, int id,
                                        double& x, double& y, double& z)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return;
    }

  vtkDoubleArray* array = this->Internal->GetPositionArray(positionType);

  double* pos = array->GetTuple3(index);
  x = pos[0];
  y = pos[1];
  z = pos[2];
}

//-----------------------------------------------------------------------------
std::string vpMultiGraphModel::GetNodeLabel(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return std::string();
    }

  return this->Internal->NodeLabels->GetValue(index);
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::GetNodeVirtualType(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return -1;
    }

  return this->Internal->NodeVirtualType->GetValue(index);
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::GetNodeDefaultEdgeAttr(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return -1;
    }

  return this->Internal->NodeDefaultEdgeAttrs->GetValue(index);
}

//-----------------------------------------------------------------------------
double vpMultiGraphModel::GetNodeStartTime(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return vgTimeStamp::InvalidTime();
    }

  return this->Internal->StartTime->GetValue(index);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetNodeStartTime(int id, double time)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    vtkErrorMacro("<< Invalid vertex id");
    return;
    }

  return this->Internal->StartTime->SetTuple1(index, time);
}

//-----------------------------------------------------------------------------
double vpMultiGraphModel::GetNodeEndTime(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return vgTimeStamp::InvalidTime();
    }

  return this->Internal->EndTime->GetValue(index);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetNodeEndTime(int id, double time)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    vtkErrorMacro("<< Invalid vertex id");
    return;
    }

  this->Internal->EndTime->SetTuple1(index, time);
}

//-----------------------------------------------------------------------------
unsigned int vpMultiGraphModel::GetNodeStartFrame(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return vgTimeStamp::InvalidTime();
    }

  return this->Internal->StartFrame->GetValue(index);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetNodeStartFrame(int id, unsigned int frame)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    vtkErrorMacro("<< Invalid vertex id");
    return;
    }

  this->Internal->StartFrame->SetTuple1(index, frame);
}

//-----------------------------------------------------------------------------
unsigned int vpMultiGraphModel::GetNodeEndFrame(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return vgTimeStamp::InvalidTime();
    }

  return this->Internal->EndFrame->GetValue(index);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetNodeEndFrame(int id, unsigned int frame)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    vtkErrorMacro("<< Invalid vertex id");
    return;
    }

  this->Internal->EndFrame->SetTuple1(index, frame);
}

//-----------------------------------------------------------------------------
unsigned int vpMultiGraphModel::GetGroupId(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return vgTimeStamp::InvalidTime();
    }

  return this->Internal->GroupIds->GetValue(index);
}

//-----------------------------------------------------------------------------
vtkIdType vpMultiGraphModel::GetNodeEventId(int id)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    return -1;
    }
  return this->Internal->EventId->GetValue(index);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::GetSpatialPosition(int id, double &x, double &y)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    vtkErrorMacro("<< Invalid vertex id");
    return;
    }

  double* position = this->Internal->SpatialPositions->GetTuple3(index);
  x = position[0];
  y = position[1];
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetSpatialPosition(int id, double x, double y)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index != -1)
    {
    this->Internal->SpatialPositions->SetTuple3(index, x, y, 0.0);
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::GetCachedPosition(int id, double &x, double &y,
                                          double &z)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    vtkErrorMacro("<< Invalid vertex id");
    return;
    }

  double* position = this->Internal->CachedPositions->GetTuple3(index);
  x = position[0];
  y = position[1];
  z = position[2];
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetCachedPosition(int id, double x, double y, double z)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index != -1)
    {
    this->Internal->CachedPositions->SetTuple3(index, x, y, z);
    }
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::GetStartPosition(int id, double &x, double &y)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    vtkErrorMacro("<< Invalid vertex id");
    return;
    }

  double* position = this->Internal->StartPositions->GetTuple2(index);
  x = position[0];
  y = position[1];
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetStartPosition(int id, double x, double y)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index != -1)
    {
    this->Internal->StartPositions->SetTuple2(index, x, y);
    }
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::GetEndPosition(int id, double &x, double &y)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index == -1)
    {
    vtkErrorMacro("<< Invalid vertex id");
    return;
    }

  double* position = this->Internal->EndPositions->GetTuple2(index);
  x = position[0];
  y = position[1];
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetEndPosition(int id, double x, double y)
{
  vtkIdType index = this->Internal->MasterGraph->FindVertex(id);
  if (index != -1)
    {
    this->Internal->EndPositions->SetTuple2(index, x, y);
    }
}

//-----------------------------------------------------------------------------
vtkIntArray* vpMultiGraphModel::GetEdgeParentAttributes(
  int id, const std::string& domain)
{
  return this->Internal->GetEdgeAttrs(domain, id, "EdgeParentAttrs");
}

//-----------------------------------------------------------------------------
vtkIntArray* vpMultiGraphModel::GetEdgeChildAttributes(
  int id, const std::string& domain)
{
  return this->Internal->GetEdgeAttrs(domain, id, "EdgeChildAttrs");
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetEdgeParentAttributes(int id,
                                                const std::string& domain,
                                                vtkIntArray* array)
{
  this->Internal->SetEdgeAttrs(domain, id, array, "EdgeParentAttrs");
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetEdgeChildAttributes(int id,
                                               const std::string& domain,
                                               vtkIntArray* array)
{
  this->Internal->SetEdgeAttrs(domain, id, array, "EdgeChildAttrs");
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::DeleteNodes(vtkIdTypeArray* ids)
{
  // Convert to vertex indices
  for (vtkIdType i = 0, end = ids->GetNumberOfTuples(); i != end; ++i)
    {
    vtkIdType index =
      this->Internal->MasterGraph->FindVertex(
        static_cast<int>(ids->GetValue(i)));
    if (index == -1)
      {
      return;
      }
    ids->SetValue(i, index);
    }

  this->Internal->MasterGraph->RemoveVertices(ids);
  this->Internal->MasterPoints->Modified();

  // Remove vertex from undirected graphs
  vtkInternal::UndirectedGraphs::iterator udGraphItr =
    this->Internal->DomainUndirectedGraphs.begin();

  for (; udGraphItr != this->Internal->DomainUndirectedGraphs.end();
       ++udGraphItr)
    {
    if (udGraphItr->second == this->Internal->MasterGraph)
      {
      continue;
      }

    udGraphItr->second->SetPoints(0);
    this->Internal->RemovePositionArrays(udGraphItr->second);
    udGraphItr->second->RemoveVertices(ids);
    this->Internal->AddPositionArrays(udGraphItr->second);
    udGraphItr->second->SetPoints(this->Internal->MasterPoints);
    }

  // Remove vertex from directed graphs
  vtkInternal::DirectedGraphs::iterator dGraphItr =
    this->Internal->DomainDirectedGraphs.begin();

  for (; dGraphItr != this->Internal->DomainDirectedGraphs.end();
       ++dGraphItr)
    {
    dGraphItr->second->SetPoints(0);
    this->Internal->RemovePositionArrays(dGraphItr->second);
    dGraphItr->second->RemoveVertices(ids);
    this->Internal->AddPositionArrays(dGraphItr->second);
    dGraphItr->second->SetPoints(this->Internal->MasterPoints);
    }

  this->Modified();

  // Notify others
  this->InvokeEvent(vtkCommand::UpdateDataEvent);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::DeleteEdges(vtkIdTypeArray* ids,
                                    const std::string& domain)
{
  vtkGraph* graph = this->GetGraph(domain);
  if (!graph)
    {
    return;
    }

  // Convert to edge indices
  for (vtkIdType i = 0, end = ids->GetNumberOfTuples(); i != end; ++i)
    {
    vtkIdType index =
      graph->GetEdgeData()->GetPedigreeIds()->LookupValue(
        static_cast<int>(ids->GetValue(i)));
    if (index == -1)
      {
      return;
      }
    ids->SetValue(i, index);
    }

  if (vtkMutableDirectedGraph* dg =
        vtkMutableDirectedGraph::SafeDownCast(graph))
    {
    dg->RemoveEdges(ids);
    }
  else if (vtkMutableUndirectedGraph* ug =
             vtkMutableUndirectedGraph::SafeDownCast(graph))
    {
    ug->RemoveEdges(ids);
    }
  else
    {
    return;
    }

  this->Modified();

  this->InvokeEvent(vtkCommand::UpdateDataEvent);
}

//-----------------------------------------------------------------------------
bool vpMultiGraphModel::HasEdge(int id, const std::string& domain)
{
  if (vtkGraph* graph = this->GetGraph(domain))
    {
    if (vtkAbstractArray* ids = graph->GetEdgeData()->GetPedigreeIds())
      {
      return ids->LookupValue(id) != -1;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::GetNodeId(vtkIdType index)
{
  if (vtkAbstractArray* ids =
        this->Internal->MasterGraph->GetVertexData()->GetPedigreeIds())
    {
    return ids->GetVariantValue(index).ToInt();
    }
  return -1;
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::GetEdgeId(vtkIdType index,
                                 const std::string& domain)
{
  if (vtkGraph* graph = this->GetGraph(domain))
    {
    if (vtkAbstractArray* ids = graph->GetEdgeData()->GetPedigreeIds())
      {
      return ids->GetVariantValue(index).ToInt();
      }
    }
  return -1;
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetNodeType(int nodeId,
                                    const std::string& nodeType,
                                    const std::string& nodeLabel,
                                    int defaultEdgeAttr)
{
  vtkIdType id = this->Internal->MasterGraph->FindVertex(nodeId);
  if (id == -1)
    {
    return;
    }

  if (!nodeLabel.empty())
    {
    this->Internal->NodeLabels->SetValue(id, nodeLabel.c_str());
    }
  this->Internal->NodeDefaultEdgeAttrs->SetValue(id, defaultEdgeAttr);

  this->Internal->NodeTypes->SetValue(id, nodeType.c_str());
  this->Internal->MasterGraph->Modified();
  this->InvokeEvent(vtkCommand::UpdateDataEvent);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetSelectedNodes(vtkIdTypeArray* selected)
{
  // Clear previous selection
  int size = this->Internal->NodeSelected->GetNumberOfTuples();
  void* begin = this->Internal->NodeSelected->WriteVoidPointer(0, size);
  memset(begin, 0, size);
  this->AddSelectedNodes(selected);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::SetSelectedEdges(vtkIdTypeArray* selected,
                                         const std::string& domain)
{
  // Clear previous selection
  for (vtkInternal::UndirectedGraphs::iterator i =
         this->Internal->DomainUndirectedGraphs.begin(), end =
         this->Internal->DomainUndirectedGraphs.end(); i != end; ++i)
    {
    if (vtkCharArray* sel =
          vtkCharArray::SafeDownCast(
            i->second->GetEdgeData()->GetArray("Selected")))
      {
      int size = sel->GetNumberOfTuples();
      void* begin = sel->WriteVoidPointer(0, size);
      memset(begin, 0, size);
      sel->Modified();
      }
    }

  this->AddSelectedEdges(selected, domain);
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::AddSelectedNodes(vtkIdTypeArray* selected)
{
  if (!selected)
    {
    return;
    }

  for (vtkIdType i = 0, end = selected->GetNumberOfTuples(); i != end; ++i)
    {
    vtkIdType id =
      this->Internal->MasterGraph->FindVertex(
        static_cast<int>(selected->GetValue(i)));
    if (id != -1)
      {
      this->Internal->NodeSelected->SetValue(id, 1);
      }
    }

  this->Internal->NodeSelected->Modified();
}

//-----------------------------------------------------------------------------
void vpMultiGraphModel::AddSelectedEdges(vtkIdTypeArray* selected,
                                         const std::string& domain)
{
  for (vtkInternal::DirectedGraphs::iterator i =
         this->Internal->DomainDirectedGraphs.begin(), end =
         this->Internal->DomainDirectedGraphs.end(); i != end; ++i)
    {
    if (vtkCharArray* sel =
          vtkCharArray::SafeDownCast(
            i->second->GetEdgeData()->GetArray("Selected")))
      {
      int size = sel->GetNumberOfTuples();
      void* begin = sel->WriteVoidPointer(0, size);
      memset(begin, 0, size);
      sel->Modified();
      }
    }

  if (!selected)
    {
    return;
    }

  if (vtkGraph* graph = this->GetGraph(domain))
    {
    if (vtkCharArray* sel =
          vtkCharArray::SafeDownCast(
            graph->GetEdgeData()->GetArray("Selected")))
      {
      for (vtkIdType i = 0, end = selected->GetNumberOfTuples(); i != end; ++i)
        {
        int id = static_cast<int>(selected->GetValue(i));
        vtkIdType index =
          graph->GetEdgeData()->GetPedigreeIds()->LookupValue(id);
        if (index != -1)
          {
          sel->SetValue(index, 1);
          }
        }
      sel->Modified();
      }
    }
}

//-----------------------------------------------------------------------------
vtkIdTypeArray* vpMultiGraphModel::GetSelectedNodes()
{
  this->Internal->SelectedNodesHelper->Reset();
  for (vtkIdType i = 0, end = this->Internal->NodeSelected->GetNumberOfTuples();
       i != end; ++i)
    {
    if (this->Internal->NodeSelected->GetValue(i))
      {
      this->Internal->SelectedNodesHelper->InsertNextValue(this->GetNodeId(i));
      }
    }
  return this->Internal->SelectedNodesHelper;
}

//-----------------------------------------------------------------------------
vtkIdTypeArray* vpMultiGraphModel::GetSelectedEdges(const std::string& domain)
{
  this->Internal->SelectedEdgesHelper->Reset();
  if (vtkGraph* graph = this->GetGraph(domain))
    {
    if (vtkCharArray* sel =
          vtkCharArray::SafeDownCast(
            graph->GetEdgeData()->GetArray("Selected")))
      {
      for (vtkIdType i = 0, end = sel->GetNumberOfTuples(); i != end; ++i)
        {
        if (sel->GetValue(i))
          {
          this->Internal->SelectedEdgesHelper->InsertNextValue(
            this->GetEdgeId(i, domain));
          }
        }
      }
    }
  return this->Internal->SelectedEdgesHelper;
}

//-----------------------------------------------------------------------------
vtkGraph* vpMultiGraphModel::GetGraph(const std::string& domain) const
{
  vtkInternal::UndirectedGraphs::const_iterator ugItr =
    this->Internal->DomainUndirectedGraphs.begin();

  for (; ugItr != this->Internal->DomainUndirectedGraphs.end(); ++ugItr)
    {
    if (ugItr->first.compare(domain) == 0)
      {
      return ugItr->second;
      }
    }

  vtkInternal::DirectedGraphs::const_iterator gItr =
    this->Internal->DomainDirectedGraphs.begin();

  for (; gItr != this->Internal->DomainDirectedGraphs.end(); ++gItr)
    {
    if (gItr->first.compare(domain) == 0)
      {
      return gItr->second;
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
std::map<std::string, vtkSmartPointer<vtkMutableDirectedGraph> >
vpMultiGraphModel::GetAllDirectedGraphs() const
{
  return this->Internal->DomainDirectedGraphs;
}

//-----------------------------------------------------------------------------
std::map<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >
vpMultiGraphModel::GetAllUndirectedGraphs() const
{
  return this->Internal->DomainUndirectedGraphs;
}

//-----------------------------------------------------------------------------
std::map<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >
vpMultiGraphModel::GetAllUndirectedGraphsWithEdges() const
{
  typedef std::map<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >
    Graphs;
  typedef std::pair<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >
    GraphElem;

  Graphs outGraphs;

  Graphs::const_iterator uConstItr =
    this->Internal->DomainUndirectedGraphs.begin();

  // Ignore master graph (since it cannot and should not have any edges)
  for (; uConstItr != this->Internal->DomainUndirectedGraphs.end(); ++uConstItr)
    {
    if (uConstItr->second != this->Internal->MasterGraph)
      {
      outGraphs.insert(GraphElem(uConstItr->first, uConstItr->second));
      }
    }

  return outGraphs;
}

//-----------------------------------------------------------------------------
int vpMultiGraphModel::Update(const vtkVgTimeStamp& vtkNotUsed(timeStamp),
                              const vtkVgTimeStamp* vtkNotUsed(referenceFrameTimeStamp))
{
  // \TODO Implement this
  return 0;
}

//-----------------------------------------------------------------------------
unsigned long vpMultiGraphModel::GetUpdateTime()
{
  // \TODO Implement this
  return 0;
}
