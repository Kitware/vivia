// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTrackFilter_h
#define __vtkVgTrackFilter_h

#include <vgExport.h>

#include <vtkObject.h>

#include <map>

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
  // Get the highest scoring classifier for a track given the current
  // filter settings. Returns -1 if no classifiers pass the filter.
  virtual int GetBestClassifier(vtkVgTrack* track);

protected:
  vtkVgTrackFilter();
  ~vtkVgTrackFilter();

  struct FilterSettings
  {
    bool Show;
    double MinProbability;
    double MaxProbability;
  };

  std::map<int, FilterSettings> Filters;

private:
  vtkVgTrackFilter(const vtkVgTrackFilter&) = delete;
  void operator=(const vtkVgTrackFilter&) = delete;
};

#endif // __vtkVgTrackFilter_h
