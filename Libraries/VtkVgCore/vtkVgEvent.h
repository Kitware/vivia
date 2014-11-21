/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgEvent_h
#define __vtkVgEvent_h

#include "vtkVgEventBase.h"
#include "vtkVgSetGet.h"

#include <vgExport.h>

class vtkIdList;
class vtkIdListCollection;
class vtkPoints;
class vtkVgTrack;

#include "vtkVgTrack.h"

class VTKVG_CORE_EXPORT vtkVgEventTrackInfo : public vtkVgEventTrackInfoBase
{
  vtkDeclareMetaObject(vtkVgEventTrackInfo);

public:
  vtkVgEventTrackInfo(vtkVgTrack* track,
                      const vtkVgTimeStamp& startFrameIndex,
                      const vtkVgTimeStamp& endFrameIndex);
  vtkVgEventTrackInfo(const vtkVgEventTrackInfoBase* fromTrackInfo);
  vtkVgEventTrackInfo(const vtkVgEventTrackInfo* fromTrackInfo);
  virtual ~vtkVgEventTrackInfo();

  virtual vtkVgEventTrackInfoBase* Clone() const;

  virtual const char* CheckValid() const;

  void SetTrack(vtkVgTrack* track);
  vtkVgTrack* GetTrack()
    {
    return this->Track;
    }

protected:

  friend class vtkVgEvent;

  vtkVgTrack* Track;
};


class VTKVG_CORE_EXPORT vtkVgEvent : public vtkVgEventBase
{
public:

  // Description:
  // Standard VTK functions.
  static vtkVgEvent* New();
  vtkTypeMacro(vtkVgEvent, vtkVgEventBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return collection of vtkIdLists containing the complete event
  vtkIdListCollection* GetFullEventIdCollection();

  vtkVgTrackDisplayData GetTrackDisplayData(unsigned int trackIndex,
                                            bool useTrackGroups,
                                            vtkVgTimeStamp start,
                                            vtkVgTimeStamp end);

  vtkGetObjectMacro(IconIds, vtkIdList);

  // Description:
  // Get the underlying vtkPoints
  vtkPoints* GetPoints();

  // Description:
  // Return the bounds of the entire event (xmin, xmax, ymin, ymax).
  double* GetFullBounds();

  // Description:
  // Get the bounds for this entire event as (xmin, xmax, ymin, ymax).
  void GetFullBounds(double bounds[4]);

  // Description:
  // The display position is the average of the current location of each track
  // making up the event, at the specified timeStamp.  Returns false if no
  // tracks (or if one of the tracks isn't valid).
  bool GetDisplayPosition(const vtkVgTimeStamp& timeStamp, double position[2]);

  // Description:
  // Add a new track into the list of tracks related with the event.
  void AddTrack(vtkVgTrack* track, const vtkVgTimeStamp& startFrame,
                const vtkVgTimeStamp& endFrame);

  // Description:
  // Overload same signature from vtkVgEventBase, since we require a pointer to
  // a vtkVgTrack when we add a track to a full blown vtkVgEvent.
  virtual void AddTrack(vtkIdType vtkNotUsed(trackId),
                        vtkVgTimeStamp& vtkNotUsed(startFrame),
                        vtkVgTimeStamp& vtkNotUsed(endFrame))
    {
    vtkErrorMacro("Must specify vtkVgTrack*, not track id,"
                  " when adding a track to vtkVgEvent!");
    }

  // Description:
  // Overload same signature from vtkVgEventBase, since we require a pointer to
  // a vtkVgTrack when we add a track to a full blown vtkVgEvent. Note that we
  // will accept a vtkVgEventTrackInfo.
  virtual void AddTrack(vtkVgEventTrackInfoBase* trackInfo)
    {
    vtkVgEventTrackInfo* ti = vtkVgEventTrackInfo::SafeDownCast(trackInfo);
    if (ti)
      {
      this->Superclass::AddTrack(trackInfo);
      }
    else
      {
      vtkErrorMacro("Must specify vtkVgTrack* or vtkVgEventTrackInfo*"
                    " when adding a track to vtkVgEvent!");
      }
    }

  // Description:
  // Given a valid index, fill in track and its related parameters.
  // \TODO: This interface might change in the future.
  void GetTrack(unsigned int index, vtkVgTrack*& track,
                vtkVgTimeStamp& startFrame,
                vtkVgTimeStamp& endFrame);

  // Description:
  // Return track at the specified index
  vtkVgTrack* GetTrack(unsigned int index);

  // Description:
  // Return track at the specified index
  vtkVgTrack* GetTrackGroupTrack(unsigned int trackGroupIndex);

  // Description:
  // Return if the event contains track with given id.
  bool HasTrack(int trackId);

  // Description:
  // Specify a track
  bool SetTrackPtr(unsigned int index, vtkVgTrack* track);

  // Description:
  // Support for per-event colors.
  vtkVgSetVector3Macro(CustomColor, double);
  vtkGetVector3Macro(CustomColor, double);

  vtkSetMacro(UseCustomColor, bool);
  vtkGetMacro(UseCustomColor, bool);
  vtkBooleanMacro(UseCustomColor, bool);

protected:
  vtkVgEvent();
  ~vtkVgEvent();

  bool GetTrackGroupInfo(unsigned int trackGroupIndex, vtkVgTrack*& theTrack,
                         vtkVgTimeStamp& startTime, vtkVgTimeStamp& endTime);

  virtual void CopyTracks(std::vector<vtkVgEventTrackInfoBase*>& tracks);

  vtkIdListCollection* FullEventIdCollection;

  vtkIdList* IconIds;

  // only use 4 or 6, but done for convenience right now
  double FullBounds[6];

  bool UseCustomColor;
  double CustomColor[3];

private:
  vtkVgEvent(const vtkVgEvent&); // Not implemented.
  void operator=(const vtkVgEvent&);  // Not implemented.

};

#endif // __vtkVgEvent_h
