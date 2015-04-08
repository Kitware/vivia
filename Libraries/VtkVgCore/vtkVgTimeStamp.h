/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgTimeStamp_h
#define __vtkVgTimeStamp_h

// \brief Represents a world time in microseconds (10^-6 seconds).
//
// A timestamp contains two pieces of information: a time and a frame
// number.  Generally, one should use the time value over the frame
// number if both are available.
//
// NOTE: This is a copy (functionality) of timestamp in vidtk
#ifdef VTKEXTENSIONS_USE_VIDTK
#include <utilities/timestamp.h>
#endif
#include <vtkType.h>

#include <vgExport.h>
#include <vgTimeStamp.h>

class VTKVG_CORE_EXPORT vtkVgTimeStamp : private vgTimeStamp
{
public:
  // Description:
  // Default constructor
  vtkVgTimeStamp() {}

  vtkVgTimeStamp(const vtkVgTimeStamp& other)
    : vgTimeStamp(other) {}

  // Description:
  // Implicit constructor from vgTimeStamp
  vtkVgTimeStamp(const vgTimeStamp& vg_timestamp)
    : vgTimeStamp(vg_timestamp) {}

  // Description:
  // Assign from a vgTimeStamp.
  vtkVgTimeStamp& operator=(const vgTimeStamp& vg_timestamp)
    {
    this->Time = vg_timestamp.Time;
    this->FrameNumber = vg_timestamp.FrameNumber;
    return *this;
    }

  // Description:
  // Cast operator to vgTimeStamp.
  vgTimeStamp GetRawTimeStamp() const
    {
    return vgTimeStamp(*this);
    }

#ifdef VTKEXTENSIONS_USE_VIDTK
  // Description:
  // Pass a vidtk::timestamp.
  explicit vtkVgTimeStamp(const vidtk::timestamp& vidtk_timestamp)
    {
    if (vidtk_timestamp.has_time())
      {
      this->Time = vidtk_timestamp.time();
      }
    if (vidtk_timestamp.has_frame_number())
      {
      this->FrameNumber = vidtk_timestamp.frame_number();
      }
    }

  // Description:
  // Assign from a vidtk::timestamp.
  vtkVgTimeStamp& operator=(const vidtk::timestamp& vidtk_timestamp)
    {
    if (vidtk_timestamp.has_time())
      {
      this->Time = vidtk_timestamp.time();
      }
    else
      {
      this->Time = vgTimeStamp::InvalidTime();
      }
    if (vidtk_timestamp.has_frame_number())
      {
      this->FrameNumber = vidtk_timestamp.frame_number();
      }
    else
      {
      this->FrameNumber = vgTimeStamp::InvalidFrameNumber();
      }
    return *this;
    }
#endif

  // Description:
  // Pass a time.
  // The constructor is explicit to make sure that the user did
  // intend to create a timestamp that does not have a frame number.
  explicit vtkVgTimeStamp(double time) : vgTimeStamp(time) {}

  // Description:
  // Constructor to initialize to min or max time
  explicit vtkVgTimeStamp(bool maxTime)
    {
    if (maxTime)
      {
      this->SetToMaxTime();
      }
    else
      {
      this->SetToMinTime();
      }
    }

  /// pass a time and a frame number.
  vtkVgTimeStamp(double time, unsigned int frameNumber)
    {
    this->Time = time;
    this->FrameNumber = frameNumber;
    }

  // Description:
  // Returns true if contains time or frame number
  bool IsValid() const
    {
    return vgTimeStamp::IsValid();
    }

  // Description:
  // Reset to initialized state (no time or frame #)
  void Reset()
    {
    this->Time = vgTimeStamp::InvalidTime();
    this->FrameNumber = vgTimeStamp::InvalidFrameNumber();
    }

  // Description:
  // Set time to large value, any comparison (frame or time), will have this
  // timestamp grater than other, unless both at max time
  void SetToMaxTime()
    {
    this->Time = VTK_DOUBLE_MAX;
    }

  // Return whether or not this timestamp set to max time
  bool IsMaxTime() const
    {
    return this->Time == VTK_DOUBLE_MAX;
    }

  void SetToMinTime()
    {
    this->Time = VTK_DOUBLE_MIN;
    }
  bool IsMinTime() const
    {
    return this->Time == VTK_DOUBLE_MIN;
    }

