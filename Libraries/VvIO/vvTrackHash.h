// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvTrackHash_h
#define __vvTrackHash_h

#include <vvTrack.h>

#include <QHashFunctions>

//-----------------------------------------------------------------------------
Q_DECL_CONST_FUNCTION Q_DECL_CONSTEXPR inline unsigned int qHash(
  const vvTrackId& tid, unsigned int seed = 0) Q_DECL_NOTHROW
{
  constexpr static int sourceRotateBits = 4;
  constexpr static int sourceReverseRotateBits =
    ((8 * sizeof(unsigned int)) - sourceRotateBits);

  unsigned int source = static_cast<unsigned int>(tid.Source);
  unsigned int sourceHash = (source >> sourceRotateBits) & (~0U);
  sourceHash ^= (source << sourceReverseRotateBits & (~0U));
  return qHash(tid.SerialNumber, sourceHash);
}

#endif
