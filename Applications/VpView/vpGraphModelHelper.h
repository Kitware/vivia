/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpGraphModelHelper_h
#define __vpGraphModelHelper_h

#include "vpMultiGraphModel.h"

#include <QObject>

#include <map>
#include <vector>

class vtkIdTypeArray;
class vtkIntArray;
class vtkVgEvent;

//-----------------------------------------------------------------------------
class vpGraphModelHelper : public QObject
{
  Q_OBJECT

public:
  explicit vpGraphModelHelper(vpMultiGraphModel* model);

  void createEventNodes(const std::vector<vtkVgEvent*>& events,
                        double centerX, double y);

  int getNodeEventId(int nodeId);

  int createNode(double x, double y);

  void createNodeExternal(int id,
                          double x, double y,
                          const std::string& type,
                          const std::string& label);

  int createNodeExternal(double x, double y,
                         const std::string& type,
                         const std::string& label);

  int createNodeExternal(double x, double y,
                         const std::string& type, const std::string& label,
                         double (&eventPositions)[4],
                         int defaultEdgeAttr,
                         unsigned int groupId,
                         double startTime, double endTime,
                         unsigned int startFrame, unsigned int endFrame,
                         double spatialX, double spatialY,
                         vtkIdType eventId);

  int createNodeExternal(int id,
                         double x, double y,
                         const std::string& type, const std::string& label,
                         double (&eventPositions)[4],
                         int defaultEdgeAttr,
                         unsigned int groupId,
                         double startTime, double endTime,
                         unsigned int startFrame, unsigned int endFrame,
                         double spatialX, double spatialY,
                         vtkIdType eventId);

  void createEdge(int parent, int child, const std::string& domain,
                  vtkIntArray* parentAttrs, vtkIntArray* childAttrs);

  void createEdgeExternal(int edgeId, int parent, int child,
                          const std::string& domain,
                          vtkIntArray* parentAttrs, vtkIntArray* childAttrs);

  void setNodeType(int nodeId, const std::string& nodeType,
                   const std::string& nodeLabel, int nodeDefaultEdgeAttr);

  void setNodePosition(int nodeId,
                       vpMultiGraphModel::NodePositionType positionType,
                       double x, double y);

  void clear();

  void autoCreateEdges(const QString& domain, bool selected,
                       vpMultiGraphModel::NodePositionType posType);

  // Temporary until have default attr config
  void setStartAttr(int attr) { this->NodeStartAttr = attr; }
  void setStopAttr(int attr)  { this->NodeStopAttr = attr; }

signals:
  void nodeCreated(QString nodeType, int);
  void edgeCreated(QString domain, int);

  void selectedNodesChanged();
  void selectedEdgesChanged(const QString& domain);

  void nodesDeleted();

public slots:
  void setNewNodeType(const QString& type);
  void setNewNodeDefaultEdgeAttr(int attr);

  void deleteNodes(vtkIdTypeArray* ids);
  void deleteEdges(vtkIdTypeArray* ids, const QString& domain);

  void selectNodes(vtkIdTypeArray* ids, bool append = false);
  void selectEdges(vtkIdTypeArray* ids, const QString& domain,
                   bool append = false);

protected:
  int getEdgeAttrForType(const std::string& nodeType);

  void getEventSpatialPoint(vtkVgEvent* event,
                            int defaultEdgeAttr,
                            double& x, double& y);

protected:
  QString NodeType;
  int NodeDefaultEdgeAttr;
  int NodeStartAttr;
  int NodeStopAttr;

  vtkSmartPointer<vpMultiGraphModel> GraphModel;

  std::map<int, vtkVgEvent*> NodeIdToEvent;
  int ImportOrderIndex;
};

#endif // __vpGraphModelHelper_h
