/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpMultiGraphRepresentation.h"

#include "vpMultiGraphModel.h"

// VisGUI includes
#include <qtUtil.h>

#include <vgRange.h>

#include <vtkVgColorUtil.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVg2DGraphMapper.h>

#include <vgEventType.h>
#include <vpEventConfig.h>
#include <vpGraphAnimateCallback.h>

// VTK includes
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkDataSetAttributes.h>
#include <vtkGlyphSource2D.h>
#include <vtkGraphToPolyData.h>
#include <vtkHardwareSelector.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkMath.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkStringArray.h>
#include <vtkTransform2D.h>
#include <vtkVertexListIterator.h>

#include <algorithm>

vtkStandardNewMacro(vpMultiGraphRepresentation);

#define vtkCreateMacro(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

typedef double (vpMultiGraphModel::*GetNodeTimeFn)(int);

namespace
{

//-----------------------------------------------------------------------------
struct SortGraphByTime
{
  vpMultiGraphModel* GraphModel;
  GetNodeTimeFn TimeFunc;

  SortGraphByTime(vpMultiGraphModel* graphModel, GetNodeTimeFn timeFunc)
    : GraphModel(graphModel), TimeFunc(timeFunc)
    {
    }

  bool operator ()(vtkIdType lhs, vtkIdType rhs)
    {
    return (this->GraphModel->*this->TimeFunc)(lhs) <
           (this->GraphModel->*this->TimeFunc)(rhs);
    }
};

//----------------------------------------------------------------------------
vpMultiGraphModel::NodePositionType GetPositionTypeForLayout(int layout)
{
  switch (layout)
    {
    case vpMultiGraphRepresentation::SortByStartTime:
      return vpMultiGraphModel::NPT_NormalizedTemporal;
    case vpMultiGraphRepresentation::Spatial:
      return vpMultiGraphModel::NPT_NormalizedSpatial;
    case vpMultiGraphRepresentation::RawSpatial:
      return vpMultiGraphModel::NPT_Spatial;
    default:
      return vpMultiGraphModel::NPT_User;
    }
}

//----------------------------------------------------------------------------
const char* GetPositionArrayNameForLayout(int layout)
{
  switch (layout)
    {
    case vpMultiGraphRepresentation::SortByStartTime:
      return "NormalizedTemporalPositions";
    case vpMultiGraphRepresentation::Spatial:
      return "NormalizedSpatialPositions";
    case vpMultiGraphRepresentation::RawSpatial:
      return "SpatialPositions";
    default:
      return "CachedPositions";
    }
}

}

//----------------------------------------------------------------------------
class vpMultiGraphRepresentation::vtkInternal
{
public:
  typedef vtkSmartPointer<vtkActor> ActorPtr;

  struct GraphEntity
    {
    std::string Key;
    ActorPtr Actor;
    };

  double TimelineXScale;
  double TimelineMinTime;
  unsigned int TimelineMinFrame;
  double TimelineFrameXScale;
  double TimelineXMin;


  vtkInternal(vpMultiGraphRepresentation* parent);
  ~vtkInternal();

  //----------------------------------------------------------------------------
  void ComputeTemporalRange(vpMultiGraphModel* graphModel,
                            GetNodeTimeFn timeFunc, double (&timeRange)[2],
                            int (&ids)[2])
  {
    vtkGraph* graph = graphModel->GetGraph(vpMultiGraphModel::NoneDomain);

    double curr;
    std::vector<vtkIdType> vertexIds;
    double min = vgTimeStamp::InvalidTime();
    double max = vgTimeStamp::InvalidTime();
    bool initialized = false;

    vtkSmartPointer<vtkVertexListIterator> vItr =
      vtkSmartPointer<vtkVertexListIterator>::New();
    graph->GetVertices(vItr);

    // Compute global min and max
    while(vItr->HasNext())
      {
      vtkIdType vid = graphModel->GetNodeId(vItr->Next());
      vertexIds.push_back(vid);
      }

    for (size_t i = 0; i < vertexIds.size(); ++i)
      {
      curr = (graphModel->*timeFunc)(static_cast<vtkIdType>(vertexIds[i]));

      if (curr == vgTimeStamp::InvalidTime())
        {
        continue;
        }

      if (!initialized)
        {
        min = curr;
        max = curr;
        ids[0] = vertexIds[i];
        ids[1] = vertexIds[i];
        initialized = true;
        continue;
        }

      if (curr < min)
        {
        ids[0] = vertexIds[i];
        min = curr;
        }
      if (curr > max)
        {
        ids[1] = vertexIds[i];
        max = curr;
        }
      }

    timeRange[0] = min;
    timeRange[1] = max;
  }

