/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgRange_h
#define __vgRange_h

template <typename T>
class vgRange
{
public:
  vgRange() {}
  vgRange(T l, T u) : lower(l), upper(u) {}
  vgRange(const vgRange<T>& other) : lower(other.lower), upper(other.upper) {}
  vgRange& operator=(const vgRange<T>& other)
    { this->lower = other.lower; this->upper = other.upper; return *this; }

  bool operator==(const vgRange<T>& other) const
    { return this->lower == other.lower && this->upper == other.upper; }
  bool operator!=(const vgRange<T>& other) const
    { return this->lower != other.lower || this->upper != other.upper; }

  bool operator<(const vgRange<T>& other) const
    {
    return (this->lower < other.lower) ||
           ((this->lower == other.lower) && (this->upper < other.upper));
    }

  T lower;
  T upper;
};

#endif
