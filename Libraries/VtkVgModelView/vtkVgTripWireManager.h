// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTripWireManager_h
#define __vtkVgTripWireManager_h

#include "vtkObject.h"

#include "vtkVgTimeStamp.h"

#include <vector>

#include <vgExport.h>

class vtkIdList;
class vtkImplicitBoolean;
class vtkPoints;
class vtkPolyLine;
class vtkVgEventModel;
class vtkVgTrack;
class vtkVgTrackModel;

class VTKVG_MODELVIEW_EXPORT vtkVgTripWireManager : public vtkObject
{
public:
  static vtkVgTripWireManager* New();
  vtkTypeMacro(vtkVgTripWireManager, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Structure describing an intersection event
  struct IntersectionInfo
    {
    vtkIdType TrackId;         // when available, set the track id for the intersection
    vtkVgTimeStamp PreFrame;   // the "time" of the track point immediately before
    vtkVgTimeStamp PostFrame;  // the "time" of the track point immediately after
    int ClassifierType;        // Entering, Exiting, or TripWire
    double IntersectionPt[2];  // x, y of intersection
    vtkVgTimeStamp IntersectionTime;  // Interpolated "Time" at IntersectionPt
    };

  // Description:
  // Main "execute" function to check the status of all "trip wires" relative
  // to the tracks in the track model.
  void CheckTripWires();

  // Description:
  // Check the specified trip-wire for interesections against all tracks in the
  // TrackModel.
  void CheckTripWire(vtkIdType tripWireId, std::vector<IntersectionInfo>& intersections);

  // Description:
  //int ClassifierType, // Entering, Exiting, or TripWire
  //double IntersectionPt[2],  // x, y of intersection
  //vtkVgTimeStamp &IntersectionTime);  // Interpolated "Time" at IntersectionPt
  void CheckTrackSegment(double pt1[2], double p2[2],
                         const vtkVgTimeStamp& timeStamp1, const vtkVgTimeStamp& timeStamp2,
                         std::vector<IntersectionInfo>& intersections);

  // Description:
  // Make sure the event model is up to date regarding detected trip wire events
  void UpdateEventModel(const vtkVgTimeStamp& timestamp);

  // Description:
  // Add a trip-wire.  The id for the trip-wire can be specified as the final
  // argument; otherwise, an internally determined (unique) id will be
  // determined and returned.  A returned value of -1 indicates that the
  // requested id was already in use.
  vtkIdType AddTripWire(vtkPoints* loopPts, bool closedLoop, vtkIdType tripWireId = -1);
  bool RemoveTripWire(vtkIdType tripWireId);
  bool IsTripWire(vtkIdType tripWireId);
  bool RemoveAllTripWires();
  int GetNumberOfTripWires();
  //void InitTripWireTraversal();
  //vtkPolyLine *GetNextTripWire();

  // Description:
  // Set/Get trip wire enabled state
  void SetTripWireEnabled(vtkIdType tripWireId, bool state);
  bool GetTripWireEnabled(vtkIdType tripWireId);

  // Description:
  // Set/Get the TrackModel used to test against trip wires
  void SetTrackModel(vtkVgTrackModel* model);
  vtkGetObjectMacro(TrackModel, vtkVgTrackModel);

  // Description:
  // Set/Get the EventModel on/to which triggered events are managed
  void SetEventModel(vtkVgEventModel* model);
  vtkGetObjectMacro(EventModel, vtkVgEventModel);

  void SetPreTripDuration(const vtkVgTimeStamp& duration);
  vtkVgTimeStamp GetPreTripDuration()
    {
    return this->PreTripDuration;
    }

  // Description:
  // Set/Get the id for EnteringRegion events.  Defaults to -1 because should
  // be set according to its application
  vtkSetMacro(EnteringRegionId, int);
  vtkGetMacro(EnteringRegionId, int);

  // Description:
  // Set/Get the id for ExitingRegion events.  Defaults to -1 because should
  // be set according to its application
  vtkSetMacro(ExitingRegionId, int);
  vtkGetMacro(ExitingRegionId, int);

  // Description:
  // Set/Get the id for TripWire events.  Defaults to -1 because should
  // be set according to its application
  vtkSetMacro(TripWireId, int);
  vtkGetMacro(TripWireId, int);

  // Description:
  // Set/Get the EventIdCounter, used to set the ids for triggered events.
  // The value monotically increases, and really should only be set once.  This
  // is temporary (imagine it should really have ids in sync with other
  // events).  Defaults to 1,000,000.
  // be set according to its application
  vtkSetMacro(EventIdCounter, int);
  vtkGetMacro(EventIdCounter, int);

  // Description:
  // Check the indicated track against all existing trip wires, and return
  // whether any trip, enter, or exit events occur.  Note, each of the bool
  // values acts as input as well, indicting whether we're looking for that
  // result (so we know if we can stop as soon as found true).
  void CheckTrackForTripWireEvents(vtkVgTrack* track, bool& tripEvent,
                                   bool& enterEvent, bool& exitEvent);

//BTX
protected:
  vtkVgTripWireManager();
  ~vtkVgTripWireManager();

  vtkVgTimeStamp PreTripDuration;

  vtkVgTrackModel* TrackModel;
  vtkVgEventModel* EventModel;

  int EnteringRegionId;
  int ExitingRegionId;
  int TripWireId;

  int EventIdCounter;

  class vtkInternal;
  vtkInternal* Internals;

//ETX
};

#endif
