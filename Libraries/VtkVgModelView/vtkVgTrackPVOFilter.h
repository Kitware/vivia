/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgTrackPVOFilter_h
#define __vtkVgTrackPVOFilter_h

#include "vtkVgTrackFilter.h"

class vtkVgTrack;

// DEPRECATED
class VTKVG_MODELVIEW_EXPORT vtkVgTrackPVOFilter : public vtkVgTrackFilter
{
public:
  vtkTypeMacro(vtkVgTrackPVOFilter, vtkVgTrackFilter);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgTrackPVOFilter* New();

  // Description:
  // Get the highest scoring PVO classifier for a track given the current
  // filter settings. Returns -1 if no classifiers pass the filter.
  virtual int GetBestClassifier(vtkVgTrack* track) override;

private:
  vtkVgTrackPVOFilter(const vtkVgTrackPVOFilter&) = delete;
  void operator=(const vtkVgTrackPVOFilter&) = delete;

  vtkVgTrackPVOFilter();
  ~vtkVgTrackPVOFilter();
};

#endif // __vtkVgTrackFilter_h
