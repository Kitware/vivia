/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// Implements a "merged" pointer / integer pair, where the integer's bits
// are merged into the low bits of the pointer. Since malloc'd pointers are
// typically at least 8-byte aligned, this allows us to store a pointer plus
// a few bits of user data in the same amount of space as a single pointer.

#ifndef __vgPointerInt_h
#define __vgPointerInt_h

#include "vgTypes.h"

#include <assert.h>

template <typename PointerType>
class vgPointerInt
{
  uintptr Data;

  enum { UsedBits = 0x7 };

public:
  vgPointerInt() : Data(0)
    {}

  explicit vgPointerInt(PointerType ptr, unsigned integer = 0) : Data(0)
    {
    this->SetPointer(ptr);
    this->SetInt(integer);
    }

  void SetPointer(PointerType ptr)
    {
    uintptr data = reinterpret_cast<uintptr>(ptr);
    assert((data & UsedBits) == 0 && "pointer not sufficiently aligned");
    this->Data &= UsedBits;
    this->Data |= data;
    }

  PointerType GetPointer() const
    {
    return reinterpret_cast<PointerType>(this->Data & ~UsedBits);
    }

  void SetInt(unsigned integer)
    {
    assert((integer & ~UsedBits) == 0 && "data exceeds available space");
    this->Data &= ~UsedBits;
    this->Data |= integer;
    }

  unsigned GetInt() const
    {
    return static_cast<unsigned>(this->Data & UsedBits);
    }

  void SetBits(unsigned bits)
    {
    assert((bits & ~UsedBits) == 0 && "data exceeds available space");
    uintptr b = bits;
    this->Data |= b;
    }

  void ClearBits(unsigned bits)
    {
    assert((bits & ~UsedBits) == 0 && "data exceeds available space");
    uintptr b = bits;
    this->Data &= ~b;
    }

  bool GetBitsAreSet(unsigned bits) const
    {
    return (this->Data & bits) == bits;
    }
};

#endif // __vgPointerInt_h
