// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgGraphModel.h"

#include "vtkVgActivity.h"
#include "vtkVgActivityManager.h"
#include "vtkVgEvent.h"
#include "vtkVgEventFilter.h"
#include "vtkVgEventModel.h"
#include "vtkVgTrack.h"
#include "vtkVgTrackModel.h"

#include <vtkCommand.h>
#include <vtkDataSetAttributes.h>
#include <vtkIdListCollection.h>
#include <vtkIntArray.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkTimeStamp.h>

// C++ includes
#include <map>

vtkStandardNewMacro(vtkVgGraphModel);

vtkCxxSetObjectMacro(vtkVgGraphModel, TrackModel, vtkVgTrackModel);
vtkCxxSetObjectMacro(vtkVgGraphModel, EventModel, vtkVgEventModel);
vtkCxxSetObjectMacro(vtkVgGraphModel, ActivityManager, vtkVgActivityManager);

vtkCxxSetObjectMacro(vtkVgGraphModel, EventFilter, vtkVgEventFilter);

#define vtkCreateMacro(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//----------------------------------------------------------------------------
struct vtkVgGraphModel::vtkInternal
{
  vtkInternal();
  ~vtkInternal();

  void reset();

  vtkTimeStamp UpdateTime;

  const double EventZOffset;
  const double ActivityZOffset;
  const double EventZThickness;
  const double ActivityZThickness;
  const double ZScaleFactor;

  bool Initialized;

  typedef std::map<vtkIdType, vtkIdType> EntityIdGraphVertexIdMap;
  typedef std::map<vtkIdType, vtkIdType> GraphVertexIdEntityTypeMap;

  typedef EntityIdGraphVertexIdMap::const_iterator   EntityIdGraphVertexIdConstItr;
  typedef GraphVertexIdEntityTypeMap::const_iterator VertexIdEntityTypeConstItr;

  EntityIdGraphVertexIdMap TrackIdVertexIdMap;
  EntityIdGraphVertexIdMap EventIdVertexIdMap;
  EntityIdGraphVertexIdMap ActivityIdVertexIdMap;

  // 0 is for event-track, 1 is for event - activity, 2 is event-event.
  std::map<vtkIdType, vtkIdType>  EdgeIdEdgeTypeMap;
  std::map<vtkIdType, double>     EdgeProbabilityMap;

  GraphVertexIdEntityTypeMap VertexIdEntityTypeMap;
};

//-----------------------------------------------------------------------------
vtkVgGraphModel::vtkInternal::vtkInternal() :
  UpdateTime(),
  EventZOffset(50.0),
  ActivityZOffset(500.0),
  EventZThickness(400.0),
  ActivityZThickness(200.0),
  ZScaleFactor(10.0),
  Initialized(false)
{
}

//-----------------------------------------------------------------------------
vtkVgGraphModel::vtkInternal::~vtkInternal()
{
}

//-----------------------------------------------------------------------------
void vtkVgGraphModel::vtkInternal::reset()
{
  this->TrackIdVertexIdMap.clear();
  this->EventIdVertexIdMap.clear();
  this->ActivityIdVertexIdMap.clear();

  this->EdgeIdEdgeTypeMap.clear();
  this->EdgeProbabilityMap.clear();
}

//-----------------------------------------------------------------------------
vtkVgGraphModel::vtkVgGraphModel()
{
  this->EventFilter         = 0;
  this->TrackModel          = 0;
  this->EventModel          = 0;
  this->ActivityManager     = 0;
  this->Graph               = 0;

  this->CurrentZOffsetMode  = ZOffsetUsingNormalcy;

  this->Internal            = new vtkInternal;

  this->CurrentTimeStamp.SetToMinTime();
}

