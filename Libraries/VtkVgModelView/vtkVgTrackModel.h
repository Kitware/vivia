/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgTrackModel_h
#define __vtkVgTrackModel_h

#include "vtkVgModelBase.h"

#include <vtkVgTrackInfo.h>
#include <vtkVgTrack.h>

#include <vgRange.h>

#include <vtkVgMacros.h>

#include <string>
#include <vector>

#include <vgExport.h>

class vtkPoints;

class VTKVG_MODELVIEW_EXPORT vtkVgTrackModel : public vtkVgModelBase
{
public:
  typedef vgRange<double> ScalarsRange;

  vtkVgClassMacro(vtkVgTrackModel);

  vtkTypeMacro(vtkVgTrackModel, vtkVgModelBase);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgTrackModel* New();

  void Initialize();

  // Description:
  // Set/Get the points defining the tracks. All the points for all the tracks
  // are included in this list.
  void SetPoints(vtkPoints* points);
  vtkGetObjectMacro(Points, vtkPoints);

  void AddTrack(vtkVgTrack* track);
  void RemoveTrack(vtkIdType trackId);

  vtkVgTrack*    GetTrack(vtkIdType trackId);
  vtkVgTrackInfo GetTrackInfo(vtkIdType trackId);

  vtkVgTrackDisplayData GetTrackDisplayData(vtkVgTrack* track,
                                            bool forceDisplay = false);

  vtkVgTimeStamp GetTrackStartDisplayFrame(vtkVgTrack* track);
  vtkVgTimeStamp GetTrackEndDisplayFrame(vtkVgTrack* track);

  void InitTrackTraversal();
  vtkVgTrackInfo GetNextTrack();
  vtkVgTrackInfo GetNextDisplayedTrack();

  int GetNumberOfTracks();

  // Description:
  // Updates the model to the given timestamp. This should become a true
  // timestamp once this class derives from vtkVgModelBase
  virtual int Update(const vtkVgTimeStamp& timestamp,
                     const vtkVgTimeStamp* referenceFrameTimeStamp);
  using Superclass::Update;

  // Description:
  // Set/Get whether the track should be displayed.  Functionality
  // could go in the track itself, but didn't seem like something the
  // track should do.
  void SetTrackDisplayState(vtkIdType trackId, bool displayTrack);

  // Description:
  // Set the TrackDisplay state for all tracks to "true" (ON).  Note, this does
  // NOT change the state of the DisplayAllTracks flag.
  void TurnOnAllTracks()
    {
    this->SetAllTracksDisplayState(true);
    }

  void SetTrackExpirationOffset(const vtkVgTimeStamp& offset);

  vtkSetMacro(ShowTracksBeforeStart, bool);
  vtkGetMacro(ShowTracksBeforeStart, bool);
  vtkBooleanMacro(ShowTracksBeforeStart, bool);

  vtkSetMacro(ShowTracksAfterExpiration, bool);
  vtkGetMacro(ShowTracksAfterExpiration, bool);
  vtkBooleanMacro(ShowTracksAfterExpiration, bool);

  // Description:
  // Convenience method to set the MaximumDisplayDuration for all tracks.
  void SetMaximumDisplayDuration(const vtkVgTimeStamp& maxDisplayDuration);

  // Description:
  // Set the TrackDisplay state for all tracks to "true" (ON).  Note, this does
  // NOT change the state of the DisplayAllTracks flag.
  void TurnOffAllTracks()
    {
    this->SetAllTracksDisplayState(false);
    }

  // Description:
  // Set/Get whether all tracks are to be displayed.  If true, then the EventManager
  // won't turn off tracks not used by events.  If false, only those tracks used
  // by events will be turned on (the act of setting this flag will turn on all
  // tracks if set to true, but do nothign to track state if false
  void SetDisplayAllTracks(bool state);
  vtkGetMacro(DisplayAllTracks, bool);

  void    SetTrackColor(vtkIdType trackId, double color[3]);
  double* GetTrackColor(vtkIdType trackId);

  // Description:
  // Return the MTime for the last Update (that did something).
  virtual unsigned long GetUpdateTime();

  // Description:
  // Set scalars identified by \c name as active. Active
  // scalars could be used for the display purposes.
  void SetActiveScalars(const std::string& name);
  // Get active scalars
  const std::string& GetActiveScalars();
  const std::string& GetActiveScalars() const;

  // Description:
  // Return name of the scalars for all the existing tracks
  std::vector<std::string> GetAllScalarsName();

  // Description:
  // Return range of the scalars
  vgRange<double> GetScalarsRange(const std::string& name);

  // Description:
  // Get the smallest id which is greater than the id of any track in the model
  vtkIdType GetNextAvailableId();

  // Description:
  // Set the id offset to be applied to scene element tracks to avoid collisions
  // with normal tracks. This is for bookkeeping only, it does not affect the
  // behavior of the model.
  vtkSetMacro(SceneElementIdsOffset, vtkIdType);
  vtkGetMacro(SceneElementIdsOffset, vtkIdType);

  vtkIdType GetSceneElementIdForTrack(vtkIdType trackId)
    {
    return trackId - this->SceneElementIdsOffset;
    }

  vtkIdType GetTrackIdForSceneElement(vtkIdType sceneElementId)
    {
    return sceneElementId + this->SceneElementIdsOffset;
    }

  void UpdateColorOfTracksOfType(int typeIndex, double *rgb);

protected:
  vtkVgTrackModel();
  ~vtkVgTrackModel();

private:
  vtkVgTrackModel(const vtkVgTrackModel&); // Not implemented.
  void operator=(const vtkVgTrackModel&);  // Not implemented.

  void SetAllTracksDisplayState(bool state);

  void UpdateTemporalFiltering(vtkVgTrackInfo& info);
  void UpdateSpatialFiltering(vtkVgTrackInfo& info);

  void ClearTracks();

  bool DisplayAllTracks;

  bool ShowTracksBeforeStart;
  bool ShowTracksAfterExpiration;

  vtkVgTimeStamp ReferenceTimeStamp;
  vtkVgTimeStamp MaxDisplayDuration;
  vtkVgTimeStamp TrackExpirationOffset;

  vtkPoints* Points;

  std::string ActiveScalarsName;

  vtkIdType SceneElementIdsOffset;

  struct vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgTrackModel_h
