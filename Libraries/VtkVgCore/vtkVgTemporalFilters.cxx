/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTemporalFilters.h"

#include "vtkVgTimeStamp.h"

#include <vtkObjectFactory.h>

#include <vector>

vtkStandardNewMacro(vtkVgTemporalFilters);

namespace
{

//----------------------------------------------------------------------------
struct vtkVgInterval
{
  vtkVgTimeStamp Start, End;

  vtkVgInterval(const vtkVgTimeStamp& start, const vtkVgTimeStamp& end)
    : Start(start), End(end)
    {}

  bool Intersects(const vtkVgInterval& other) const
    {
    if (this->Start > other.End)
      {
      return false;
      }

    if (this->End < other.Start)
      {
      return false;
      }

    return true;
    }

  bool Contains(const vtkVgInterval& other) const
    {
    if (other.End > this->End)
      {
      return false;
      }

    if (other.Start < this->Start)
      {
      return false;
      }

    return true;
    }
};

//----------------------------------------------------------------------------
struct vtkVgTemporalFilter
{
  int Id;
  vtkVgTemporalFilters::FilterType Type;
  vtkVgInterval Interval;
  bool Enabled;

  vtkVgTemporalFilter(int id, vtkVgTemporalFilters::FilterType type,
                      const vtkVgTimeStamp& start,
                      const vtkVgTimeStamp& end)
    : Id(id), Type(type), Interval(start, end), Enabled(true)
    {}
};

} // end anonymous namespace

//----------------------------------------------------------------------------
class vtkVgTemporalFilters::vtkInternal
{
public:
  int CurrId;
  std::vector<vtkVgTemporalFilter> Filters;

public:
  vtkInternal()
    : CurrId(0)
    {}

  int AddFilter(FilterType type,
                const vtkVgTimeStamp& start,
                const vtkVgTimeStamp& end)
    {
    this->Filters.push_back(
      vtkVgTemporalFilter(this->CurrId, type, start, end));

    return this->CurrId++;
    }
};

//----------------------------------------------------------------------------
vtkVgTemporalFilters::vtkVgTemporalFilters()
  : Internal(new vtkInternal)
{
}

//----------------------------------------------------------------------------
vtkVgTemporalFilters::~vtkVgTemporalFilters()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
int vtkVgTemporalFilters::AddFilter(FilterType type,
                                    const vtkVgTimeStamp& start,
                                    const vtkVgTimeStamp& end)
{
  int id = this->Internal->AddFilter(type, start, end);
  this->Modified();
  return id;
}

//----------------------------------------------------------------------------
void vtkVgTemporalFilters::SetFilter(int id, FilterType type,
                                     const vtkVgTimeStamp& start,
                                     const vtkVgTimeStamp& end,
                                     bool enabled)
{
  size_t filterIndex = this->GetFilterIndex(id);
  if (filterIndex == -1)
    {
    vtkWarningMacro("Temporal filter not found!");
    return;
    }
  vtkVgTemporalFilter& filter = this->Internal->Filters[filterIndex];
  filter.Type = type;
  filter.Enabled = enabled;
  filter.Interval.Start = start;
  filter.Interval.End = end;
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVgTemporalFilters::GetFilterInfo(int id, FilterType& type,
                                         vtkVgTimeStamp& start,
                                         vtkVgTimeStamp& end,
                                         bool& enabled)
{
  size_t filterIndex = this->GetFilterIndex(id);
  if (filterIndex == -1)
    {
    vtkWarningMacro("Temporal filter not found!");
    return false;
    }
  vtkVgTemporalFilter& filter = this->Internal->Filters[filterIndex];
  type = filter.Type;
  enabled = filter.Enabled;
  start = filter.Interval.Start;
  end = filter.Interval.End;
  return true;
}

//----------------------------------------------------------------------------
void vtkVgTemporalFilters::RemoveFilter(int id)
{
  this->Internal->Filters.erase(this->Internal->Filters.begin() +
                                this->GetFilterIndex(id));
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkVgTemporalFilters::RemoveAllFilters()
{
  this->Internal->Filters.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVgTemporalFilters::EvaluateInterval(const vtkVgTimeStamp& start,
                                            const vtkVgTimeStamp& end)
{
  for (size_t i = 0, e = this->Internal->Filters.size(); i < e; ++i)
    {
    const vtkVgTemporalFilter& filter = this->Internal->Filters[i];
    if (!filter.Enabled)
      {
      continue;
      }
    if (filter.Type == FT_Select)
      {
      if (!filter.Interval.Intersects(vtkVgInterval(start, end)))
        {
        return false;
        }
      }
    else // FT_Exclude
      {
      if (filter.Interval.Contains(vtkVgInterval(start, end)))
        {
        return false;
        }
      }
    }

  return true;
}

//----------------------------------------------------------------------------
const char* vtkVgTemporalFilters::StringForType(
  vtkVgTemporalFilters::FilterType type)
{
  switch (type)
    {
    case vtkVgTemporalFilters::FT_Select:   return "Select";
    case vtkVgTemporalFilters::FT_Exclude:  return "Exclude";
    }
  return "";
}

//----------------------------------------------------------------------------
size_t vtkVgTemporalFilters::GetFilterIndex(int id)
{
  for (size_t i = 0, end = this->Internal->Filters.size(); i < end; ++i)
    {
    if (this->Internal->Filters[i].Id == id)
      {
      return i;
      }
    }
  return -1;
}