//-----------------------------------------------------------------------------
vtkVgGraphModel::~vtkVgGraphModel()
{
  this->SetEventFilter(0);
  this->SetTrackModel(0);
  this->SetEventModel(0);
  this->SetActivityManager(0);

  if (this->Graph)
    {
    this->Graph->Delete();
    }

  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgGraphModel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgGraphModel::Initialize(const vtkVgTimeStamp& timeStamp)
{
  this->Internal->reset();

  if (!this->EventModel || !this->TrackModel || !this->ActivityManager)
    {
    vtkErrorMacro("Sub-models must be set before initialization.");
    return;
    }

  if (this->Graph)
    {
    this->Graph->Delete();
    }

  this->Graph = vtkMutableDirectedGraph::New();
  vtkMutableDirectedGraph* graph =
    vtkMutableDirectedGraph::SafeDownCast(this->Graph);

  vtkCreateMacro(vtkPoints, points);

  double eventTimeRange = 1;
  vtkVgTimeStamp minTime, maxTime;
  if (this->CurrentZOffsetMode)
    {
    this->EventModel->InitEventTraversal();
    vtkVgEvent* e = this->EventModel->GetNextEvent().GetEvent();
    if (e)
      {
      minTime = e->GetStartFrame();
      maxTime = e->GetStartFrame();
      while ((e = this->EventModel->GetNextEvent().GetEvent()))
        {
        if (e->GetStartFrame() < minTime)
          {
          minTime = e->GetStartFrame();
          }
        else if (maxTime < e->GetStartFrame())
          {
          maxTime = e->GetStartFrame();
          }
        }
      }

    // want non-zero to divide by below in case MaxTime == MinTime
    if (maxTime != minTime)
      {
      eventTimeRange = maxTime.GetTimeDifferenceInSecs(minTime);
      }
    }

  vtkVgEventInfo info;
  this->EventModel->InitEventTraversal();
  while ((info = this->EventModel->GetNextEvent()).GetEvent())
    {
    vtkVgEvent* e = info.GetEvent();
    if (this->Internal->Initialized && this->EventFilter)
      {
      int bestType = this->EventFilter->GetBestClassifier(e);
      if (bestType < 0 || !info.GetDisplayEvent())
        {
        continue;
        }
      }

    double eventPosition[3] = { 0.0 };

    std::vector<vtkIdType> trackVertexIds;

    int numTracks = this->EventModel->GetUseTrackGroups() ?
                    e->GetNumberOfTrackGroups() : e->GetNumberOfTracks();
    int numDisplayedTracks = 0;
    for (int i = 0; i < numTracks; ++i)
      {
      vtkVgTrackDisplayData tdd = this->EventModel->GetTrackDisplayData(e, i);
      if (tdd.NumIds == 0)
        {
        continue;
        }

      ++numDisplayedTracks;

      // display the last point returned from this track within the event,
      // UNLESS timeStamp is before the start of the event. This is primarily
      // (only?) for full volume mode... don't use end as location if we are
      // before the event
      vtkIdType pointId;
      if (timeStamp < e->GetStartFrame())
        {
        pointId = tdd.IdsStart[0];
        }
      else
        {
        pointId = tdd.IdsStart[tdd.NumIds - 1];
        }

      double point[3];
      e->GetPoints()->GetPoint(pointId, point);

      // Add a vertex at just determined track location (if doesn't already exist).
      if (this->Internal->TrackIdVertexIdMap.find(pointId) ==
          this->Internal->TrackIdVertexIdMap.end())
        {
        vtkIdType trackGraphId = graph->AddVertex();
        this->Internal->TrackIdVertexIdMap[pointId] = trackGraphId;
        this->Internal->VertexIdEntityTypeMap[trackGraphId] = 0;
        points->InsertNextPoint(point);
        }

      trackVertexIds.push_back(this->Internal->TrackIdVertexIdMap[pointId]);
      }

    if (numDisplayedTracks == 0) // nothing to display for the event
      {
      continue;
      }

    // Get x,y component of event position
    e->GetDisplayPosition(timeStamp, eventPosition);

    // Compute z component of event position
    if (this->CurrentZOffsetMode == vtkVgGraphModel::ZOffsetUsingNormalcy)
      {
      eventPosition[2] = this->Internal->EventZOffset +
        e->GetNormalcy(e->GetActiveClassifierType())  *
        this->Internal->ZScaleFactor;
      }
    else if (this->CurrentZOffsetMode == vtkVgGraphModel::ZOffsetUsingStartTime)
      {
      eventPosition[2] = this->Internal->EventZOffset +
        this->Internal->EventZThickness *
        e->GetStartFrame().GetTimeDifferenceInSecs(minTime) / eventTimeRange;
      }
    else
      {
      vtkErrorMacro("ERROR: ZOffsetMode " << this->CurrentZOffsetMode
                    << " is not implemented\n");
      return;
      }

    // Add event vertex at the midpoint of supporting track positions.
    vtkIdType eventGraphId = graph->AddVertex();
    this->Internal->EventIdVertexIdMap[e->GetId()] = eventGraphId;
    this->Internal->VertexIdEntityTypeMap[eventGraphId] = 1;
    points->InsertNextPoint(eventPosition);

    // Add edges from event to supporting track verts.
    for (size_t i = 0; i < trackVertexIds.size(); ++i)
      {
      vtkEdgeType edgeType = graph->AddEdge(eventGraphId, trackVertexIds[i]);
      this->Internal->EdgeIdEdgeTypeMap[edgeType.Id] = 0;
      }
    }

  // Add event link edges.
  for (int i = 0, end = this->EventModel->GetNumberOfEventLinks(); i < end; ++i)
    {
    EventLink link;
    this->EventModel->GetEventLink(i, link);

    // Add the edge only if both event vertices are in the graph.
    vtkInternal::EntityIdGraphVertexIdMap::iterator srcItr =
      this->Internal->EventIdVertexIdMap.find(link.Source);
    if (srcItr == this->Internal->EventIdVertexIdMap.end())
      {
      continue;
      }

    vtkInternal::EntityIdGraphVertexIdMap::iterator destItr =
      this->Internal->EventIdVertexIdMap.find(link.Destination);
    if (destItr == this->Internal->EventIdVertexIdMap.end())
      {
      continue;
      }

    vtkEdgeType edgeType = graph->AddEdge(srcItr->second, destItr->second);
    this->Internal->EdgeIdEdgeTypeMap[edgeType.Id] = 2;
    this->Internal->EdgeProbabilityMap[edgeType.Id] = link.Probability;
    }

  // Add activity graph data.
  // initialize to times that aren't desired min/max such that we don't have to
  // check for validity of the min/max time variables
  double activityTimeRange = 1;
  vtkVgTimeStamp activityMinTime = maxTime, activityMaxTime = minTime;
  int numActivities = this->ActivityManager->GetNumberOfActivities();
  if (this->CurrentZOffsetMode)
    {
    for (int i = 0; i < numActivities; ++i)
      {
      vtkVgActivity* a = this->ActivityManager->GetActivity(i);
      // Add all events in this activity
      int numEvents = a->GetNumberOfEvents();

      if (numEvents > 0) // JUST IN CASE!!!!
        {
        vtkVgTimeStamp minEventTime = a->GetEvent(0)->GetStartFrame();
        for (int j = 1; j < numEvents; ++j)
          {
          vtkVgEvent* e = a->GetEvent(j);

          if (e->GetStartFrame() < minEventTime)
            {
            minEventTime = e->GetStartFrame();
            }
          }
        if (minEventTime < activityMinTime)
          {
          activityMinTime = minEventTime;
          }
        if (activityMaxTime < minEventTime)
          {
          activityMaxTime = minEventTime;
          }
        }
      }

    // want non-zero to divide by below in case MaxTime == MinTime
    if (activityMaxTime != activityMinTime)
      {
      activityTimeRange =
        activityMaxTime.GetTimeDifferenceInSecs(activityMinTime);
      }
    }

  for (int i = 0; i < numActivities; ++i)
    {
    double maxEventNormalcy  = VTK_DOUBLE_MIN;
    double avg[3] = {0.0, 0.0, 0.0};

    vtkVgActivity* a = this->ActivityManager->GetActivity(i);
    // should we display the activity?
    if (!this->ActivityManager->GetActivityDisplayState(i) ||
        !this->ActivityManager->GetActivityFilteredDisplayState(i) ||
        this->ActivityManager->ActivityIsFiltered(a))
      {
      continue;
      }

    std::vector<vtkIdType> eventVertexIds;

    // Add all events in this activity
    int numEvents = a->GetNumberOfEvents();
    if (numEvents == 0) // shouldn't happen... but???
      {
      continue;
      }

    vtkVgTimeStamp minEventTime;
    for (int j = 0; j < numEvents; ++j)
      {
      double point[3];

      vtkVgEvent* e = a->GetEvent(j);

      int bestType = e->GetActiveClassifierType();
      if (this->Internal->Initialized && this->EventFilter)
        {
        bestType = this->EventFilter->GetBestClassifier(e);
        }
      if (bestType >= 0)
        {
        double eventNormalcy = e->GetNormalcy(bestType);

        if (eventNormalcy > maxEventNormalcy)
          {
          maxEventNormalcy = eventNormalcy;
          }
        }

      // all events within activity contribute, even if not currently displayed
      vtkIdType eventVertexId = this->GetEventGraphVertexId(e->GetId());
      if (eventVertexId != -1)
        {
        eventVertexIds.push_back(eventVertexId);
        points->GetPoint(eventVertexId, point);
        }
      else
        {
        // currently no vertex, so need to get from event
        e->GetDisplayPosition(timeStamp, point);
        }
      avg[0] += point[0];
      avg[1] += point[1];

      if (!minEventTime.IsValid() || e->GetStartFrame() < minEventTime)
        {
        minEventTime = e->GetStartFrame();
        }
      }

    // display activity if show always or time during activity
    vtkVgTimeStamp activityStart, activityEnd;
    a->GetActivityFrameExtents(activityStart, activityEnd);
    if (a->GetShowAlways() ||
        (timeStamp <= activityEnd && activityStart <= timeStamp))
      {
      avg[0] /= numEvents;
      avg[1] /= numEvents;

      if (this->CurrentZOffsetMode == vtkVgGraphModel::ZOffsetUsingNormalcy)
        {
        avg[2] = this->Internal->ActivityZOffset +
                 maxEventNormalcy * this->Internal->ZScaleFactor;
        }
      else if (this->CurrentZOffsetMode == vtkVgGraphModel::ZOffsetUsingStartTime)
        {
        avg[2] = this->Internal->ActivityZOffset +
                 this->Internal->ActivityZThickness *
                 minEventTime.GetTimeDifferenceInSecs(activityMinTime) / activityTimeRange;
        }
      else
        {
        vtkErrorMacro("ERROR: ZPositionMode " << this->CurrentZOffsetMode
                      << " is not implemented\n");
        return;
        }

      vtkIdType activityVertexId = graph->AddVertex();
      vtkIdType activityId = i;
      this->Internal->ActivityIdVertexIdMap[activityId] = activityVertexId;
      this->Internal->VertexIdEntityTypeMap[activityVertexId] = 2;

      for (size_t eid = 0; eid < eventVertexIds.size(); ++eid)
        {
        vtkEdgeType edgeType = graph->AddEdge(activityVertexId, eventVertexIds[eid]);
        this->Internal->EdgeIdEdgeTypeMap[edgeType.Id] = 1;
        }

      points->InsertPoint(activityVertexId, avg);
      }
    }

  graph->SetPoints(points);

  this->Internal->Initialized = true;
}

//-----------------------------------------------------------------------------
int vtkVgGraphModel::Update(const vtkVgTimeStamp& timeStamp,
  const vtkVgTimeStamp* vtkNotUsed(referenceFrameTimeStamp)/*=0*/)
{
  if (this->CurrentTimeStamp == timeStamp &&
      this->Internal->UpdateTime > this->GetMTime() && !this->Internal->Initialized)
    {
    return VTK_OK;
    }

  this->CurrentTimeStamp = timeStamp;

  this->Internal->UpdateTime.Modified();

  // Update graph model here...
  this->Initialize(timeStamp);

  // Let the representation (if listening) know it needs to update
  this->InvokeEvent(vtkCommand::UpdateDataEvent);

  return VTK_OK;
}

//-----------------------------------------------------------------------------
unsigned long vtkVgGraphModel::GetUpdateTime()
{
  return this->Internal->UpdateTime.GetMTime();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgGraphModel::GetTrackGraphVertexId(const vtkIdType& trackId)
{
  vtkInternal::EntityIdGraphVertexIdConstItr constItr =
    this->Internal->TrackIdVertexIdMap.find(trackId);
  if (constItr != this->Internal->TrackIdVertexIdMap.end())
    {
    return constItr->second;
    }

  return -1;
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgGraphModel::GetEventGraphVertexId(const vtkIdType& eventId)
{
  vtkInternal::EntityIdGraphVertexIdConstItr constItr =
    this->Internal->EventIdVertexIdMap.find(eventId);
  if (constItr != this->Internal->EventIdVertexIdMap.end())
    {
    return constItr->second;
    }

  return -1;
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgGraphModel::GetActivityGraphVertexId(const vtkIdType& activityId)
{
  vtkInternal::EntityIdGraphVertexIdConstItr constItr =
    this->Internal->TrackIdVertexIdMap.find(activityId);
  if (constItr != this->Internal->ActivityIdVertexIdMap.end())
    {
    return constItr->second;
    }

  return -1;
}

//-----------------------------------------------------------------------------
const std::map<vtkIdType, vtkIdType>& vtkVgGraphModel::GetTracksVerticesMap()
{
  return this->Internal->TrackIdVertexIdMap;
}

//-----------------------------------------------------------------------------
const std::map<vtkIdType, vtkIdType>& vtkVgGraphModel::GetEventsVerticesMap()
{
  return this->Internal->EventIdVertexIdMap;
}

//-----------------------------------------------------------------------------
const std::map<vtkIdType, vtkIdType>& vtkVgGraphModel::GetActivitiesVerticesMap()
{
  return this->Internal->ActivityIdVertexIdMap;
}

//-----------------------------------------------------------------------------
const std::map<vtkIdType, vtkIdType>& vtkVgGraphModel::GetEdgeIdToEdgeTypeMap()
{
  return this->Internal->EdgeIdEdgeTypeMap;
}

//-----------------------------------------------------------------------------
const std::map<vtkIdType, double>& vtkVgGraphModel::GetEdgeProbabilityMap()
{
  return this->Internal->EdgeProbabilityMap;
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgGraphModel::GetVertexEntityType(vtkIdType vertexId)
{
  vtkInternal::VertexIdEntityTypeConstItr constItr =
    this->Internal->VertexIdEntityTypeMap.find(vertexId);

  if (constItr != this->Internal->VertexIdEntityTypeMap.end())
    {
    return constItr->second;
    }

  return -1;
}
