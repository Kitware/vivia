// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVpTrackModel_h
#define __vtkVpTrackModel_h

#include "vtkVgTrackModel.h"

#include <set>
#include <unordered_map>

class vtkVpTrackModel : public vtkVgTrackModel
{
public:
  static vtkVpTrackModel* New();
  vtkTypeMacro(vtkVpTrackModel, vtkVgTrackModel);

  void SetTrackId(vtkVgTrack* track, vtkIdType newId);

  bool GetIsKeyframe(vtkIdType trackId, const vtkVgTimeStamp& timeStamp) const;

  void AddKeyframe(vtkIdType trackId, const vtkVgTimeStamp& timeStamp);
  void RemoveKeyframe(vtkIdType trackId, const vtkVgTimeStamp& timeStamp);

  void RemoveTrack(vtkIdType trackId);

protected:
  // Description:
  // Constructor / Destructor.
  vtkVpTrackModel();
  virtual ~vtkVpTrackModel();

private:
  vtkVpTrackModel(const vtkVpTrackModel&);  // Not implemented.
  void operator=(const vtkVpTrackModel&);  // Not implemented.

  using frame_set = std::set<vtkVgTimeStamp>;
  std::unordered_map<vtkIdType, frame_set> TrackKeyframes;
};

#endif // __vpFseTrackIO_h
