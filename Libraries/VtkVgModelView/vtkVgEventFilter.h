// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgEventFilter_h
#define __vtkVgEventFilter_h

#include "vtkObject.h"

#include <vector>
#include <utility>

#include <vgExport.h>

class vtkVgEvent;

class VTKVG_MODELVIEW_EXPORT vtkVgEventFilter : public vtkObject
{
public:
  vtkTypeMacro(vtkVgEventFilter, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgEventFilter* New();

  // Description:
  // Enable or disable filtering of all events of a certain type.
  void SetShowType(int eventType, bool state);
  bool GetShowType(int eventType);

  // Description:
  // Enable or disable inverse state of filtering.
  void SetInverse(int eventType, bool state);
  bool GetInverse(int eventType);

  // Description:
  // Don't display events of this type with a normalcy above the threshold value.
  void   SetMaxNormalcy(int eventType, double threshold);
  double GetMaxNormalcy(int eventType);

  // Description:
  // Don't display events of this type with a probability below the threshold value.
  void   SetMinProbability(int eventType, double threshold);
  double GetMinProbability(int eventType);

  // Description:
  // Get the list of all classifiers that pass the filter, along with their
  // associated scores. Returns an empty list if no classifiers pass the filter.
  typedef std::pair<int, double> ScoredClassifier;
  std::vector<ScoredClassifier> GetActiveClassifiers(vtkVgEvent* event);

  // Description:
  // Get the highest scoring classifier for an event given the current filter settings.
  // Returns -1 if no classifiers pass the filter.
  int GetBestClassifier(vtkVgEvent* event);

private:
  vtkVgEventFilter(const vtkVgEventFilter&); // Not implemented.
  void operator=(const vtkVgEventFilter&);   // Not implemented.

  vtkVgEventFilter();
  ~vtkVgEventFilter();

  struct vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgEventFilter_h