  //----------------------------------------------------------------------------
  void  UseSortByTimeLayout(vpMultiGraphModel* graphModel,
                            vtkRenderer* ren,
                            const vgRange<double>& widthRange,
                            const vgRange<double>& heightRange,
                            GetNodeTimeFn timeFunc,
                            int prevLayoutMode)
  {
    vtkGraph* graph = graphModel->GetGraph(vpMultiGraphModel::NoneDomain);

    // Some useful variables
    typedef std::map< unsigned int, std::vector<vtkIdType> >::iterator Itr;
    Itr itr;
    bool initialized = false;
    int row = 0;
    double x, y;
    double min = vgTimeStamp::InvalidTime();
    double max = vgTimeStamp::InvalidTime();
    unsigned int minFrame = vgTimeStamp::InvalidFrameNumber();
    unsigned int maxFrame = vgTimeStamp::InvalidFrameNumber();
    double curr;
    double xScale, yScale;

    double geomBounds[6];
    this->NodeGeometery->GetBounds(geomBounds);
    double distanceBtwCols =  (geomBounds[1] - geomBounds[0]);
    double distanceBtwRows =  (geomBounds[3] - geomBounds[2]) * 2.0;

    std::vector<vtkIdType> vertexIds;
    std::map< unsigned int, std::vector<vtkIdType> > verticesGroupById;

    vtkSmartPointer<vtkVertexListIterator> vItr =
      vtkSmartPointer<vtkVertexListIterator>::New();
    graph->GetVertices(vItr);

    // Compute global min and max
    while(vItr->HasNext())
      {
      vtkIdType vid = graphModel->GetNodeId(vItr->Next());
      vertexIds.push_back(vid);

      std::vector<vtkIdType>& vertices =
        verticesGroupById[graphModel->GetGroupId(vid)];

      vertices.push_back(vid);
      }

    int minId = -1;
    int maxId = -1;
    for (size_t i = 0; i < vertexIds.size(); ++i)
      {
      curr = (graphModel->*timeFunc)(static_cast<vtkIdType>(vertexIds[i]));

      if (curr == vgTimeStamp::InvalidTime())
        {
        continue;
        }

      if (!initialized)
        {
        min = curr;
        max = curr;
        minId = vertexIds[i];
        maxId = vertexIds[i];

        initialized = true;
        continue;
        }

      if (curr < min)
        {
        min = curr;
        minId = vertexIds[i];
        }
      if (curr > max)
        {
        max = curr;
        maxId = vertexIds[i];
        }
      }

    minFrame = graphModel->GetNodeStartFrame(minId);
    maxFrame = graphModel->GetNodeStartFrame(maxId);

    if (max == min)
      {
      min = max * 0.5;
      minFrame = maxFrame * 0.5;
      }

    this->TimelineFrameXScale = maxFrame - minFrame;

    xScale = max - min;
    if (xScale != 0.0)
      {
      xScale = (widthRange.upper - widthRange.lower) / xScale;
      this->TimelineFrameXScale =
        (widthRange.upper - widthRange.lower) / this->TimelineFrameXScale;
      }

    yScale = static_cast<double>(verticesGroupById.size());
    if (yScale > 0.0)
      {
      yScale = (heightRange.upper - heightRange.lower) / yScale;
      }

    this->TimelineXScale = xScale;
    this->TimelineMinTime = min;
    this->TimelineMinFrame = minFrame;
    this->TimelineXMin = widthRange.lower;

    itr = verticesGroupById.begin();

    // Current positions
    double currX, currY, currZ;
    std::vector<vtkIdType> ids;
    std::vector<double> currXYZs;
    std::vector<double> newXYZs;

    // Now position the nodes appropriately
    if (!verticesGroupById.empty())
      {
      distanceBtwRows = distanceBtwRows / verticesGroupById.size();
      }

    vpMultiGraphModel::NodePositionType prevPositionType =
      GetPositionTypeForLayout(prevLayoutMode);

    for (; itr != verticesGroupById.end(); ++itr, ++row)
      {
      std::vector<vtkIdType>& vts = itr->second;

      // Now sort them
      std::sort(vts.begin(), vts.end(),
                SortGraphByTime(graphModel, timeFunc));

      std::map<double, int> nodesWithSameXPos;
      for (size_t i = 0; i < vts.size(); ++i)
        {
        curr = (graphModel->*timeFunc)(vts[i]);

        if (curr != vgTimeStamp::InvalidTime())
          {
          x = widthRange.lower + (curr - min) * xScale;
          y = heightRange.lower + row * yScale +
              nodesWithSameXPos[x] * yScale * 0.01;
          }
        else
          {
          x = widthRange.lower - (1.5 * distanceBtwCols);
          y = heightRange.lower - (1.5 * distanceBtwRows);
          }

        if ( x < this->TimelineXMin)
          {
          this->TimelineXMin = x;
          }

        ids.push_back(vts[i]);

        graphModel->GetNodePosition(prevPositionType, vts[i],
                                    currX, currY, currZ);
        currXYZs.push_back(currX);
        currXYZs.push_back(currY);
        currXYZs.push_back(currZ);

        newXYZs.push_back(x);
        newXYZs.push_back(y);
        newXYZs.push_back(0.0);

        nodesWithSameXPos[x] += 1;
        }
      }

    this->Animate(graphModel, ren, ids, currXYZs, newXYZs);
    }

