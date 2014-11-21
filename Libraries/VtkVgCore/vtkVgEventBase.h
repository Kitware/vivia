/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgEventBase_h
#define __vtkVgEventBase_h

#include <vtkObject.h>

#include "vtkVgMetaObject.h"
#include "vtkVgTimeStamp.h"

#include <map>
#include <vector>

#include <vgExport.h>

class vtkIdList;
class vtkPoints;

class VTKVG_CORE_EXPORT vtkVgEventTrackInfoBase : public vtkVgMetaObject
{
  vtkDeclareMetaObject(vtkVgEventTrackInfoBase);

public:
  vtkVgEventTrackInfoBase(vtkIdType trackId, const vtkVgTimeStamp& startFrame,
                          const vtkVgTimeStamp& endFrame);
  vtkVgEventTrackInfoBase(const vtkVgEventTrackInfoBase& fromTrackInfo);
  virtual ~vtkVgEventTrackInfoBase() {};

  virtual vtkVgEventTrackInfoBase* Clone() const;

  virtual const char* CheckValid() const;

  vtkIdType TrackId;
  vtkVgTimeStamp StartFrame;
  vtkVgTimeStamp EndFrame;

protected:
  const char* CheckBaseValid() const;
};

class VTKVG_CORE_EXPORT vtkVgEventBase : public vtkObject
{
public:
  enum EventFlags
    {
    EF_UserCreated  = 1 << 0,
    EF_Modifiable   = 1 << 1,
    EF_Dirty        = 1 << 2,
    EF_Starred      = 1 << 3
    };

  enum DisplayFlags
    {
    DF_TrackEvent          = 1 << 0,
    DF_RegionEvent         = 1 << 1,
    DF_TrackAndRegionEvent = DF_TrackEvent | DF_RegionEvent,
    DF_Selected            = 1 << 2
    };

  struct RegionPoint
    {
    RegionPoint() : X(0.0), Y(0.0) {}
    RegionPoint(double x, double y) : X(x), Y(y) {}

    double X;
    double Y;
    };

  typedef std::vector<RegionPoint> Region;

