// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvUserData_h
#define __vvUserData_h

#include <vgFlags.h>

#include <string>

//-----------------------------------------------------------------------------
namespace vvUserData
{
  enum Flag
    {
    Starred = 1 << 0
    };

  VG_DECLARE_FLAGS(Flags, Flag)

  struct Data
    {
    vvUserData::Flags Flags;
    std::string Notes;
    };
}

VG_DECLARE_OPERATORS_FOR_FLAGS(vvUserData::Flags)

#endif