  //----------------------------------------------------------------------------
  void  UseSpatialLayout(vpMultiGraphModel* graphModel,
                         vtkRenderer* ren,
                         const vgRange<double>& widthRange,
                         const vgRange<double>& heightRange,
                         int prevLayoutMode)
  {
    vtkGraph* graph = graphModel->GetGraph(vpMultiGraphModel::NoneDomain);

    double geomBounds[6];
    this->NodeGeometery->GetBounds(geomBounds);
    double distanceBtwCols =  (geomBounds[1] - geomBounds[0]);
    double distanceBtwRows =  (geomBounds[3] - geomBounds[2]) * 2.0;

    vtkSmartPointer<vtkVertexListIterator> vItr =
      vtkSmartPointer<vtkVertexListIterator>::New();
    graph->GetVertices(vItr);

    // Compute min and max spatial positions
    double minX = VTK_DOUBLE_MAX;
    double maxX = VTK_DOUBLE_MIN;
    double minY = VTK_DOUBLE_MAX;
    double maxY = VTK_DOUBLE_MIN;
    while (vItr->HasNext())
      {
      vtkIdType vid = graphModel->GetNodeId(vItr->Next());
      double x, y;
      graphModel->GetSpatialPosition(vid, x, y);

      if (vtkMath::IsNan(x) || vtkMath::IsNan(y))
        {
        continue;
        }

      if (x < minX)
        {
        minX = x;
        }
      if (x > maxX)
        {
        maxX = x;
        }
      if (y < minY)
        {
        minY = y;
        }
      if (y > maxY)
        {
        maxY = y;
        }
      }

    double xScale = maxX - minX;
    if (xScale != 0.0)
      {
      xScale = (widthRange.upper - widthRange.lower) / xScale;
      }

    double yScale = maxY - minY;
    if (yScale != 0.0)
      {
      yScale = (heightRange.upper - heightRange.lower) / yScale;
      }

    // Fit the graph bounds in the viewport, but maintain the aspect ratio
    double scale = std::min(xScale, yScale);

    // Compute the transform to map the spatial bounds
    // to the viewport bounds we have been given.
    this->SpatialTransform->Identity();
    this->SpatialTransform->Translate(
      0.5 * (widthRange.lower + widthRange.upper),
      0.5 * (heightRange.lower + heightRange.upper));
    this->SpatialTransform->Scale(scale, scale);
    this->SpatialTransform->Translate(-0.5 * (maxX + minX),
                                      -0.5 * (maxY + minY));

    std::vector<vtkIdType> ids;
    std::vector<double> currXYZs;
    std::vector<double> newXYZs;

    vpMultiGraphModel::NodePositionType prevPositionType =
      GetPositionTypeForLayout(prevLayoutMode);

    // Compute node positions based on normalized spatial positions
    graph->GetVertices(vItr);
    while (vItr->HasNext())
      {
      vtkIdType vid = graphModel->GetNodeId(vItr->Next());
      double point[3];
      graphModel->GetSpatialPosition(vid, point[0], point[1]);
      point[2] = 1.0;

      if (!vtkMath::IsNan(point[0]) && !vtkMath::IsNan(point[1]))
        {
        SpatialTransform->TransformPoints(point, point, 1);
        }
      else
        {
        point[0] = widthRange.lower - (1.5 * distanceBtwCols);
        point[1] = heightRange.lower - (1.5 * distanceBtwRows);
        }

      ids.push_back(vid);

      double currX, currY, currZ;
      graphModel->GetNodePosition(prevPositionType, vid, currX, currY, currZ);
      currXYZs.push_back(currX);
      currXYZs.push_back(currY);
      currXYZs.push_back(currZ);

      newXYZs.push_back(point[0]);
      newXYZs.push_back(point[1]);
      newXYZs.push_back(0.0);
      }

    // Invert so clients can later quickly get the graph to spatial transform
    this->SpatialTransform->Inverse();

    this->Animate(graphModel, ren, ids, currXYZs, newXYZs);
  }

  //---------------------------------------------------------------------------
  void Animate(vpMultiGraphModel* graphModel, vtkRenderer* ren,
    const std::vector<vtkIdType>& ids, const std::vector<double>& currXYZs,
    const std::vector<double>& newXYZs)
  {
    this->GraphAnimateCallback->Reset(
      graphModel, this->Parent, ren, ids, currXYZs, newXYZs,
      GetPositionTypeForLayout(this->Parent->GetLayoutMode()));

    if (!ren->GetRenderWindow()->GetInteractor()->HasObserver(
          vtkCommand::TimerEvent, this->GraphAnimateCallback))
      {
      ren->GetRenderWindow()->GetInteractor()->AddObserver(
        vtkCommand::TimerEvent, this->GraphAnimateCallback);
      }

    ren->GetRenderWindow()->GetInteractor()->CreateOneShotTimer(0);
  }

  //----------------------------------------------------------------------------
  vtkVgTimeStamp ComputeNodeStartTime(
      double x, double vtkNotUsed(y), double vtkNotUsed(z))
    {
      vtkVgTimeStamp timestamp;
      timestamp.SetTime(vgTimeStamp::InvalidTime());
      timestamp.SetFrameNumber(vgTimeStamp::InvalidFrameNumber());

      if (this->TimelineXScale != 0.0)
        {
        double time;
        unsigned int frame;

        double distance = x - this->TimelineXMin;
        time = fabs(distance) * (1 / this->TimelineXScale);
        frame = fabs(distance) * (1.0 / this->TimelineFrameXScale);
        if (distance < 0.0)
          {
          time = this->TimelineMinTime - time;
          frame = this->TimelineMinFrame - frame;
          }
        else
          {
          time = this->TimelineMinTime + time;
          frame = this->TimelineMinFrame + frame;
          }
        timestamp.SetTime(time);
        timestamp.SetFrameNumber(frame);
      }

      return timestamp;
    }

  vpMultiGraphRepresentation* Parent;

  std::vector<GraphEntity> GraphEntities;

  vtkSmartPointer<vtkPolyData> NodeGeometery;
  vtkSmartPointer<vtkUnsignedCharArray> VertexColorArray;
  vtkSmartPointer<vtkUnsignedCharArray> EdgeColorArray;
  vtkSmartPointer<vtkIdTypeArray> PickedIdArray;

  vtkSmartPointer<vtkTransform2D> SpatialTransform;

  vtkSmartPointer<vpGraphAnimateCallback> GraphAnimateCallback;
};