  // Description:
  // Standard VTK functions.
  static vtkVgEventBase* New();
  vtkTypeMacro(vtkVgEventBase, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the points used for the "region" that may or may not be present at
  // frames during the event.
  vtkGetObjectMacro(RegionPoints, vtkPoints);
  void SetRegionPoints(vtkPoints* points);

  void AddRegion(const vtkVgTimeStamp& timeStamp,
                 vtkIdType numberOfRegionPts, double* regionPts);
  void SetRegion(const vtkVgTimeStamp& timeStamp,
                 vtkIdType numberOfRegionPts, double* regionPts);
  void GetRegion(const vtkVgTimeStamp& timeStamp,
                 vtkIdType& npts, vtkIdType*& pts);
  bool GetRegionCenter(const vtkVgTimeStamp& timeStamp, double* center,
                       bool interpolated);
  bool GetRegionAtOrAfter(vtkVgTimeStamp& timeStamp,
                          vtkIdType& npts, vtkIdType*& pts);

  void AddRegion(const vtkVgTimeStamp& timeStamp, const Region& region);
  Region GetRegion(const vtkVgTimeStamp& timeStamp) const;
  std::map<vtkVgTimeStamp, Region> GetRegions() const;

  // Description:
  // Get the closest region with time less than or equal to timestamp, or the
  // region after if no other region is available. Returns false if the region
  // found does not have the same timestamp as requested.
  bool GetClosestDisplayRegion(vtkVgTimeStamp& timeStamp,
                               vtkIdType& npts, vtkIdType*& pts);

  // Description:
  // Remove all regions from the event.
  void ClearRegions();

  // Description:
  // Return the # of regions.
  unsigned int GetNumberOfRegions() const;

  // Description:
  // Methods for efficient traversal of event regions.
  void InitRegionTraversal();
  bool GetNextRegion(vtkVgTimeStamp& timeStamp, vtkIdType& npts, vtkIdType*& pts);

  // Description:
  // Add a classifier, with probability and normalcy, to this event.  If don't
  // have probability or normalcy, set to a constant value (same for all events)
  // so that scoring works to set a default "Active" classifier
  void AddClassifier(int type, double probability = 0, double normalcy = 0);

  // Description:
  // Remove a classifier.
  bool RemoveClassifier(int type);

  // Description:
  // Remove all previously added classifiers.
  void ResetClassifiers();

  // Description:
  // Return the # of classifiers for this event
  unsigned int GetNumberOfClassifiers();

  // Description:
  // Returns a std::set of all classifier types for this event
  std::vector<int> GetClassifierTypes();

  // Description:
  // Methods for iterator traversal of the classifiers.  Note, both the Init
  // as well as NextClassifier fns return wether the current item is valid.
  bool InitClassifierTraversal();
  bool NextClassifier();
  int GetClassifierType();
  double GetClassifierProbability();
  double GetClassifierNormalcy();
  double GetClassifierScore();

  // Description:
  // Set/Get the probability for the specified classifier type.  This could
  // update the ActiveClassifier, if the "score" (1 + prob) / (1 + normalcy)
  // becomes the largest (if the user didn't already specify a particular
  // classifier as active).
  void SetProbability(int classifierType, double probability);
  double GetProbability(int classifierType);

  // Description:
  // Set/Get the normalcy for the specified classifier type.  This could
  // update the ActiveClassifier, if the "score" (1 + prob) / (1 + normalcy)
  // becomes the largest (if the user didn't already specify a particular
  // classifier as active).
  void SetNormalcy(int classifierType, double normalcy);
  double GetNormalcy(int classifierType);

  // Description:
  // Does this event use the specified classifier type
  bool HasClassifier(int classifierType);

  // Description:
  // If the classifier type exists, set it as the "Active" classifier
  void SetActiveClassifier(int classifierType);

  // Description:
  // Clear a user specified active classifier and set the active classifier
  // to the classifier with the largest score
  void ResetActiveClassifier();

  // Description:
  // Get the Type/Normalcy/Probability of the "Active" classifier
  int GetActiveClassifierType();
  double GetActiveClassifierNormalcy();
  double GetActiveClassifierProbability();

  vtkSetMacro(Id, vtkIdType);
  vtkGetMacro(Id, vtkIdType);

  // Description:
  // Set/Get the "Name" of the event
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  // Description:
  // Set/Get a "Note" that is to be associated with this event
  vtkSetStringMacro(Note);
  vtkGetStringMacro(Note);

  void SetStartFrame(const vtkVgTimeStamp& timeStamp);
  vtkVgTimeStamp GetStartFrame() const
    {
    return this->StartFrame;
    }

  void SetEndFrame(const vtkVgTimeStamp& timeStamp);
  vtkVgTimeStamp GetEndFrame() const
    {
    return this->EndFrame;
    }

  // Description:
  // Return true if specified time is within event's time range (start, end).
  bool IsTimeWithinExtents(const vtkVgTimeStamp& timeStamp);

  // Description:
  // Add a new track Id into the list of tracks related with the event. Do NOT
  // call this from a subclass (we overload in subclasses to give warning...
  // if did force call, bad things (crash) likely later on.
  virtual void AddTrack(vtkIdType trackId, vtkVgTimeStamp& startFrame,
                        vtkVgTimeStamp& endFrame);

  // Description:
  // Add a new track info structure into the list of tracks related with the
  // event. Do NOT call this from a subclass (we overload in subclasses to give
  // warning... if did force call, bad things (crash) likely later on.
  void AddTrack(vtkVgEventTrackInfoBase* trackInfo);

  // Description:
  // Remove track at the specified index
  void RemoveTrack(unsigned int index);

  // Description:
  // Swap tracks at given indices
  void SwapTracks(unsigned int index1, unsigned int index2);

  // Description:
  // Change the track id in the given slot without modifying the start or end
  // time
  void ReplaceTrack(unsigned int index, vtkIdType trackId);

  // Description:
  // Given a valid index, fill in track and its related parameters.
  // \TODO: This interface might change in the future.
  void GetTrack(unsigned int index, vtkIdType& trackId,
                vtkVgTimeStamp& startFrame,
                vtkVgTimeStamp& endFrame);

  // Description:
  // Return trackId at the specified index
  vtkIdType GetTrackId(unsigned int index) const;

  // Description:
  // Return track info at the specified index
  vtkVgEventTrackInfoBase* GetTrackInfo(unsigned int index) const;

  // Description:
  // Get the # of tracks.
  unsigned int GetNumberOfTracks() const;

  // Description:
  // Get the # of track groups (tracks with same id can be grouped for display)
  unsigned int GetNumberOfTrackGroups() const;

  // Description:
  // Set track frame info
  void SetTrackStartFrame(unsigned int trackIndex,
                          const vtkVgTimeStamp& startFrame);
  void SetTrackEndFrame(unsigned int trackIndex,
                        const vtkVgTimeStamp& endFrame);

  void DeepCopy(vtkVgEventBase* src, bool appendRegionPoints = false);

  // Description:
  // Merge with another event, leaving the second event unmodified
  bool Merge(vtkVgEventBase* other);

  // Description:
  // Set/Get the status of the event (adjudicated, excluded, or otherwise)
  vtkSetMacro(Status, int);
  vtkGetMacro(Status, int);

  // Description:
  // Set/Get the flags of the event
  int GetFlags(int mask) const
    { return this->Flags & mask; }

  void SetFlags(int mask)
    { this->Flags |= mask; }

  void ClearFlags(int mask)
    { this->Flags &= ~mask; }

  bool IsUserCreated()
    { return (this->Flags & EF_UserCreated) != 0; }

  bool IsModifiable()
    { return (this->Flags & EF_Modifiable) != 0; }

  bool IsDirty()
    { return (this->Flags & EF_Dirty) != 0; }

  bool IsStarred()
    { return (this->Flags & EF_Starred) != 0; }

  // Description:
  // Set/Get the display flags of the event
  vtkSetMacro(DisplayFlags, unsigned);
  vtkGetMacro(DisplayFlags, unsigned);

  // Description:
  // Since, for now, this is just to support trip wire events, I'm
  // going to keep the implementation specific to that.  The trip "property"
  // is a single point (and time, though actually don't need that right now).
  // Note, the IsTripEvent state is set by setting the info and clearing it.
  bool IsTripEvent()
    {
    return this->TripEventFlag;
    }
  void ClearTripEvent();
  void SetTripEventInfo(double tripLocation[3], const vtkVgTimeStamp& tripTime);
  vtkGetVector3Macro(TripEventPosition, double);
  vtkVgTimeStamp GetTripEventTime()
    {
    return this->TripEventTime;
    }

  // Description:
  // Replace event definition based on input event
  virtual void UpdateEvent(vtkVgEventBase* fromEvent);

  // Description:
  // Make this event a copy of the input event.  Note that it will be set to
  // use the same RegionPoints as the event being cloned.  Thus this is similar
  // to a ShallowCopy, though the Id is NOT copied as part of the cloning
  void CloneEvent(vtkVgEventBase* fromEvent);

protected:
  vtkVgEventBase();
  ~vtkVgEventBase();

  vtkSetVector3Macro(TripEventPosition, double);

  void ClearTracks();
  virtual void CopyTracks(std::vector<vtkVgEventTrackInfoBase*>& tracks);

  // Recalculate start and end times based on component tracks
  void UpdateEventFrameBounds();

  // Copy almost everything but the RegionPoints, which is handled differently
  // depending on whether calling DeepCopy, UpdateEvent, or CloneEvent.
  // The copyRegionPtIds flag indicates whether the "internal" RegionPtIds
  // should be copied. The default (true) is used by both DeepCopy and
  // CloneEvent.  UpdateEvent, which does not assume an exactly matching
  // RegionPoints, sets the flag to false when calling.
  // The calling fn should handle calling Modified()
  void CopyEventInfo(vtkVgEventBase* fromEvent, bool copyRegionPtIds = true);

  std::vector<vtkVgEventTrackInfoBase*> Tracks;

  vtkPoints* RegionPoints;

  vtkIdType Id;
  char* Name;
  char* Note;

  vtkVgTimeStamp StartFrame;
  vtkVgTimeStamp EndFrame;

  int Status;
  unsigned Flags;
  unsigned DisplayFlags;

  bool TripEventFlag;
  double TripEventPosition[3];
  vtkVgTimeStamp TripEventTime;

private:
  vtkVgEventBase(const vtkVgEventBase&); // Not implemented.
  void operator=(const vtkVgEventBase&);  // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgEventBase_h
