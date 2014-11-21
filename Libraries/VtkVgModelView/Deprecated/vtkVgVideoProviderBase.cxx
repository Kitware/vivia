/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgVideoProviderBase.h"

//-----------------------------------------------------------------------------
vtkVgVideoProviderBase::vtkVgVideoProviderBase() : vtkVgDataSourceBase()
{
  this->TimeRange[1] = this->TimeRange[0] = -1.0;
  this->TimeInterval = -1.0;
  this->RequestedPadding = 0;
  this->Looping = 0;
  this->StreamId = 0;
  this->MissionId = 0;
}

//-----------------------------------------------------------------------------
vtkVgVideoProviderBase::~vtkVgVideoProviderBase()
{
  this->SetStreamId(0);
  this->SetMissionId(0);
}

//-----------------------------------------------------------------------------
void vtkVgVideoProviderBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgVideoProviderBase::ShallowCopy(vtkVgDataSourceBase& other)
{
  this->Superclass::ShallowCopy(other);

  vtkVgVideoProviderBase* otherDerived =
    vtkVgVideoProviderBase::SafeDownCast(&other);
  if (otherDerived)
    {
    this->TimeRange[0] = otherDerived->TimeRange[0];
    this->TimeRange[1] = otherDerived->TimeRange[1];

    this->TimeInterval = otherDerived->TimeInterval;

    this->FrameRange[0] = otherDerived->FrameRange[0];
    this->FrameRange[1] = otherDerived->FrameRange[1];

    this->FrameInterval = otherDerived->FrameInterval;

    this->RequestedPadding = otherDerived->RequestedPadding;
    this->Looping = otherDerived->Looping;

    this->SetStreamId(otherDerived->StreamId);
    this->SetMissionId(otherDerived->MissionId);
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoProviderBase::DeepCopy(vtkVgDataSourceBase& other)
{
  this->Superclass::DeepCopy(other);

  vtkVgVideoProviderBase* otherDerived =
    vtkVgVideoProviderBase::SafeDownCast(&other);
  if (otherDerived)
    {
    this->TimeRange[0] = otherDerived->TimeRange[0];
    this->TimeRange[1] = otherDerived->TimeRange[1];

    this->TimeInterval = otherDerived->TimeInterval;

    this->FrameRange[0] = otherDerived->FrameRange[0];
    this->FrameRange[1] = otherDerived->FrameRange[1];

    this->FrameInterval = otherDerived->FrameInterval;

    this->RequestedPadding = otherDerived->RequestedPadding;
    this->Looping = otherDerived->Looping;

    this->SetStreamId(otherDerived->StreamId);
    this->SetMissionId(otherDerived->MissionId);
    }
}

//-----------------------------------------------------------------------------
void vtkVgVideoProviderBase::AddTimeMark(double startTime, double endTime)
{
  this->TimeMarks.push_back(TimeMark(startTime, endTime));
}

//-----------------------------------------------------------------------------
std::vector<std::pair<double, double> > vtkVgVideoProviderBase::GetTimeMarks()
{
  return this->TimeMarks;
}