  // Description:
  // Returns true if this contains time, otherwise false.
  bool HasTime() const
    {
    return vgTimeStamp::HasTime();
    }

  // Description:
  // Set/Get the time.
  void SetTime(double time)
    {
    this->Time = time;
    }
  double GetTime() const
    {
    return this->Time;
    }

  // Description:
  // Return true if frame number is found, otherwise false.
  bool HasFrameNumber()  const
    {
    return vgTimeStamp::HasFrameNumber();
    }

  // Description:
  // Set/Get frame number.
  void SetFrameNumber(unsigned int number)
    {
    this->FrameNumber = number;
    }
  unsigned int GetFrameNumber() const
    {
    return this->FrameNumber;
    }

  // Description:
  // Return the time or frame number, depending on availability.
  // If a time value is available, this will be a time value in seconds.  If
  // not, it will be the frame number.
  double GetTimeInSecs() const
    {
    return this->HasTime() ? this->Time / 1e6 : double(this->FrameNumber);
    }

  // Description:
  // Return the time or frame number difference, depending on
  // availability.
  //
  // If a time value is available in both numbers, the result is the
  // difference expressed in seconds.  Otherwise, if the frame number
  // if available in both, the result is the difference in frame
  // numbers.  Finally, if neither of these hold, the result is
  // undefined.
  double GetTimeDifferenceInSecs(const vtkVgTimeStamp& other) const;

  // Description:
  // Ordering on timestamps.
  // If the timestamps have time values, the order is based on time.
  // Otherwise, if neither have time values, but have frame numbers,
  // the order is based on frame number.  Otherwise, the comparison
  // will always return \c false.
  //
  // Note that this means if one has a time value but the other does
  // not, the result will be \c false.
  bool operator<(const vtkVgTimeStamp& other) const;

  bool operator>(const vtkVgTimeStamp& other) const
    {
    return other < *this;
    }

  // Description:
  // Ordering on timestamps.
  bool operator<(const vgTimeStamp& other) const
    {
    return *this < vtkVgTimeStamp(other);
    }

  bool operator>(const vgTimeStamp& other) const
    {
    return *this > vtkVgTimeStamp(other);
    }

  // Description:
  // Equality of timestamps.
  // If the timestamps have time values, the comparison is based on
  // time.  If neither have time values, but both have frame numbers,
  // the comparison is based on frame number.  In all other cases,
  // the result is \c false.
  bool operator==(const vtkVgTimeStamp& other) const;
  bool operator!=(const vtkVgTimeStamp& other) const
    {
    return !(*this == other);
    }

  // Description:
  // Combined ordering and equality operator
  bool operator<=(const vtkVgTimeStamp& other) const
    {
    return (*this < other) || (*this == other);
    }

  bool operator>=(const vtkVgTimeStamp& other) const
    {
    return other <= *this;
    }

  // Description:
  // Provide subtraction operators
  vtkVgTimeStamp& operator-(const double& timeInterval);
  vtkVgTimeStamp& operator-(const unsigned int& frameInterval);

  // Description:
  // Provide addition operators
  vtkVgTimeStamp& operator+(const double& timeInterval);
  vtkVgTimeStamp& operator+(const unsigned int& frameInterval);

  bool FuzzyEquals(const vtkVgTimeStamp& other, double tol = 1e-5) const
    {
    return this->vgTimeStamp::FuzzyEquals(other.GetRawTimeStamp(), tol);
    }

  // Description:
  // Shift the timestamp forward
  void ShiftForward(const vtkVgTimeStamp& other);

  // Description:
  // Shift the timestamp backward
  void ShiftBackward(const vtkVgTimeStamp& other);
};

// Description:
// Ordering on timestamps.
inline bool operator<(const vgTimeStamp& a, const vtkVgTimeStamp& b)
{
  return vtkVgTimeStamp(a) < b;
}

// Description:
// Provide global addition operators
extern vtkVgTimeStamp& operator+(const double& timeInterval,
                                  vtkVgTimeStamp& timestamp);
extern vtkVgTimeStamp& operator+(const int& frameInterval,
                                  vtkVgTimeStamp& timestamp);

#endif // __vtkVgTimeStamp_h