//-----------------------------------------------------------------------------
vpMultiGraphRepresentation::vtkInternal::vtkInternal(
  vpMultiGraphRepresentation* parent)
{
  this->Parent = parent;

  vtkSmartPointer<vtkRegularPolygonSource> polygonSource =
    vtkSmartPointer<vtkRegularPolygonSource>::New();

  polygonSource->SetNumberOfSides(50);
  polygonSource->SetRadius(0.1);
  polygonSource->SetCenter(0, 0, 0);
  polygonSource->Update();

  this->NodeGeometery = vtkSmartPointer<vtkPolyData>::New();
  this->NodeGeometery->DeepCopy(polygonSource->GetOutput());

  this->GraphAnimateCallback = vtkSmartPointer<vpGraphAnimateCallback>::New();

  this->SpatialTransform = vtkSmartPointer<vtkTransform2D>::New();
}

//-----------------------------------------------------------------------------
vpMultiGraphRepresentation::vtkInternal::~vtkInternal()
{
}

//-----------------------------------------------------------------------------
vpMultiGraphRepresentation::vpMultiGraphRepresentation()
{
  this->Visible = 1;

  this->Internal = new vtkInternal(this);

  this->NewPropCollection = vtkPropCollectionRef::New();
  this->ActivePropCollection = vtkPropCollectionRef::New();
  this->ExpirePropCollection = vtkPropCollectionRef::New();

  this->Picker = vtkSmartPointer<vtkHardwareSelector>::New();

  this->GraphLayoutMode = Default;

  this->VertexSize = 400.0;
  this->VertexOpacity = 1.0;
  this->ZOffset = 0.0;

  this->ForegroundColor[0] =
    this->ForegroundColor[1] =
    this->ForegroundColor[2] = 0.0;
}

