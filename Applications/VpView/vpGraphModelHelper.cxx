// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpGraphModelHelper.h"

#include "vpEventConfig.h"
#include "vpGraphSortAlgorithms.h"
#include "vpMultiGraphModel.h"
#include "vpMultiGraphRepresentation.h"

#include <qtStlUtil.h>

#include <vtkVgEvent.h>

#include <vtkIdTypeArray.h>
#include <vtkVertexListIterator.h>

#include <QDebug>

#include <algorithm>
#include <sstream>

//-----------------------------------------------------------------------------
vpGraphModelHelper::vpGraphModelHelper(vpMultiGraphModel* model) :
  QObject(), GraphModel(model), ImportOrderIndex(0)
{
  this->NodeDefaultEdgeAttr = this->NodeStartAttr = this->NodeStopAttr = -1;
}

//-----------------------------------------------------------------------------
int vpGraphModelHelper::createNode(double x, double y)
{
  std::string nodeType = stdString(this->NodeType);
  int nodeId = this->GraphModel->CreateNode(nodeType,
                                            x, y, 0.0,
                                            this->getEdgeAttrForType(nodeType));

  if (nodeId == -1)
    {
    qDebug() << "Failed to create node";
    return -1;
    }

  emit this->nodeCreated(this->NodeType, nodeId);
  return nodeId;
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::createNodeExternal(int id,
                                            double x, double y,
                                            const std::string& type,
                                            const std::string& label)
{
  double eventPositions[4] = {x, y, x, y};
  id = this->GraphModel->CreateNode(id, type, label, x, y, 0.0, eventPositions);

  if (id == -1)
    {
    qDebug() << "Failed to create node";
    return;
    }

  emit this->nodeCreated(qtString(type), id);
}

//-----------------------------------------------------------------------------
int vpGraphModelHelper::createNodeExternal(double x, double y,
                                           const std::string& type,
                                           const std::string& label)
{
  double eventPositions[4] = {x, y, x, y};
  int id = this->GraphModel->CreateNode(type, label, x, y, 0.0, eventPositions);

  if (id == -1)
    {
    qDebug() << "Failed to create node";
    return id;
    }

  emit this->nodeCreated(qtString(type), id);
  return id;
}

//-----------------------------------------------------------------------------
int vpGraphModelHelper::createNodeExternal(double x, double y,
  const std::string& type, const std::string& label,
  double (&eventPositions)[4],
  int defaultEdgeAttr,
  unsigned int groupId,
  double startTime, double endTime,
  unsigned int startFrame, unsigned int endFrame,
  double spatialX, double spatialY,
  vtkIdType eventId)

{
  int id = this->GraphModel->CreateNode(type, label, x, y, 0.0,
             eventPositions, defaultEdgeAttr, groupId, startTime,
             endTime, startFrame, endFrame,
             spatialX, spatialY, eventId);

  if (id == -1)
    {
    qDebug() << "Failed to create node";
    return id;
    }

  emit this->nodeCreated(qtString(type), id);
  return id;
}

//-----------------------------------------------------------------------------
int vpGraphModelHelper::createNodeExternal(int id,
  double x, double y,
  const std::string& type, const std::string& label,
  double (&eventPositions)[4],
  int defaultEdgeAttr,
  unsigned int groupId,
  double startTime, double endTime,
  unsigned int startFrame, unsigned int endFrame,
  double spatialX, double spatialY,
  vtkIdType eventId)

{
  id = this->GraphModel->CreateNode(id, type, label, x, y, 0.0,
         eventPositions, defaultEdgeAttr, groupId, startTime, endTime,
         startFrame, endFrame, spatialX, spatialY, eventId);

  if (id == -1)
    {
    qDebug() << "Failed to create node";
    return id;
    }

  emit this->nodeCreated(qtString(type), id);
  return id;
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::createEdge(
  int parent, int child, const std::string& domain,
  vtkIntArray* parentAttrs, vtkIntArray* childAttrs)
{
  int edgeId
    = this->GraphModel->CreateEdge(parent, child, domain,
                                   parentAttrs, childAttrs);

  if (edgeId < 0)
    {
    qDebug() << "Failed to create edge";
    return;
    }

  emit this->edgeCreated(qtString(domain), edgeId);
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::createEdgeExternal(
  int edgeId, int parent, int child, const std::string& domain,
  vtkIntArray* parentAttrs, vtkIntArray* childAttrs)
{
  edgeId
    = this->GraphModel->CreateEdge(edgeId, parent, child, domain,
                                   parentAttrs, childAttrs);

  if (edgeId < 0)
    {
    qDebug() << "Failed to create edge";
    return;
    }

  emit this->edgeCreated(qtString(domain), edgeId);
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::setNodeType(int nodeId,
                                     const std::string& nodeType,
                                     const std::string& nodeLabel,
                                     int defaultEdgeAttr)
{
  this->GraphModel->SetNodeType(nodeId, nodeType, nodeLabel, defaultEdgeAttr);

  // Update the spatial position in case the default edge attr changed
  std::map<int, vtkVgEvent*>::iterator itr = this->NodeIdToEvent.find(nodeId);
  if (itr != this->NodeIdToEvent.end())
    {
    double x, y;
    this->getEventSpatialPoint(itr->second, defaultEdgeAttr, x, y);
    this->GraphModel->SetSpatialPosition(nodeId, x, y);
    }
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::setNodePosition(int nodeId,
                                         vpMultiGraphModel::NodePositionType
                                           positionType,
                                         double x, double y)
{
  this->GraphModel->MoveNode(positionType, nodeId, x, y, 0.0);
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::deleteNodes(vtkIdTypeArray* ids)
{
  this->GraphModel->DeleteNodes(ids);
  emit this->nodesDeleted();
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::deleteEdges(vtkIdTypeArray* ids,
                                     const QString& domain)
{
  this->GraphModel->DeleteEdges(ids, stdString(domain));
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::selectNodes(vtkIdTypeArray* ids, bool append)
{
  if (append)
    {
    this->GraphModel->AddSelectedNodes(ids);
    }
  else
    {
    this->GraphModel->SetSelectedNodes(ids);
    }
  this->GraphModel->SetSelectedEdges(0, std::string());
  emit this->selectedNodesChanged();
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::selectEdges(vtkIdTypeArray* ids,
                                     const QString& domain, bool append)
{
  this->GraphModel->SetSelectedNodes(0);
  if (append)
    {
    this->GraphModel->AddSelectedEdges(ids, stdString(domain));
    }
  else
    {
    this->GraphModel->SetSelectedEdges(ids, stdString(domain));
    }
  emit this->selectedEdgesChanged(domain);
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::createEventNodes(
  const std::vector<vtkVgEvent*>& events, double centerX, double y)
{
  if (events.empty())
    {
    return;
    }

  const double spacing = 0.3;

  // Space the nodes evenly apart on a horizontal line
  double width = (events.size() - 1) * spacing;
  double x = centerX - 0.5 * width;

  for (size_t i = 0, size = events.size(); i < size; ++i)
    {
    vtkVgEvent* event = events[i];
    vtkIdType id = event->GetId();
    std::string nodeType =
      vpEventConfig::GetStringFromId(event->GetActiveClassifierType());

    std::ostringstream ostr;
    ostr << "Event: " << nodeType << '_' << id;

    int attr = this->getEdgeAttrForType(nodeType);

    double point[2];
    this->getEventSpatialPoint(event, attr, point[0], point[1]);

    double eventPositions[4];
    event->GetDisplayPosition(event->GetStartFrame(), &eventPositions[0]);
    event->GetDisplayPosition(event->GetEndFrame(), &eventPositions[2]);

    int nodeId = this->createNodeExternal(x, y,
                   nodeType, ostr.str(), eventPositions,
                   attr, this->ImportOrderIndex,
                   event->GetStartFrame().GetTime(),
                   event->GetEndFrame().GetTime(),
                   event->GetStartFrame().GetFrameNumber(),
                   event->GetEndFrame().GetFrameNumber(),
                   point[0], point[1], id);

    this->NodeIdToEvent[nodeId] = event;
    x += spacing;
    }

  this->ImportOrderIndex += 1;
}

//-----------------------------------------------------------------------------
int vpGraphModelHelper::getNodeEventId(int nodeId)
{
  std::map<int, vtkVgEvent*>::iterator itr = this->NodeIdToEvent.find(nodeId);
  if (itr == this->NodeIdToEvent.end())
    {
    return -1;
    }
  return itr->second->GetId();
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::clear()
{
  this->GraphModel->Reset();
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::setNewNodeType(const QString& type)
{
  this->NodeType = type;
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::setNewNodeDefaultEdgeAttr(int attr)
{
  this->NodeDefaultEdgeAttr = attr;
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::autoCreateEdges(const QString &domain, bool selected,
                                         vpMultiGraphModel::NodePositionType
                                           posType)
{
  std::string dom = stdString(domain);

  if ((dom.compare("before") != 0) &&
      (dom.compare("adjacent") != 0))
    {
    return;
    }

  vtkGraph* graph = this->GraphModel->GetGraph(dom);

  if (!graph)
    {
    graph = this->GraphModel->CreateGraph(dom);
    }

  std::vector<vtkIdType> vertexIds;
  std::map<vtkIdType, vtkIdType> vertexIdToIndex;

  vtkSmartPointer<vtkVertexListIterator> vItr =
    vtkSmartPointer<vtkVertexListIterator>::New();
  graph->GetVertices(vItr);

  if (!selected)
    {
    while (vItr->HasNext())
      {
      vtkIdType index = vItr->Next();
      vtkIdType id = this->GraphModel->GetNodeId(index);

      vertexIdToIndex[id] = index;
      vertexIds.push_back(id);
      }
    }
  else
    {
    vtkIdTypeArray* selectedNodes = this->GraphModel->GetSelectedNodes();
    while (vItr->HasNext())
      {
      vtkIdType index = vItr->Next();
      vtkIdType id = this->GraphModel->GetNodeId(index);

      if (selectedNodes->LookupValue(id) != -1)
        {
        vertexIdToIndex[id] = index;
        vertexIds.push_back(id);
        }
      }
    }

  if (dom.compare("before") == 0)
    {
    SortVerticesByXPosDesc sortAlg(this->GraphModel, posType);
    std::sort(vertexIds.begin(), vertexIds.end(), sortAlg);

    for (size_t i = 0; i < vertexIds.size(); ++i)
      {
      for (size_t j = i + 1; j < vertexIds.size(); ++j)
        {
        // If the edge exists between two nodes skip adding another edge.
        if (graph->GetNumberOfEdges() &&
            graph->GetEdgeId(vertexIdToIndex[vertexIds[j]],
                             vertexIdToIndex[vertexIds[i]]) != -1)
          {
          continue;
          }
        int id = this->GraphModel->CreateEdge(vertexIds[j], vertexIds[i], dom);
        emit this->edgeCreated(domain, id);
        }
      }
    } // before
  else if (dom.compare("adjacent") == 0)
    {
    for (size_t i = 0; i < vertexIds.size(); ++i)
      {
      for (size_t j = i + 1; j < vertexIds.size(); ++j)
        {
        // If the edge exists between two nodes skip adding another edge.
        if (graph->GetNumberOfEdges() &&
            graph->GetEdgeId(vertexIdToIndex[vertexIds[i]],
                             vertexIdToIndex[vertexIds[j]]) != -1)
          {
          continue;
          }
        int id = this->GraphModel->CreateEdge(vertexIds[i], vertexIds[j], dom);
        emit this->edgeCreated(domain, id);
        }
      }
    } // adjacent
  else
    {
    // Do nothing for now
    }
}

//-----------------------------------------------------------------------------
int vpGraphModelHelper::getEdgeAttrForType(const std::string& nodeType)
{
  if ((nodeType == "STARTING" || nodeType == "EXIT_VEHICLE") &&
      this->NodeStartAttr != -1)
    {
    return this->NodeStartAttr;
    }
  if ((nodeType == "STOPPING" || nodeType == "ENTER_VEHICLE") &&
      this->NodeStopAttr != -1)
    {
    return this->NodeStopAttr;
    }
  return this->NodeDefaultEdgeAttr;
}

//-----------------------------------------------------------------------------
void vpGraphModelHelper::getEventSpatialPoint(vtkVgEvent* event,
                                              int defaultEdgeAttr,
                                              double& x, double& y)
{
  // Change the event point used depending on the default edge attribute type
  double point[3];
  if (defaultEdgeAttr != -1 && defaultEdgeAttr == this->NodeStopAttr)
    {
    event->GetDisplayPosition(event->GetEndFrame(), point);
    }
  else
    {
    event->GetDisplayPosition(event->GetStartFrame(), point);
    }
  x = point[0];
  y = point[1];
}
