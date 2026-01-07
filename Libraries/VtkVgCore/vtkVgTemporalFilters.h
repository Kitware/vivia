// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTemporalFilters_h
#define __vtkVgTemporalFilters_h

#include "vtkObject.h"

#include <vgExport.h>

class vtkVgTimeStamp;

class VTKVG_CORE_EXPORT vtkVgTemporalFilters : public vtkObject
{
public:
  enum FilterType
    {
    FT_Select,
    FT_Exclude
    };

public:
  static vtkVgTemporalFilters* New();

  vtkTypeMacro(vtkVgTemporalFilters, vtkObject);

  // Description:
  // Add a new filter that is enabled by default. Returns the added filter id.
  int  AddFilter(FilterType type,
                 const vtkVgTimeStamp& start, const vtkVgTimeStamp& end);

  // Description:
  // Assign properties to an existing filter.
  void SetFilter(int id, FilterType type,
                 const vtkVgTimeStamp& start, const vtkVgTimeStamp& end,
                 bool enabled);

  // Description:
  // Return ture if filter with id exists; if so, return properties of the
  // filter via the other fn parameters.
  bool GetFilterInfo(int id, FilterType& type,
                     vtkVgTimeStamp& start, vtkVgTimeStamp& end,
                     bool& enabled);

  void RemoveFilter(int id);
  void RemoveAllFilters();

  // Description:
  // Return true if any part of the interval is not excluded by the current
  // set of filters.
  bool EvaluateInterval(const vtkVgTimeStamp& start, const vtkVgTimeStamp& end);

  static const char* StringForType(FilterType type);

private:
  vtkVgTemporalFilters();
  ~vtkVgTemporalFilters();

  size_t GetFilterIndex(int id);

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
