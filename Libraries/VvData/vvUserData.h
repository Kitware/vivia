/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
