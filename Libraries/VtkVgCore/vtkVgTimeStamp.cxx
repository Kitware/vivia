// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgTimeStamp.h"

#include <iostream>

//-----------------------------------------------------------------------------
double vtkVgTimeStamp::GetTimeDifferenceInSecs(const vtkVgTimeStamp& other) const
{
  if (this->HasTime() && other.HasTime())
    {
    return (this->Time - other.Time) / 1e6;
    }
  else if (this->IsMaxTime() || other.IsMaxTime() ||
           this->IsMinTime() || other.IsMinTime())
    {
    return VTK_DOUBLE_MAX;
    }
  else
    {
    return double(this->FrameNumber) - double(other.FrameNumber);
    }
}

//-----------------------------------------------------------------------------
bool vtkVgTimeStamp::operator<(const vtkVgTimeStamp& other) const
{
  if (this->HasTime() && other.HasTime())
    {
    return this->GetTime() < other.GetTime();
    }
  else if (this->IsMaxTime() || other.IsMinTime())
    {
    return false;
    }
  else if (other.IsMaxTime() || this->IsMinTime())
    {
    return true;
    }
  //else if (this->HasTime() || other.HasTime())
  //  {
  //  std::cerr << "comparing timestamps that don't both have time values\n";
  //  return false;
  //  }
  else if (this->HasFrameNumber() && other.HasFrameNumber())
    {
    return this->GetFrameNumber() < other.GetFrameNumber();
    }
  else
    {
    std::cerr << "comparing timestamps that don't both have time values or frame number values, or are empty\n";
    return false;
    }
}

//-----------------------------------------------------------------------------
bool vtkVgTimeStamp::operator==(const vtkVgTimeStamp& other) const
{
  if (this->HasTime() && other.HasTime())
    {
    return this->GetTime() == other.GetTime();
    }
  else if (this->IsMaxTime() || other.IsMaxTime() ||
           this->IsMinTime() || other.IsMinTime())
    {
    return false;
    }
  //else if (this->HasTime() || other.HasTime())
  //  {
  //  std::cerr << "comparing timestamps that don't both have time values\n";
  //  return false;
  //  }
  else if (this->HasFrameNumber() && other.HasFrameNumber())
    {
    return this->GetFrameNumber() == other.GetFrameNumber();
    }
  else
    {
    std::cerr << "comparing timestamps that don't both have time values or frame number values, or are empty\n";
    return false;
    }
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp& vtkVgTimeStamp::operator-(const double& timeInterval)
{
  if(this->HasTime())
    {
    this->SetTime(this->GetTime() - timeInterval);
    }

  return *this;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp& vtkVgTimeStamp::operator-(const unsigned int& frameInterval)
{
  if(this->HasFrameNumber())
    {
    this->SetFrameNumber(this->GetFrameNumber() - frameInterval);
    }

  return *this;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp& vtkVgTimeStamp::operator+(const double& timeInterval)
{
  if(this->HasTime())
    {
    this->SetTime(this->GetTime() + timeInterval);
    }

  return *this;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp& vtkVgTimeStamp::operator+(const unsigned int& frameInterval)
{
  if(this->HasFrameNumber())
    {
    this->SetFrameNumber(this->GetFrameNumber() + frameInterval);
    }

  return *this;
}

//-----------------------------------------------------------------------------
void vtkVgTimeStamp::ShiftForward(const vtkVgTimeStamp& other)
{
  if (this->HasTime() && other.HasTime())
    {
    this->Time += other.GetTime();
    }
  if (this->HasFrameNumber() && other.HasFrameNumber())
    {
    this->FrameNumber += other.GetFrameNumber();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTimeStamp::ShiftBackward(const vtkVgTimeStamp& other)
{
  if (this->HasTime() && other.HasTime())
    {
    this->Time -= other.GetTime();
    }
  if (this->HasFrameNumber() && other.HasFrameNumber())
    {
    this->FrameNumber -= other.GetFrameNumber();
    }
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp& operator+(const double& time, vtkVgTimeStamp& timestamp)
{
  if(timestamp.HasTime())
    {
    timestamp.SetTime(timestamp.GetTime() + time);
    }

  return timestamp;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp& operator+(const unsigned int& interval,
                           vtkVgTimeStamp& timestamp)
{
  if(timestamp.HasFrameNumber())
    {
    timestamp.SetFrameNumber(timestamp.GetFrameNumber() + interval);
    }

  return timestamp;
}
