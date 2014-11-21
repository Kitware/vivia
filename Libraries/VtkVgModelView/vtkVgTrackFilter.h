/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgTrackFilter_h
#define __vtkVgTrackFilter_h

#include "vtkObject.h"

#include <vgExport.h>

class vtkVgTrack;

class VTKVG_MODELVIEW_EXPORT vtkVgTrackFilter : public vtkObject
{
public:
  vtkTypeMacro(vtkVgTrackFilter, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgTrackFilter* New();

  // Description:
  // Enable or disable filtering of all tracks of a certain type.
  void SetShowType(int trackType, bool state);
  bool GetShowType(int trackType);

  // Description:
  // Don't display tracks of this type with a probability below the threshold value.
  void   SetMinProbability(int trackType, double threshold);
  double GetMinProbability(int trackType);

  // Description:
  // Don't display tracks of this type with a probability above the threshold
  // value. Set to -1.0 to disable the filter (default setting).
  void   SetMaxProbability(int trackType, double threshold);
  double GetMaxProbability(int trackType);

  // Description:
  // Get the highest scoring PVO classifier for a track given the current
  // filter settings. Returns -1 if no classifiers pass the filter.
  int GetBestClassifier(vtkVgTrack* track);

private:
  vtkVgTrackFilter(const vtkVgTrackFilter&); // Not implemented.
  void operator=(const vtkVgTrackFilter&);   // Not implemented.

  vtkVgTrackFilter();
  ~vtkVgTrackFilter();

  bool   FilterShow[3];
  double FilterMinProbability[3];
  double FilterMaxProbability[3];
};

#endif // __vtkVgTrackFilter_h
