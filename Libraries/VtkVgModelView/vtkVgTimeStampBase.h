// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTimeStampBase_h
#define __vtkVgTimeStampBase_h

// \brief Represents a world time in microseconds (10^-6 seconds).
//
// A timestamp contains two pieces of information: a time and a frame
// number.  Generally, one should use the time value over the frame
// number if both are available.

// VTK includes.
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// VG includes.
#include "vtkVgMacros.h"

class vtkVgTimeStampBase : public vtkObject
{
public:
  // Description:
  // Easy to use vars.
  vtkVgClassMacro(vtkVgTimeStampBase);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgTimeStampBase, vtkObject);

  virtual void PrintSelf(ostream& os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os, indent);
    }

  void Copy(const vtkVgTimeStampBase& srcTimeStamp)
    {
    this->SetFrameNumber(srcTimeStamp.GetFrameNumber());
    this->SetTime(srcTimeStamp.GetTime());
    }

  // Description:
  // Returns true if this contains time, otherwise false.
  virtual bool HasTime() const = 0;

  // Description:
  // Set/Get the time.
  virtual void   SetTime(double time) = 0;
  virtual double GetTime() const      = 0;

  // Description:
  // Return true if frame number is found, otherwise false.
  virtual bool   HasFrameNumber()  const = 0;

  // Description:
  // Set/Get frame number.
  virtual void   SetFrameNumber(int number) = 0;
  virtual int    GetFrameNumber() const     = 0;

  // Description:
  // Return the time or frame number, depending on availability.
  // If a time value is available, this will be a time value in seconds.  If
  // not, it will be the frame number.
  virtual double GetTimeInSecs() const = 0;

  // Description:
  // \brief Return the time or frame number difference, depending on
  // availability.
  //
  // The return value is conceptually <tt>*this - \a other</tt>
  // expressed in seconds or in frames.
  //
  // If a time value is available in both numbers, the result is the
  // difference expressed in seconds.  Otherwise, if the frame number
  // if available in both, the result is the difference in frame
  // numbers.  Finally, if neither of these hold, the result is
  // undefined.
  virtual double GetTimeDifferenceInSecs(vtkVgTimeStampBase* other) const = 0;

  // Description:
  // \brief Ordering on timestamps.
  //
  // If the timestamps have time values, the order is based on time.
  // Otherwise, if neither have time values, but have frame numbers,
  // the order is based on frame number.  Otherwise, the comparison
  // will always return \c false.
  //
  // Note that this means if one has a time value but the other does
  // not, the result will be \c false.
  virtual bool operator<(const vtkVgTimeStampBase& other) const = 0;

  // Description:
  // \brief Equality of timestamps.
  //
  // If the timestamps have time values, the comparison is based on
  // time.  If neither have time values, but both have frame numbers,
  // the comparison is based on frame number.  In all other cases,
  // the result is \c false.
  virtual bool operator==(const vtkVgTimeStampBase& other) const = 0;

  // Description:
  // \brief Combined ordering and equality operator
  //
  // Logically identical to (*this < other) || (*this == other)
  virtual bool operator<=(const vtkVgTimeStampBase& other) const = 0;

  // Description:
  // \brief Shift the timestamps.
  //
  // Shift the timestamp forward
  virtual void   ShiftForward(const vtkVgTimeStampBase& other) = 0;

  // Description:
  // Shift the timestamp backward
  virtual void   ShiftBackward(const vtkVgTimeStampBase& other) = 0;

protected:
  vtkVgTimeStampBase() {;}
  virtual ~vtkVgTimeStampBase() {;}

private:

  vtkVgTimeStampBase(const vtkVgTimeStampBase&);  // Not implemented.
  void operator=(const vtkVgTimeStampBase&);      // Not implemented.
};

#endif // __vtkVgTimeStampBase_h
