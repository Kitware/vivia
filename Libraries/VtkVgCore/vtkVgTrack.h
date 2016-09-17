/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgTrack_h
#define __vtkVgTrack_h

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkDenseArray.h>

#include "vtkVgSetGet.h"
#include "vtkVgTimeStamp.h"

#include <string>
#include <vector>

#include <vgExport.h>

#include "vtkVgGeoCoord.h"

class vtkIdList;
class vtkPoints;

class vtkVgScalars;

class VTKVG_CORE_EXPORT vtkVgTrackDisplayData
{
public:
  vtkVgTrackDisplayData() : IdsStart(0), NumIds(0) {}

  vtkIdType* IdsStart;
  vtkIdType NumIds;

  std::vector<double> Scalars;
};

class VTKVG_CORE_EXPORT vtkVgTrack : public vtkObject
{
public:
  enum TrackFlags
    {
    TF_Modifiable  = 1 << 0,
    TF_Starred     = 1 << 1,
    TF_UserCreated = 1 << 2
    };

  // Description:
  // Standard VTK functions.
  static vtkVgTrack* New();
  vtkTypeMacro(vtkVgTrack, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The main identifier for the track
  vtkSetMacro(Id, vtkIdType);
  vtkGetMacro(Id, vtkIdType);

  // Description:
  // A name for the track
  vtkGetStringMacro(Name);
  vtkSetStringMacro(Name);

  // Description:
  // Set/Get a "Note" that is to be associated with this event
  vtkSetStringMacro(Note);
  vtkGetStringMacro(Note);

  // Description:
  // Set/Get the vtkPoints defining the track.  Note, the same vtkPoints
  // should be shared across all tracks
  void SetPoints(vtkPoints* points);
  vtkGetObjectMacro(Points, vtkPoints);

  // Description:
  // Allocate id arrays for specified number of frames.  Not required, but
  // more efficient for memory managemnet if known
  void Allocate(vtkIdType numberOfFrames);

  // Description:
  // Copy geometry data from another track, replacing any existing points
  void CopyData(vtkVgTrack* other);

  // Description:
  // Merge a second track into this one. Any overlapping portions of the other
  // track will be ignored. The second track is not modified.
  void Merge(vtkVgTrack* other);

  // Description:
  // Add a point (in world coordinates) to the track.  The timeStamp indicates
  // the frame/time at which the point occurs.  numberOfShellPts indciates how
  // many points in the following array of 3D points, which define the "shell"
  // around the tracked object at this point. Shell points may be supplied
  // either as a float* or vtkPoints* + start index. Caller should pass 0 in
  // fromShellPts if the shell point data is provided in shellPts. If both are
  // given, only the vtkPoints data will be used.
  void InsertNextPoint(const vtkVgTimeStamp& timeStamp, double point[2],
                       const vtkVgGeoCoord& geoCoord,
                       vtkIdType numberOfShellPts,
                       float* shellPts,
                       vtkPoints* fromShellPoints = 0,
                       vtkIdType fromShellPtsStart = -1,
                       bool interpolateShell = false);

  // Description:
  // Wrapper around InsertNextPoint to facilitate python wrapping by using a
  // vtkDenseArray instead of a C-style array with a size argument.
  void InsertNextPoint(const vtkVgTimeStamp& timeStamp, double point[2],
                       const vtkVgGeoCoord& geoCoord,
                       vtkDenseArray<double>* shellPts = 0);

  // Description: Set a point (in world coordinates) of the track, at the
  // specified timeStamp.  As opposed to InsertNextPoint, this can be used to
  // replace a previously set (or inserted) track point and can also be used on
  // a closed track. Shell points may be supplied either as a float* or
  // vtkPoints* + start index. Caller should pass 0 in fromShellPts if the shell
  // point data is provided in shellPts. If both are given, only the vtkPoints
  // data will be used.
  void SetPoint(const vtkVgTimeStamp& timeStamp, double point[2],
                vtkVgGeoCoord geoCoord,
                vtkIdType numberOfShellPts = 0, float* shellPts = 0,
                vtkPoints* fromShellPts = 0, vtkIdType fromShellPtsStart = -1);

  // Description:
  // Delete track point (and head) at the specified timeStamp.
  void DeletePoint(const vtkVgTimeStamp& timeStamp, bool isBatchDelete = false);

  // Description:
  // Indicate that the track definition is finished, that no addition points
  // will be added to the track.
  void Close();

  // Description:
  // Get the set of point ids that, with the Points (vtkPoints), defines
  // the geometry of the track
  vtkIdList* GetPointIds();

  // Description:
  // Get the id pointer + offset that forms the polyline defining the track
  // representation over the given time interval.
  vtkVgTrackDisplayData GetDisplayData(vtkVgTimeStamp start,
                                       vtkVgTimeStamp end);

  // Description:
  // Return the head identifier (typically a bbox around the tracked object).
  // If no HeadIdentifier was specified for the timeStamp within the specified
  // tolerance (default = 0.001), return id for a trackPoint if there is one
  // within tolerance (in trackPointId; -1 if one doesn't exist). The tolerance
  // is an absolute delta between the timeStamp parameter and the closest time
  // stamp in the track, regardless of whether the time stamps are specified in
  // seconds or frames.  Thus, the default tolerance is effectively 0 if only
  // frame numbers are available.
  void GetHeadIdentifier(const vtkVgTimeStamp& timeStamp, vtkIdType& npts,
                         vtkIdType*& pts, vtkIdType& trackPointId,
                         double tolerance = 0.001);

  // Description:
  // Return the geo coordinate for given time
  vtkVgGeoCoord GetGeoCoord(const vtkVgTimeStamp& timeStamp);

  // Description:
  // Returns true if the given frame contains interpolated head data
  bool GetFrameIsInterpolated(const vtkVgTimeStamp& timeStamp);

  //Description:
  // Get the frame index of the 1st frame in this track
  vtkVgTimeStamp GetStartFrame() const
    {
    return this->StartFrame;
    }

  //Description:
  // Get the frame index of the last frame in this track.  If the track hasn't
  // been closed, it returns the index of the last frame that was added.
  vtkVgTimeStamp GetEndFrame() const;

  // Description:
  // Returns whether or not there is a valid "start" to this track
  // (doesn't yet have to be closed)
  bool IsStarted()
    {
    return this->StartFrame.IsValid();
    }

  // Description:
  // Set/Get whether to interpolate missing points upon insertion (determined
  // via InterpolationSpacing)
  vtkBooleanMacro(InterpolateMissingPointsOnInsert, bool);
  vtkSetMacro(InterpolateMissingPointsOnInsert, bool);
  vtkGetMacro(InterpolateMissingPointsOnInsert, bool);

  // Description:
  // Set/Get the delta between "frames", or at least the delta we want to
  // interpolate at when inserting points.
  void SetInterpolationSpacing(const vtkVgTimeStamp& spacing);
  vtkVgTimeStamp GetInterpolationSpacing()
    {
    return this->InterpolationSpacing;
    }

  // Description:
  // Return the bounds of the entire track (xmin, xmax, ymin, ymax).
  double* GetFullBounds();

  // Description:
  // Get the bounds for this entire track as (xmin, xmax, ymin, ymax).
  void GetFullBounds(double bounds[4]);

  // Description:
  // Get the point of the track from the frame at timeStamp.
  // Returns true if point exists at timeStamp.
  bool GetPoint(const vtkVgTimeStamp& timeStamp, double pt[2],
                bool includeInterpolated = true);

  // Description:
  // Get the point of the track from the frame closest to input timeStamp.
  // Thus, if before the track, will retrieve beginning of track; if after
  // the track will retrieve endPt of the track.  Returns true if successful.
  bool GetClosestFramePt(const vtkVgTimeStamp& timeStamp, double pt[2]);

  // Description:
  // Get the point id of frame closest (equal to or before unless beginning
  // of track) to input time stamp
  vtkIdType GetClosestFramePtId(const vtkVgTimeStamp& timeStamp);

  // Description:
  // Get the point of the track (and associated TimeStamp) from the frame
  // closest to, but before, the input timeStamp.  Returns false if no point
  // before the input timeStamp.
  bool GetPriorFramePt(const vtkVgTimeStamp& timeStamp, double pt[2],
                       vtkVgTimeStamp& priorTimeStamp);

  // Description:
  // Get the point id (and TimeStamp) of pt immediately prior to input timeStamp.
  // Returns -1 if no point before the input timestamp
  vtkIdType GetPriorFramePtId(const vtkVgTimeStamp& timeStamp,
                              vtkVgTimeStamp& priorTimeStamp);

  // Description:
  // Get the timestamp of the closest adjacent non-interpolated frame
  bool GetFrameAtOrBefore(vtkVgTimeStamp& timeStamp);
  bool GetFrameAtOrAfter(vtkVgTimeStamp& timeStamp);

  vtkIdType GetNumberOfPathPoints();
  void InitPathTraversal();
  vtkIdType GetNextPathPt(vtkVgTimeStamp& timeStamp);

  // Description:
  // Get / set the normalcy value of the track.
  vtkSetMacro(Normalcy, double);
  vtkGetMacro(Normalcy, double);

  // Description:
  // Get / set the type of the track.
  vtkSetMacro(Type, int);
  vtkGetMacro(Type, int);

  // Description:
  // Set/Get the PVO classification for this track.  These must sum to 1;
  // to guarantee summing to 1, we normalize on input.
  void SetPVO(double fish, double scallop, double other);
  void SetPVO(double pvo[3]);
  vtkGetVector3Macro(PVO, double);

  enum enumTrackPVOType
    {
    Fish = 0,
    Scallop,
    Other,
    Unclassified
    };

  int GetBestPVOClassifier();

  // Description:
  // Support for per-track colors.
  vtkSetVector3Macro(Color, double);
  vtkGetVector3Macro(Color, double);

  vtkVgSetVector3Macro(CustomColor, double);
  vtkGetVector3Macro(CustomColor, double);

  vtkSetMacro(UseCustomColor, bool);
  vtkGetMacro(UseCustomColor, bool);
  vtkBooleanMacro(UseCustomColor, bool);

  // Description:
  // Set/Get the flags of the event
  int GetFlags(int mask) const
    { return this->Flags & mask; }

  void SetFlags(int mask)
    { this->Flags |= mask; }

  void ClearFlags(int mask)
    { this->Flags &= ~mask; }

  bool IsModifiable()
    { return (this->Flags & TF_Modifiable) != 0; }

  void SetModifiable(bool enable)
    { enable ? this->Flags |= TF_Modifiable : this->Flags &= ~TF_Modifiable; }

  bool IsStarred()
    { return (this->Flags & TF_Starred) != 0; }

  bool IsUserCreated()
    { return (this->Flags & TF_UserCreated) != 0; }

  void SetUserCreated(bool enable)
    { enable ? this->Flags |= TF_UserCreated : this->Flags &= ~TF_UserCreated; }

  enum DisplayFlags
    {
    DF_Normal       = 1 << 0,
    DF_Selected     = 1 << 1,
    DF_SceneElement = 1 << 2
    };

  // Description:
  // Set/Get the display flags of the track
  vtkSetMacro(DisplayFlags, unsigned char);
  vtkGetMacro(DisplayFlags, unsigned char);

  // Description:
  // Set/Get the status of the track (positive, negative, or none)
  vtkSetMacro(Status, int);
  vtkGetMacro(Status, int);

  // Description:
  // Set scalars to be used for visualization
  bool SetActiveScalars(const std::string& name);
  // Get scalars that are currently being used for the visualization
  vtkSmartPointer<vtkVgScalars> GetActiveScalars();
  const vtkSmartPointer<vtkVgScalars> GetActiveScalars() const;

  // Description:
  // Set scalars linked with the track
  void SetScalars(const std::string& name, vtkVgScalars* scalars);
  // Get scalars linked with the track
  vtkSmartPointer<vtkVgScalars> GetScalars(const std::string& name);
  const vtkSmartPointer<vtkVgScalars> GetScalars(const std::string& name) const;

  // Description:
  // Get scalars name
  std::vector<std::string> GetScalarsName() const;

private:
  vtkVgTrack(const vtkVgTrack&); // Not implemented.
  void operator=(const vtkVgTrack&);  // Not implemented.

  // Description:
  // Constructor / Destructor.
  vtkVgTrack();
  ~vtkVgTrack();

  void AddTrackSegment(vtkVgTimeStamp& startFrame, vtkVgTimeStamp& endFrame,
                       vtkVgTimeStamp& maximumDisplayDuration);

  // Shell points may be supplied either as a float* or vtkPoints* + start
  // index. Caller should pass 0 as fromShellPts if the shell data is given
  // in a float array. If both are given, only the vtkPoints data will be used.
  void AddInterpolationPoints(const vtkVgTimeStamp& previousTimeStamp,
                              const vtkVgTimeStamp& timeStamp,
                              double previousPoint[2], double point[2],
                              vtkIdType numShellPts,
                              vtkPoints* fromShellPts,
                              vtkIdType fromShellPtsStart,
                              float* shellPts = 0,
                              bool warnOnFailure = true);

  void BuildAllPointsIdMap(const vtkVgTimeStamp& timeStamp,
                           vtkIdType newTrackPtId, double point[2]);

  int Type;
  vtkIdType Id;
  unsigned char Flags;
  unsigned char DisplayFlags;

  bool InterpolateMissingPointsOnInsert;
  vtkVgTimeStamp InterpolationSpacing;

  vtkVgTimeStamp StartFrame;
  vtkVgTimeStamp EndFrame;

  vtkPoints* Points;
  vtkIdList* PointIds;

  char* Name;
  char* Note;

  double Normalcy;
  double PVO[3];

  double Color[3];

  bool UseCustomColor;
  double CustomColor[3];

  // only use 4 or 6, but done for convenience right now
  double FullBounds[6];

  int Status;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgTrack_h
