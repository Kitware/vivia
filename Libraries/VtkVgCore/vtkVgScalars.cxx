/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgScalars.h"

#include "vtkVgTimeStamp.h"

#include <vtkObjectFactory.h>

#include <map>
#include <limits>

vtkStandardNewMacro(vtkVgScalars);

double vtkVgScalars::NotFoundValue = -std::numeric_limits<double>::max();

class vtkVgScalars::vtkVgScalarsInternal
{
public:
  double Range[2];
  vtkTimeStamp ComputeRangeTime;
  std::map<vtkVgTimeStamp, double> Scalars;
};

//-----------------------------------------------------------------------------
vtkVgScalars::vtkVgScalars() : vtkObject()
{
  this->Implementation = new vtkVgScalarsInternal();
  this->Implementation->Range[0] =  std::numeric_limits<double>::max();
  this->Implementation->Range[1] = -std::numeric_limits<double>::max();
  this->Implementation->ComputeRangeTime.Modified();
}

//-----------------------------------------------------------------------------
vtkVgScalars::~vtkVgScalars()
{
  delete this->Implementation;
}

//-----------------------------------------------------------------------------
void vtkVgScalars::SetNotFoundValue(double value)
{
  NotFoundValue = value;
}

//-----------------------------------------------------------------------------
double vtkVgScalars::GetNotFoundValue()
{
  return NotFoundValue;
}

//-----------------------------------------------------------------------------
void vtkVgScalars::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
bool vtkVgScalars::HasValue(const vtkVgTimeStamp &timestamp) const
{
  return this->Implementation->Scalars.count(timestamp) != 0;
}

//-----------------------------------------------------------------------------
void vtkVgScalars::InsertValue(const vtkVgTimeStamp& timestamp,
                               const double& value)
{
  this->Implementation->Scalars[timestamp] = value;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgScalars::InsertValues(const std::map<vtkVgTimeStamp, double>& values)
{
  std::map<vtkVgTimeStamp, double>::const_iterator constItr =
    values.begin();

  for (; constItr != values.end(); ++constItr)
    {
    this->InsertValue(constItr->first, constItr->second);
    }
}

//-----------------------------------------------------------------------------
double vtkVgScalars::GetValue(const vtkVgTimeStamp& timestamp,
                              bool interpolate) const
{
  if (this->Implementation->Scalars.empty())
    {
    return vtkVgScalars::NotFoundValue;
    }

  std::map<vtkVgTimeStamp, double>::const_iterator itr;
  itr = this->Implementation->Scalars.lower_bound(timestamp);

  if (itr == this->Implementation->Scalars.end())
    {
    // There is no scalar in the map defined at this time or later
    if (interpolate)
      {
      // Use the previously defined value
      return (--itr)->second;
      }
    else
      {
      return vtkVgScalars::NotFoundValue;
      }
    }
  if (itr->first == timestamp)
    {
    // Found an exact match
    return itr->second;
    }
  if (!interpolate)
    {
    // Needed an exact match
    return vtkVgScalars::NotFoundValue;
    }

  // Return the scalar defined at the next later timestamp
  // TODO: really interpolate
  return itr->second;
}

//-----------------------------------------------------------------------------
const double* vtkVgScalars::GetRange()
{
  if (this->Implementation->ComputeRangeTime > this->GetMTime())
    {
    return this->Implementation->Range;
    }

  std::map<vtkVgTimeStamp, double>::const_iterator constItr =
    this->Implementation->Scalars.begin();
  for (; constItr != this->Implementation->Scalars.end(); ++constItr)
    {
    if (constItr->second < this->Implementation->Range[0])
      {
      this->Implementation->Range[0] = constItr->second;
      }
    if (constItr->second > this->Implementation->Range[1])
      {
      this->Implementation->Range[1] = constItr->second;
      }
    }

  this->Implementation->ComputeRangeTime.Modified();

  return this->Implementation->Range;
}
