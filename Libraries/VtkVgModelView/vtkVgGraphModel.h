/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgGraphModel_h
#define __vtkVgGraphModel_h

#include "vtkVgModelBase.h"

// C++ includes.
#include <map>

#include <vgExport.h>

// Forward declarations.
class vtkVgActivityManager;
class vtkVgEventFilter;
class vtkVgEventModel;
class vtkVgTrackModel;

class vtkGraph;

class VTKVG_MODELVIEW_EXPORT vtkVgGraphModel : public vtkVgModelBase
{
public:

  enum ZOffsetMode
    {
    ZOffsetUsingNormalcy = 0,
    ZOffsetUsingStartTime
    };

  vtkVgClassMacro(vtkVgGraphModel);

  vtkTypeMacro(vtkVgGraphModel, vtkVgModelBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgGraphModel* New();

  // Description:
  // Specify a track model.
  void SetTrackModel(vtkVgTrackModel* tm);
  vtkGetObjectMacro(TrackModel, vtkVgTrackModel);

  // Description:
  // Specify an event model.
  void SetEventModel(vtkVgEventModel* em);
  vtkGetObjectMacro(EventModel, vtkVgEventModel);

  // Description:
  // Specify an activity manager.
  void SetActivityManager(vtkVgActivityManager* am);
  vtkGetObjectMacro(ActivityManager, vtkVgActivityManager);

  // Description:
  // Create and initialize graph structures using the sub-models.
  void Initialize(const vtkVgTimeStamp& timeStamp);

  virtual int Update(const vtkVgTimeStamp& timeStamp,
                     const vtkVgTimeStamp* referenceFrameTimeStamp);
  using Superclass::Update;

  // Description:
  // Get current frame (index) as of the most recent Update() call
  vtkGetMacro(CurrentFrame, vtkIdType);

  // Description:
  // Return the MTime for the last Update (that did something).
  virtual unsigned long GetUpdateTime();

  // Description:
  // Get the vtkGraph built by the latest update of the model.
  vtkGetObjectMacro(Graph, vtkGraph);

  // Description:
  // Set the mode to be used for Z positioning.
  vtkSetMacro(CurrentZOffsetMode, vtkIdType);
  vtkGetMacro(CurrentZOffsetMode, vtkIdType);

  // Description:
  // Set the event filter that this representation will use.
  virtual void SetEventFilter(vtkVgEventFilter* filter);
  vtkGetObjectMacro(EventFilter, vtkVgEventFilter);

  // Description:
  // Given a track id return graph vertex id.
  // Return -1 if not found.
  virtual vtkIdType GetTrackGraphVertexId(const vtkIdType& trackId);

  // Description:
  // Given a track id return graph vertex id.
  // Return -1 if not found.
  virtual vtkIdType GetEventGraphVertexId(const vtkIdType& eventId);

  // Description:
  // Given a track id return graph vertex id.
  // Return -1 if not found.
  virtual vtkIdType GetActivityGraphVertexId(const vtkIdType& activityId);

  // Description:
  // Return map of tracks contained in the graph.
  virtual const std::map<vtkIdType, vtkIdType>& GetTracksVerticesMap();

  // Description:
  // Return map of events contained in the graph.
  virtual const std::map<vtkIdType, vtkIdType>& GetEventsVerticesMap();

  // Description:
  // Return map of activities contained in the graph.
  virtual const std::map<vtkIdType, vtkIdType>& GetActivitiesVerticesMap();

  // Description:
  // Return map of edge id to edge type.
  // 0 is track-event edge, 1 is event-activity edge.
  virtual const std::map<vtkIdType, vtkIdType>& GetEdgeIdToEdgeTypeMap();

  // Description:
  // Return map of edge id to edge probability.
  virtual const std::map<vtkIdType, double>& GetEdgeProbabilityMap();

  virtual vtkIdType GetVertexEntityType(vtkIdType vertexId);


private:
  vtkVgGraphModel(const vtkVgGraphModel&); // Not implemented.
  void operator=(const vtkVgGraphModel&);  // Not implemented.

  vtkVgGraphModel();
  ~vtkVgGraphModel();

  vtkVgTrackModel*           TrackModel;
  vtkVgEventModel*           EventModel;
  vtkVgActivityManager*      ActivityManager;

  vtkIdType                 CurrentFrame;

  vtkGraph*                  Graph;

  vtkIdType                 CurrentZOffsetMode;

  struct vtkInternal;
  vtkInternal*               Internal;

  vtkVgEventFilter*          EventFilter;
};

#endif // __vtkVgGraphModel_h
