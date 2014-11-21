/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgTimeMap_h
#define __vgTimeMap_h

#include <QMap>

#include <vgTimeStamp.h>
#include <vgNamespace.h>

//-----------------------------------------------------------------------------
template <typename Value>
class vgTimeMap : public QMap<vgTimeStamp, Value>
{
public:
  typedef typename QMap<vgTimeStamp, Value>::iterator iterator;
  typedef typename QMap<vgTimeStamp, Value>::const_iterator const_iterator;

  vgTimeMap() {}
  vgTimeMap(const vgTimeMap<Value>& other) : QMap<vgTimeStamp, Value>(other) {}
  ~vgTimeMap() {}

  using QMap<vgTimeStamp, Value>::find;
  using QMap<vgTimeStamp, Value>::constFind;

  iterator find(vgTimeStamp pos, vg::SeekMode direction);
  const_iterator find(vgTimeStamp pos, vg::SeekMode direction) const;
  const_iterator constFind(vgTimeStamp pos, vg::SeekMode direction) const;
};

//-----------------------------------------------------------------------------
template <typename Value>
typename vgTimeMap<Value>::const_iterator vgTimeMap<Value>::constFind(
  vgTimeStamp pos, vg::SeekMode direction) const
{
  // Check for empty map or invalid request
  if (this->count() < 1 || !pos.IsValid())
    {
    return this->constEnd();
    }

  const_iterator iter;

  switch (direction)
    {
    case vg::SeekExact:
      // Exact is easy, find() does what we need
      return this->find(pos);

    case vg::SeekLowerBound:
      // Lower is easy, lowerBound() does what we need
      return this->lowerBound(pos);

    case vg::SeekNext:
      // Next is easy, upperBound() does what we need
      return this->upperBound(pos);

    case vg::SeekUpperBound:
      // Check first for exact match
      iter = this->find(pos);
      if (iter != this->end())
        {
        return iter;
        }

      // Check for an answer
      if (pos < this->begin().key())
        {
        return this->end();
        }

      // upperBound() gets us the position *after* the one we want, so return
      // the preceding position (we know this exists because count() > 0)
      return --this->upperBound(pos);

    case vg::SeekPrevious:
      // Check for an answer
      if (pos <= this->begin().key())
        {
        return this->end();
        }

      // lowerBound() gets us the position *after* the one we want, so return
      // the preceding position (we know this exists because of the previous
      // test)
      return --this->lowerBound(pos);


    default: // Nearest
      // Find the first item >= pos; it will be this or the one preceding
      iter = this->upperBound(pos);
      // If upperBound() falls off the end, we want the last valid position
      if (iter == this->end())
        {
        return --iter;
        }
      // If item == pos, return that
      else if (iter == this->begin())
        {
        return iter;
        }
      else
        {
        // Check if the distance to next is less than to previous
        return ((iter.key() - pos) < (pos - (iter - 1).key()))
                 ? iter // Yes; return next
                 : --iter; // No; return previous
        }
    }
}

//-----------------------------------------------------------------------------
template <typename Value>
typename vgTimeMap<Value>::const_iterator vgTimeMap<Value>::find(
  vgTimeStamp pos, vg::SeekMode direction) const
{
  return this->constFind(pos, direction);
}

//-----------------------------------------------------------------------------
template <typename Value>
typename vgTimeMap<Value>::iterator vgTimeMap<Value>::find(
  vgTimeStamp pos, vg::SeekMode direction)
{
  return iterator(this->constFind(pos, direction));
}

#endif
