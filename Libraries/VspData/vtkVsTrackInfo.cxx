/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVsTrackInfo.h"

vtkImplementMetaObject(vtkVsTrackInfo, vtkVgEventTrackInfoBase);

//-----------------------------------------------------------------------------
vtkVsTrackInfo::vtkVsTrackInfo(
  vvTrackId tid, const vtkVgTimeStamp& start, const vtkVgTimeStamp& end)
  : vtkVgEventTrackInfoBase(-1, start, end), VvTrackId(tid)
{
}

//-----------------------------------------------------------------------------
vtkVsTrackInfo::vtkVsTrackInfo(const vtkVsTrackInfo& other)
  : vtkVgEventTrackInfoBase(other), VvTrackId(other.VvTrackId)
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
  if (this->VvTrackId.Source < 0)
    {
    return "Only non-negative track sources are allowed.\n";
    }

  if (this->VvTrackId.SerialNumber < 0)
    {
    return "Only non-negative track serial numbers are allowed.\n";
    }

  return vtkVgEventTrackInfoBase::CheckBaseValid();
}
