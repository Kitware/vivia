// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
