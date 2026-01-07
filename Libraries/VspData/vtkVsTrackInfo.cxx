// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVsTrackInfo.h"

vtkImplementMetaObject(vtkVsTrackInfo, vtkVgEventTrackInfoBase);

//-----------------------------------------------------------------------------
vtkVsTrackInfo::vtkVsTrackInfo(
  vsTrackId tid, const vtkVgTimeStamp& start, const vtkVgTimeStamp& end)
  : vtkVgEventTrackInfoBase(-1, start, end), LogicalId(tid)
{
}

//-----------------------------------------------------------------------------
vtkVsTrackInfo::vtkVsTrackInfo(const vtkVsTrackInfo& other)
  : vtkVgEventTrackInfoBase(other), LogicalId(other.LogicalId)
{
}

//-----------------------------------------------------------------------------
vtkVsTrackInfo::~vtkVsTrackInfo()
{
}

//-----------------------------------------------------------------------------
vtkVgEventTrackInfoBase* vtkVsTrackInfo::Clone() const
{
  return new vtkVsTrackInfo(*this);
}

//-----------------------------------------------------------------------------
const char* vtkVsTrackInfo::CheckValid() const
{
  if (this->LogicalId.Source < 0)
    {
    return "Only non-negative track sources are allowed.\n";
    }

  if (this->LogicalId.SerialNumber < 0)
    {
    return "Only non-negative track serial numbers are allowed.\n";
    }

  return vtkVgEventTrackInfoBase::CheckBaseValid();
}
