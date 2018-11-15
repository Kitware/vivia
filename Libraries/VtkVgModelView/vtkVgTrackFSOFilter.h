/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgTrackFSOFilter_h
#define __vtkVgTrackFSOFilter_h

#include "vtkVgTrackFilter.h"

class vtkVgTrack;

// DEPRECATED
class VTKVG_MODELVIEW_EXPORT vtkVgTrackFSOFilter : public vtkVgTrackFilter
{
public:
  vtkTypeMacro(vtkVgTrackFSOFilter, vtkVgTrackFilter);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgTrackFSOFilter* New();

  // Description:
  // Get the highest scoring FSO classifier for a track given the current
  // filter settings. Returns -1 if no classifiers pass the filter.
  virtual int GetBestClassifier(vtkVgTrack* track) override;

private:
  vtkVgTrackFSOFilter(const vtkVgTrackFSOFilter&) = delete;
  void operator=(const vtkVgTrackFSOFilter&) = delete;

  vtkVgTrackFSOFilter();
  ~vtkVgTrackFSOFilter();
};

#endif // __vtkVgTrackFilter_h
