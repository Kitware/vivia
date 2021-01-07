// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgUtil_h
#define __vgUtil_h

//-----------------------------------------------------------------------------
template <typename T>
inline void vgExpandLowerBoundary(T& lowerBound, const T& value)
{ if (value < lowerBound) lowerBound = value; }

//-----------------------------------------------------------------------------
template <typename T>
inline void vgExpandUpperBoundary(T& upperBound, const T& value)
{ if (upperBound < value) upperBound = value; }

//-----------------------------------------------------------------------------
template <typename T>
inline void vgExpandBoundaries(T& lowerBound, T& upperBound, const T& value)
{
  vgExpandLowerBoundary(lowerBound, value);
  vgExpandUpperBoundary(upperBound, value);
}

//-----------------------------------------------------------------------------
template <typename T>
inline void vgTruncateLowerBoundary(T& lowerBound, const T& value)
{ vgExpandUpperBoundary(lowerBound, value); }

//-----------------------------------------------------------------------------
template <typename T>
inline void vgTruncateUpperBoundary(T& upperBound, const T& value)
{ vgExpandLowerBoundary(upperBound, value); }

//-----------------------------------------------------------------------------
template <typename T>
inline void vgBound(T& value, const T& lowerBound, const T& upperBound)
{
  vgTruncateLowerBoundary(value, lowerBound);
  vgTruncateUpperBoundary(value, upperBound);
}

#endif
