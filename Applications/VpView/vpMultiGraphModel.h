// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpMultiGraphModel_h
#define __vpMultiGraphModel_h

#include <vtkVgModelBase.h>

//
#include <vgTimeStamp.h>

// VTK includes
#include <vtkGraph.h>
#include <vtkMath.h>

// C++ include
#include <map>
#include <string>

class vtkGraph;
class vtkIntArray;
class vtkMutableDirectedGraph;
class vtkMutableUndirectedGraph;

class vpMultiGraphModel : public vtkVgModelBase
{
public:
  enum NodePositionType
    {
    NPT_User,
    NPT_Spatial,
    NPT_NormalizedSpatial,
    NPT_NormalizedTemporal
    };

  vtkVgClassMacro(vpMultiGraphModel);

  vtkTypeMacro(vpMultiGraphModel, vtkVgModelBase);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vpMultiGraphModel* New();

  // Restore initial state
  void Reset();

  /// Set graph type (directed vs undirected)
  int SetGraphIsDirected(const std::string& domain, int val);
  int GetGraphIsDirected(const std::string& domain) const;

  // Create node using internally generated id
  int CreateNode(
    const std::string& nodeType, double x, double y, double z,
    int defaultEdgeAttr = -1);

  // Create node using internally generated id
  int CreateNode(const std::string& nodeType, const std::string& nodeLabel,
    double x, double y, double z,
    double (&eventPositions)[4],
    int defaultEdgeAttr = -1,
    unsigned int groupId = 0,
    double st=vgTimeStamp::InvalidTime(),
    double et=vgTimeStamp::InvalidTime(),
    unsigned int sf=vgTimeStamp::InvalidFrameNumber(),
    unsigned int ef=vgTimeStamp::InvalidFrameNumber(),
    double spatialX = vtkMath::Nan(), double spatialY = vtkMath::Nan(),
    vtkIdType eventId = -1);

  // Create node with desired id
  int CreateNode(int id, const std::string& nodeType,
    const std::string& nodeLabel, double x, double y, double z,
    double (&eventPositions)[4],
    int defaultEdgeAttr = -1,
    unsigned int orderIndex = 0,
    double st=vgTimeStamp::InvalidTime(),
    double et=vgTimeStamp::InvalidTime(),
    unsigned int sf=vgTimeStamp::InvalidFrameNumber(),
    unsigned int ef=vgTimeStamp::InvalidFrameNumber(),
    double spatialX = vtkMath::Nan(), double spatialY = vtkMath::Nan(),
    vtkIdType eventId = -1);

  // Create edge using internally generated id
  int CreateEdge(
    int parent, int child,  const std::string& domain,
    vtkIntArray* parentAttrs = 0, vtkIntArray* childAttrs = 0);

  // Create edge with desired id
  int CreateEdge(
    int edgeId, int parent, int child,
    const std::string& domain, vtkIntArray* parentAttrs = 0,
    vtkIntArray* childAttrs = 0);

  // Create a new graph
  vtkGraph* CreateGraph(const std::string& domain);

  void MoveNode(NodePositionType positionType,
                int id, double x, double y, double z,
                bool overrideLock = false);

  vtkSetMacro(LockPositionOfNode, bool);
  vtkGetMacro(LockPositionOfNode, bool);
  vtkBooleanMacro(LockPositionOfNode, bool);

  // Accessors
  void GetNodePosition(NodePositionType positionType,
                       int id, double& x, double& y, double& z);
  std::string GetNodeLabel(int id);

  int GetNodeVirtualType(int id);

  int GetNodeDefaultEdgeAttr(int id);

  double GetNodeStartTime(int id);
  void SetNodeStartTime(int id, double time);

  double GetNodeEndTime(int id);
  void SetNodeEndTime(int id, double time);

  unsigned int GetNodeStartFrame(int id);
  void SetNodeStartFrame(int id, unsigned int frame);

  unsigned int GetNodeEndFrame(int id);
  void SetNodeEndFrame(int id, unsigned int frame);

  unsigned int GetGroupId(int id);
  vtkIdType GetNodeEventId(int id);

  void GetSpatialPosition(int id, double& x, double& y);
  void SetSpatialPosition(int id, double x, double y);

  void GetCachedPosition(int id, double&x, double& y, double& z);
  void SetCachedPosition(int id, double x, double y, double z);

  void GetStartPosition(int id, double&x, double& y);
  void SetStartPosition(int id, double x, double y);

  void GetEndPosition(int id, double&x, double& y);
  void SetEndPosition(int id, double x, double y);

  vtkIntArray* GetEdgeParentAttributes(int id,
                                       const std::string& domain);
  vtkIntArray* GetEdgeChildAttributes(int id,
                                      const std::string& domain);

  void SetEdgeParentAttributes(int id,
                               const std::string& domain,
                               vtkIntArray* array);

  void SetEdgeChildAttributes(int id,
                              const std::string& domain,
                              vtkIntArray* array);

  void DeleteNodes(vtkIdTypeArray* ids);
  void DeleteEdges(vtkIdTypeArray* ids, const std::string& domain);

  bool HasEdge(int id, const std::string& domain);

  int GetNodeId(vtkIdType index);
  int GetEdgeId(vtkIdType index, const std::string& domain);

  void SetNodeType(int nodeId, const std::string& nodeType,
                   const std::string& nodeLabel = std::string(),
                   int defaultEdgeAttr = -1);

  void SetSelectedNodes(vtkIdTypeArray* selected);
  void SetSelectedEdges(vtkIdTypeArray* selected, const std::string& domain);

  void AddSelectedNodes(vtkIdTypeArray* selected);
  void AddSelectedEdges(vtkIdTypeArray* selected, const std::string& domain);

  vtkIdTypeArray* GetSelectedNodes();
  vtkIdTypeArray* GetSelectedEdges(const std::string& domain);

  vtkGraph* GetGraph(const std::string& domain) const;
  std::map<std::string, vtkSmartPointer<vtkMutableDirectedGraph> >
  GetAllDirectedGraphs() const;
  std::map<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >
  GetAllUndirectedGraphs() const;
  std::map<std::string, vtkSmartPointer<vtkMutableUndirectedGraph> >
  GetAllUndirectedGraphsWithEdges() const;

  virtual int Update(
    const vtkVgTimeStamp& timeStamp,
    const vtkVgTimeStamp* referenceFrameTimeStamp);
  using Superclass::Update;

  /// Return the MTime for the last Update (that did something).
  virtual unsigned long GetUpdateTime();

  static const std::string NoneDomain;

private:
  /// Not implemented.
  vpMultiGraphModel(const vpMultiGraphModel&);

  /// Not implemented.
  void operator=(const vpMultiGraphModel&);

  vpMultiGraphModel();
  virtual ~vpMultiGraphModel();

  bool LockPositionOfNode;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif // __vpMultiGraphModel_h