//-----------------------------------------------------------------------------
vpMultiGraphRepresentation::~vpMultiGraphRepresentation()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetGraphModel(
  vpMultiGraphModel* graphModel)
{
  if (this->GraphModel != graphModel)
    {
    this->GraphModel = graphModel;

    if (this->GraphModel && this->UseAutoUpdate)
      {
      this->GraphModel->AddObserver(vtkCommand::UpdateDataEvent, this,
                                    &vpMultiGraphRepresentation::Update);
      }

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vpMultiGraphModel* vpMultiGraphRepresentation::GetGraphModel() const
{
  return this->GraphModel;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vpMultiGraphRepresentation::GetNewRenderObjects() const
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vpMultiGraphRepresentation::GetActiveRenderObjects() const
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
const vtkPropCollection* vpMultiGraphRepresentation::GetExpiredRenderObjects() const
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vpMultiGraphRepresentation::GetNewRenderObjects()
{
  return this->NewPropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vpMultiGraphRepresentation::GetActiveRenderObjects()
{
  return this->ActivePropCollection;
}

//-----------------------------------------------------------------------------
vtkPropCollection* vpMultiGraphRepresentation::GetExpiredRenderObjects()
{
  return this->ExpirePropCollection;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::ResetTemporaryRenderObjects()
{
  this->NewPropCollection->RemoveAllItems();
  this->ExpirePropCollection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetVisible(int flag)
{
  if (flag == this->Visible)
    {
    return;
    }

  this->Visible = flag;

  this->ActivePropCollection->InitTraversal();
  while (vtkProp* prop = this->ActivePropCollection->GetNextProp())
    {
    prop->SetVisibility(flag);
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetCurrentVisibleGraph(
  const std::string& key)
{
  if (key.compare(this->CurrentVisibleGraphKey) == 0)
    {
    return;
    }

  size_t count = this->Internal->GraphEntities.size();

  for (size_t i = 0; i < count; ++i)
    {
    // Master graph is always visible
    if (this->Internal->GraphEntities[i].Key.compare(
          vpMultiGraphModel::NoneDomain) == 0)
      {
      continue;
      }
    if (key.compare(this->Internal->GraphEntities[i].Key) == 0)
      {
      this->Internal->GraphEntities[i].Actor->SetVisibility(1);
      }
    else
      {
      this->Internal->GraphEntities[i].Actor->SetVisibility(0);
      }
    }

  this->CurrentVisibleGraphKey = key;

  this->Modified();
}

//-----------------------------------------------------------------------------
const std::string& vpMultiGraphRepresentation::GetCurrentVisibleGraph() const
{
  return this->CurrentVisibleGraphKey;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::Initialize()
{
  if (!this->GraphModel)
    {
    vtkErrorMacro("Event model must be set before initialization");
    return;
    }

  this->ReinitializeRenderData();

  typedef vtkSmartPointer<vtkMutableDirectedGraph>
    vtkMutableDirectedGraphPtr;
  typedef vtkSmartPointer<vtkMutableUndirectedGraph>
    vtkMutableUndirectedGraphPtr;

  // Add undirected graphs
  std::map<std::string, vtkMutableUndirectedGraphPtr>::iterator ungItr;
  std::map<std::string, vtkMutableUndirectedGraphPtr> unDirGraphs =
    this->GraphModel->GetAllUndirectedGraphs();
  ungItr = unDirGraphs.begin();
  for (; ungItr != unDirGraphs.end(); ++ungItr)
    {
    if (ungItr->first != vpMultiGraphModel::NoneDomain)
      {
      this->AddGraph(ungItr->second, ungItr->first);
      }
    }

  // Add directed graphs
  std::map<std::string, vtkMutableDirectedGraphPtr>::iterator dgItr;
  std::map<std::string, vtkMutableDirectedGraphPtr> dirGraphs =
    this->GraphModel->GetAllDirectedGraphs();
  dgItr = dirGraphs.begin();
  for (; dgItr != dirGraphs.end(); ++dgItr)
    {
    this->AddGraph(dgItr->second, dgItr->first);
    }

  // Add the main node graph last so that it gets rendered last
  this->AddGraph(this->GraphModel->GetGraph(vpMultiGraphModel::NoneDomain),
                 vpMultiGraphModel::NoneDomain);

  this->UpdateRenderObjectsTime.Modified();
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::Update()
{
  // \note: Currently call initialize every update as the graph model has
  // changed.
  this->Initialize();

  if (!this->GetVisible())
    {
    return;
    }

  double zOffset = this->GetZOffset();
  size_t count = this->Internal->GraphEntities.size();

  for (size_t i = 0; i < count; ++i)
    {
    this->Internal->GraphEntities[i].Actor->SetPosition(0.0, 0.0, zOffset);
    this->Internal->GraphEntities[i].Actor->GetMapper()->Update();
    }

  this->UpdateTime.Modified();
}

class HideLabelsAndGlyphs
{
  vpMultiGraphRepresentation* Rep;

public:
  HideLabelsAndGlyphs(vpMultiGraphRepresentation* rep)
    : Rep(rep)
    {
    Rep->SetLabelsVisible(false);
    Rep->SetEdgeGlyphsVisible(false);
    }

  ~HideLabelsAndGlyphs()
    {
    Rep->SetLabelsVisible(true);
    Rep->SetEdgeGlyphsVisible(true);
    }
};

//-----------------------------------------------------------------------------
vtkIdType vpMultiGraphRepresentation
::Pick(double renX, double renY, vtkRenderer* ren, vtkIdType& pickType)
{
  return this->Pick(renX, renY, renX, renY, ren, pickType);
}

//-----------------------------------------------------------------------------
vtkIdType vpMultiGraphRepresentation
::Pick(double x1, double y1, double x2, double y2,
       vtkRenderer* ren, vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;
  this->PickedGraphDomain.clear();

  double minX = std::min(x1, x2);
  double minY = std::min(y1, y2);
  double maxX = std::max(x1, x2);
  double maxY = std::max(y1, y2);

  if (!this->GetVisible() ||
      minX < 0.0 || minY < 0.0 ||
      maxX >= ren->GetSize()[0] || maxY >= ren->GetSize()[1])
    {
    return -1;
    }

  // Labels interfere with selection so turn them off temporarily
  HideLabelsAndGlyphs hide(this);

  // If any nodes are picked, only pick nodes. This behavior is intentional.
    {
    vtkIdType id = this->PickNodes(minX, minY, maxX, maxY, ren);
    if (id != -1)
      {
      pickType = vtkVgPickData::PickedGraphNode;
      return id;
      }
    }

    {
    vtkIdType id = this->PickEdges(minX, minY, maxX, maxY, ren);
    if (id != -1)
      {
      pickType = vtkVgPickData::PickedGraphEdge;
      return id;
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
vtkIdType vpMultiGraphRepresentation
::PickNodes(double x1, double y1, double x2, double y2, vtkRenderer* ren)
{
  this->Picker->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_POINTS);
  this->Picker->SetRenderer(ren);
  this->Picker->SetArea(x1, y1, x2, y2);

  vtkSmartPointer<vtkSelection> selection =
    vtkSmartPointer<vtkSelection>::Take(this->Picker->Select());

  // Figure out which actor is being used for the master graph. It will
  // most likely always be the first one, but just to be sure...
  vtkActor* nodeGraphActor = 0;
  for (size_t i = 0, end = this->Internal->GraphEntities.size(); i != end; ++i)
    {
    if (this->Internal->GraphEntities[i].Key == vpMultiGraphModel::NoneDomain)
      {
      nodeGraphActor = this->Internal->GraphEntities[i].Actor;
      break;
      }
    }

  for (int i = 0, end = selection->GetNumberOfNodes(); i != end; ++i)
    {
    vtkSelectionNode* node = selection->GetNode(i);

    if (vtkActor* actor =
          vtkActor::SafeDownCast(
            node->GetProperties()->Get(vtkSelectionNode::PROP())))
      {
      // Is the selected prop our node graph?
      if (actor != nodeGraphActor)
        {
        continue;
        }

      if (vtkIdTypeArray* ids =
            vtkIdTypeArray::SafeDownCast(node->GetSelectionList()))
        {
        this->Internal->PickedIdArray = ids;
        this->PickedGraphDomain = vpMultiGraphModel::NoneDomain;

        // Convert to pedigree ids
        for (vtkIdType i = 0,
             end = this->Internal->PickedIdArray->GetNumberOfTuples();
             i != end; ++i)
          {
          this->Internal->PickedIdArray->SetValue(
            i, this->GraphModel->GetNodeId(
                this->Internal->PickedIdArray->GetValue(i)));
          }

        // Return first selected node id
        return this->Internal->PickedIdArray->GetValue(0);
        }
      }
    }

  this->Internal->PickedIdArray = 0;
  return -1;
}

//-----------------------------------------------------------------------------
vtkIdType vpMultiGraphRepresentation
::PickEdges(double x1, double y1, double x2, double y2, vtkRenderer* ren)
{
  this->Picker->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_CELLS);
  this->Picker->SetRenderer(ren);
  this->Picker->SetArea(x1, y1, x2, y2);

  vtkSmartPointer<vtkSelection> selection =
    vtkSmartPointer<vtkSelection>::Take(this->Picker->Select());

  if (selection->GetNumberOfNodes() == 0)
    {
    this->Internal->PickedIdArray = 0;
    return -1;
    }

  vtkSelectionNode* node = selection->GetNode(0);

  if (vtkIdTypeArray* ids =
        vtkIdTypeArray::SafeDownCast(node->GetSelectionList()))
    {
    // Figure out which graph this edge belonged to
    if (vtkActor* actor =
          vtkActor::SafeDownCast(
            node->GetProperties()->Get(vtkSelectionNode::PROP())))
      {
      for (size_t i = 0, end = this->Internal->GraphEntities.size();
           i != end; ++i)
        {
        if (actor == this->Internal->GraphEntities[i].Actor)
          {
          this->PickedGraphDomain = this->Internal->GraphEntities[i].Key;
          break;
          }
        }
      }

    this->Internal->PickedIdArray = ids;

    // Convert to pedigree ids
    for (vtkIdType i = 0,
         end = this->Internal->PickedIdArray->GetNumberOfTuples();
         i != end; ++i)
      {
      this->Internal->PickedIdArray->SetValue(
        i, this->GraphModel->GetEdgeId(
            this->Internal->PickedIdArray->GetValue(i),
            this->PickedGraphDomain));
      }

    // Return first selected edge id
    return this->Internal->PickedIdArray->GetValue(0);
    }

  return -1;
}

//-----------------------------------------------------------------------------
void  vpMultiGraphRepresentation::UseSortByStartTimeLayout(vtkRenderer* ren,
  const vgRange<double>& widthRange, const vgRange<double>& heightRange,
  int prevLayoutMode)
{
  this->Internal->UseSortByTimeLayout(this->GraphModel, ren,
    widthRange, heightRange, &vpMultiGraphModel::GetNodeStartTime,
    prevLayoutMode);
}

//-----------------------------------------------------------------------------
void  vpMultiGraphRepresentation::UseSpatialLayout(vtkRenderer* ren,
  const vgRange<double>& widthRange, const vgRange<double>& heightRange,
  int prevLayoutMode)
{
  this->Internal->UseSpatialLayout(this->GraphModel, ren,
                                   widthRange, heightRange, prevLayoutMode);
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::UseRandomLayout(vtkRenderer* ren,
  const vgRange<double>& widthRange,
  const vgRange<double>& heightRange,
  int prevLayoutMode)
{
  Q_UNUSED(widthRange);
  Q_UNUSED(heightRange);
  vtkGraph* graph = this->GraphModel->GetGraph(vpMultiGraphModel::NoneDomain);

  // Generate a random position in x and y directions.
  vtkSmartPointer<vtkVertexListIterator> vItr =
    vtkSmartPointer<vtkVertexListIterator>::New();
  graph->GetVertices(vItr);

  double x, y, z;
  double currX, currY, currZ;
  std::vector<vtkIdType> ids;
  std::vector<double> fromXYZs;
  std::vector<double> toXYZs;

  vpMultiGraphModel::NodePositionType prevPositionType =
    GetPositionTypeForLayout(prevLayoutMode);

  while(vItr->HasNext())
    {
    vtkIdType id = this->GraphModel->GetNodeId(vItr->Next());

    this->GraphModel->GetNodePosition(prevPositionType, id, currX, currY, currZ);
    fromXYZs.push_back(currX);
    fromXYZs.push_back(currY);
    fromXYZs.push_back(currZ);

    this->GraphModel->GetCachedPosition(id, x, y, z);
    toXYZs.push_back(x);
    toXYZs.push_back(y);
    toXYZs.push_back(0.0);

    ids.push_back(id);
    }

  this->Internal->Animate(this->GraphModel, ren, ids, fromXYZs, toXYZs);

  this->Modified();
}

//-----------------------------------------------------------------------------
vtkIdTypeArray* vpMultiGraphRepresentation::GetPickedIds()
{
  return this->Internal->PickedIdArray;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetEventRegistry(vtkVgEventTypeRegistry* reg)
{
  if (!reg)
    {
    return;
    }

  this->EventTypeRegisty = reg;
  this->Modified();
}

//-----------------------------------------------------------------------------
vtkVgEventTypeRegistry* vpMultiGraphRepresentation::GetEventRegistry() const
{
  return this->EventTypeRegisty;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::AddGraph(vtkGraph* graph,
                                          const std::string& domain)
{
  if (!graph)
    {
    return;
    }

  if (this->Internal->VertexColorArray)
    {
    graph->GetVertexData()->AddArray(this->Internal->VertexColorArray);
    }

  vtkCreateMacro(vtkVg2DGraphMapper, mapper);
  mapper->SetVertexPointSize(this->VertexSize);
  mapper->SetVertexOpacity(this->VertexOpacity);
  mapper->SetInputData(graph);

  mapper->SetVertexPositionArrayName(
    GetPositionArrayNameForLayout(this->GraphLayoutMode));

  // Only show the vertices of the master graph
  if (domain.compare(vpMultiGraphModel::NoneDomain) == 0)
    {
    mapper->SetVertexGeometry(this->Internal->NodeGeometery);
    mapper->SetVertexColorArrayName("VertexColors");
    mapper->SetColorVertices(true);
    mapper->SetLabelArrayName("NodeLabels");
    mapper->SetLabelVisibility(true);
    mapper->SetVertexVisibility(1);
    mapper->SetVertexSelectionArrayName("Selected");
    mapper->SetUseVertexSelection(true);
    mapper->SetLabelColor(this->ForegroundColor);
    }
  else
    {
    mapper->SetVertexVisibility(0);
    if (vtkUndirectedGraph::SafeDownCast(graph))
      {
      mapper->GetEdgeGlyphSource()->SetGlyphTypeToDiamond();
      mapper->GetEdgeGlyphSource()->SetScale(0.75);
      }
    mapper->SetEdgeSelectionArrayName("Selected");
    mapper->SetUseEdgeSelection(true);
    mapper->SetUseEdgeLayout(true);
    mapper->SetEdgeGlyphColor(this->ForegroundColor);
    }

  mapper->SetEdgeLineWidth(5.0f);
  mapper->Update();

  vtkInternal::GraphEntity graphEntity;
  graphEntity.Actor = vtkSmartPointer<vtkActor>::New();
  graphEntity.Actor->SetMapper(mapper);
  graphEntity.Actor->PickableOn();
  graphEntity.Key = domain;

  this->Internal->GraphEntities.push_back(graphEntity);

  this->NewPropCollection->AddItem(graphEntity.Actor);
  this->ActivePropCollection->AddItem(graphEntity.Actor);

  // Do not turn off visibility of master graph
  if (domain.compare(vpMultiGraphModel::NoneDomain) != 0)
    {
    if (this->CurrentVisibleGraphKey.compare(domain) != 0)
      {
      graphEntity.Actor->SetVisibility(0);
      }
    else
      {
      graphEntity.Actor->SetVisibility(1);
      }
    }
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetLabelsVisible(bool visible)
{
  for (size_t i = 0, end = this->Internal->GraphEntities.size(); i != end; ++i)
    {
    if (this->Internal->GraphEntities[i].Key == vpMultiGraphModel::NoneDomain)
      {
      if (vtkVg2DGraphMapper* mapper =
            vtkVg2DGraphMapper::SafeDownCast(
              this->Internal->GraphEntities[i].Actor->GetMapper()))
        {
        mapper->SetLabelVisibility(visible ? 1 : 0);
        }
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetEdgeGlyphsVisible(bool visible)
{
  for (size_t i = 0, end = this->Internal->GraphEntities.size(); i != end; ++i)
    {
    if (this->Internal->GraphEntities[i].Key != vpMultiGraphModel::NoneDomain)
      {
      if (vtkVg2DGraphMapper* mapper =
            vtkVg2DGraphMapper::SafeDownCast(
              this->Internal->GraphEntities[i].Actor->GetMapper()))
        {
        mapper->SetEdgeGlyphVisibility(visible ? 1 : 0);
        }
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::ResetCachedNodePositions()
{
  vtkGraph* graph = this->GraphModel->GetGraph(vpMultiGraphModel::NoneDomain);

  vtkSmartPointer<vtkVertexListIterator> vItr =
    vtkSmartPointer<vtkVertexListIterator>::New();
  graph->GetVertices(vItr);

  vpMultiGraphModel::NodePositionType positionType = this->GetNodePositionType();

  double x, y, z;
  while (vItr->HasNext())
    {
    vtkIdType id = vItr->Next();
    this->GraphModel->GetNodePosition(positionType, id, x, y, z);
    this->GraphModel->SetCachedPosition(id, x, y, z);
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetLayoutMode(int mode, vtkRenderer* ren,
  const vgRange<double>& widthRange, const vgRange<double>& heightRange)
{
  int prevLayoutMode = this->GraphLayoutMode;
  this->GraphLayoutMode = mode;

  if (!this->GraphModel)
    {
      vtkErrorMacro("<< Graph model is not set. Cannot change layout.");
    }

  vtkGraph* graph = this->GraphModel->GetGraph(vpMultiGraphModel::NoneDomain);
  if (!graph)
    {
    vtkErrorMacro("<< Master graph does not exist.");
    }

  switch (this->GraphLayoutMode)
    {
    case SortByStartTime:
      {
      this->UseSortByStartTimeLayout(ren, widthRange, heightRange, prevLayoutMode);
      this->GraphModel->LockPositionOfNodeOn();
      break;
      }
    case Spatial:
      {
      this->UseSpatialLayout(ren, widthRange, heightRange, prevLayoutMode);
      // Non-event based nodes can be moved in this mode
      this->GraphModel->LockPositionOfNodeOff();
      break;
      }
    case RawSpatial:
      {
      // Using raw spatial coordinates, no explicit layout is required
      break;
      }
    default:
      {
      this->GraphModel->LockPositionOfNodeOff();
      this->UseRandomLayout(ren, widthRange, heightRange, prevLayoutMode);
      }
    };

  // Tell the mapper which array to use for vertex positioning
  for (size_t i = 0, sz = this->Internal->GraphEntities.size(); i != sz; ++i)
    {
    if (vtkVg2DGraphMapper* mapper =
          vtkVg2DGraphMapper::SafeDownCast(
            this->Internal->GraphEntities[i].Actor->GetMapper()))
      {
      mapper->SetVertexPositionArrayName(
        GetPositionArrayNameForLayout(this->GraphLayoutMode));
      }
    }

  this->CurrentLayoutRenderer = ren;
  this->CurrentLayoutExtentsX = widthRange;
  this->CurrentLayoutExtentsY = heightRange;

  this->Modified();
}

//-----------------------------------------------------------------------------
int vpMultiGraphRepresentation::GetLayoutMode()
{
  return this->GraphLayoutMode;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::RefreshLayout()
{
  if (this->CurrentLayoutRenderer)
    {
    this->SetLayoutMode(this->GraphLayoutMode, this->CurrentLayoutRenderer,
                        this->CurrentLayoutExtentsX,
                        this->CurrentLayoutExtentsY);
    }
}

//-----------------------------------------------------------------------------
vpMultiGraphModel::NodePositionType
vpMultiGraphRepresentation::GetNodePositionType()
{
  return GetPositionTypeForLayout(this->GraphLayoutMode);
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetVertexSize(double size)
{
  for (size_t i = 0, end = this->Internal->GraphEntities.size(); i != end; ++i)
    {
    if (this->Internal->GraphEntities[i].Key == vpMultiGraphModel::NoneDomain)
      {
      if (vtkVg2DGraphMapper* mapper =
            vtkVg2DGraphMapper::SafeDownCast(
              this->Internal->GraphEntities[i].Actor->GetMapper()))
        {
        mapper->SetVertexPointSize(size);
        }
      break;
      }
    }

  this->VertexSize = size;

  this->Modified();
}

//-----------------------------------------------------------------------------
double vpMultiGraphRepresentation::GetVertexSize()
{
  return this->VertexSize;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetVertexOpacity(double opacity)
{
  for (size_t i = 0, end = this->Internal->GraphEntities.size(); i != end; ++i)
    {
    if (this->Internal->GraphEntities[i].Key == vpMultiGraphModel::NoneDomain)
      {
      if (vtkVg2DGraphMapper* mapper =
            vtkVg2DGraphMapper::SafeDownCast(
              this->Internal->GraphEntities[i].Actor->GetMapper()))
        {
        mapper->SetVertexOpacity(opacity);
        }
      break;
      }
    }

  this->VertexOpacity = opacity;

  this->Modified();
}

//-----------------------------------------------------------------------------
vtkMatrix3x3* vpMultiGraphRepresentation::GetGraphToSpatialMatrix()
{
  return this->Internal->SpatialTransform->GetMatrix();
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::SetForegroundColor(double color[3])
{
  this->ForegroundColor[0] = color[0];
  this->ForegroundColor[1] = color[1];
  this->ForegroundColor[2] = color[2];
  for (size_t i = 0, end = this->Internal->GraphEntities.size(); i != end; ++i)
    {
    if (vtkVg2DGraphMapper* mapper =
          vtkVg2DGraphMapper::SafeDownCast(
            this->Internal->GraphEntities[i].Actor->GetMapper()))
      {
      if (this->Internal->GraphEntities[i].Key == vpMultiGraphModel::NoneDomain)
        {
        mapper->SetLabelColor(color);
        }
      else
        {
        mapper->SetEdgeGlyphColor(color);
        }
      }
    }
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vpMultiGraphRepresentation::ComputeNodeStartTime(
  double x, double y, double z)
{
  return this->Internal->ComputeNodeStartTime(x, y, z);
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::GetCurrentLayoutExtents(vgRange<double>& xext,
                                                         vgRange<double>& yext)
{
  xext = this->CurrentLayoutExtentsX;
  yext = this->CurrentLayoutExtentsY;
}

//-----------------------------------------------------------------------------
void vpMultiGraphRepresentation::ReinitializeRenderData()
{
  // Add existing actor to the list of to be removed
  size_t count = this->Internal->GraphEntities.size();
  for (size_t i = 0; i < count; ++i)
    {
    this->ExpirePropCollection->AddItem(this->Internal->GraphEntities[i].Actor);
    }
  this->Internal->GraphEntities.clear();

  this->ActivePropCollection->RemoveAllItems();
  this->NewPropCollection->RemoveAllItems();

  // Reinitialize arrays
  this->Internal->VertexColorArray =
    vtkSmartPointer<vtkUnsignedCharArray>::New();

  vtkGraph* masterGraph =
    this->GraphModel->GetGraph(vpMultiGraphModel::NoneDomain);
  if (masterGraph)
    {
    vtkSmartPointer<vtkVertexListIterator> vItr =
      vtkSmartPointer<vtkVertexListIterator>::New();
    masterGraph->GetVertices(vItr);

    vtkSmartPointer<vtkStringArray> nodeTypes =
      vtkStringArray::SafeDownCast(
        masterGraph->GetVertexData()->GetAbstractArray("NodeTypes"));

    this->Internal->VertexColorArray->SetName("VertexColors");
    this->Internal->VertexColorArray->SetNumberOfComponents(4);
    this->Internal->VertexColorArray->SetNumberOfTuples(
      masterGraph->GetNumberOfVertices());

    while (vItr->HasNext())
      {
      vtkIdType id = vItr->Next();
      std::string label = nodeTypes->GetValue(id);
      int typeId = vpEventConfig::GetIdFromString(label.c_str());
      if (typeId == -1)
        {
        // Type string not valid
        unsigned char c[4] = { 0 };
        this->Internal->VertexColorArray->SetTypedTuple(id, c);
        continue;
        }

      vgEventType type = this->EventTypeRegisty->GetTypeById(typeId);

      double rgba[4] = { 1.0, 1.0, 1.0, 1.0 };
      type.GetColor(rgba[0], rgba[1], rgba[2]);
      unsigned char c[4];
      vtkVgColorUtil::convert(rgba, c);
      this->Internal->VertexColorArray->SetTypedTuple(id, c);
      }
    }
}
