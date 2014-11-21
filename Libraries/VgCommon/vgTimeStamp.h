/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgTimeStamp_h
#define __vgTimeStamp_h

//-----------------------------------------------------------------------------
struct vgTimeStamp
{
  static double InvalidTime()
    { return -1e300; }
  static unsigned int InvalidFrameNumber()
    { return static_cast<unsigned int>(-1); }

  inline vgTimeStamp();
  inline explicit vgTimeStamp(double time);
  inline explicit vgTimeStamp(unsigned int frame_number);
  inline explicit vgTimeStamp(double time, unsigned int frame_number);

  inline bool IsValid() const;
  inline bool HasTime() const;
  inline bool HasFrameNumber() const;

  inline bool operator==(const vgTimeStamp&) const;
  inline bool operator!=(const vgTimeStamp&) const;

  inline bool operator<(const vgTimeStamp&) const;
  inline bool operator>(const vgTimeStamp&) const;
  inline bool operator<=(const vgTimeStamp&) const;
  inline bool operator>=(const vgTimeStamp&) const;

  inline vgTimeStamp operator-(const vgTimeStamp&) const;

  inline static vgTimeStamp fromTime(double time);
  inline static vgTimeStamp fromFrameNumber(unsigned int frame_number);

  double Time;
  unsigned int FrameNumber;
};

//-----------------------------------------------------------------------------
vgTimeStamp::vgTimeStamp()
  : Time(InvalidTime()), FrameNumber(InvalidFrameNumber())
{
}

//-----------------------------------------------------------------------------
vgTimeStamp::vgTimeStamp(double time)
  : Time(time), FrameNumber(InvalidFrameNumber())
{
}

//-----------------------------------------------------------------------------
vgTimeStamp::vgTimeStamp(unsigned int frame_number)
  : Time(InvalidTime()), FrameNumber(frame_number)
{
}

//-----------------------------------------------------------------------------
vgTimeStamp::vgTimeStamp(double time, unsigned int frame_number)
  : Time(time), FrameNumber(frame_number)
{
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::IsValid() const
{
  return this->HasTime() || this->HasFrameNumber();
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::HasTime() const
{
  return this->Time != InvalidTime();
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::HasFrameNumber() const
{
  return this->FrameNumber != InvalidFrameNumber();
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::operator==(const vgTimeStamp& other) const
{
  return (this->Time == other.Time) &&
         (this->FrameNumber == other.FrameNumber);
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::operator!=(const vgTimeStamp& other) const
{
  return !(*this == other);
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::operator<(const vgTimeStamp& other) const
{
  if (this->HasTime() && other.HasTime())
    return this->Time < other.Time;

  if (this->HasFrameNumber() && other.HasFrameNumber())
    return this->FrameNumber < other.FrameNumber;

  return false;
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::operator>(const vgTimeStamp& other) const
{
  return other < *this;
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::operator<=(const vgTimeStamp& other) const
{
  if (this->HasTime() && other.HasTime())
    return this->Time <= other.Time;

  if (this->HasFrameNumber() && other.HasFrameNumber())
    return this->FrameNumber <= other.FrameNumber;

  return false;
}

//-----------------------------------------------------------------------------
bool vgTimeStamp::operator>=(const vgTimeStamp& other) const
{
  return other <= *this;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgTimeStamp::operator-(const vgTimeStamp& other) const
{
  vgTimeStamp result;
  if (this->HasTime() && other.HasTime())
    result.Time = this->Time - other.Time;
  if (this->HasFrameNumber() && this->FrameNumber > other.FrameNumber)
    result.FrameNumber = this->FrameNumber - other.FrameNumber;
  return result;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgTimeStamp::fromTime(double time)
{
  return vgTimeStamp(time);
}

//-----------------------------------------------------------------------------
vgTimeStamp vgTimeStamp::fromFrameNumber(unsigned int frame_number)
{
  return vgTimeStamp(frame_number);
}

#endif
