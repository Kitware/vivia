// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgActivityType_h
#define __vgActivityType_h

#include "vgEntityType.h"

class VG_COMMON_EXPORT vgActivityType : public vgEntityType
{
public:
  enum DisplayMode
    {
    DM_Events,
    DM_Bounds,
    NumDisplayModes
    };

  vgActivityType();

  void SetId(const char* id);
  const char* GetId() const { return this->Id; }

  void SetDisplayMode(int mode) { this->DisplayMode = mode; }
  int  GetDisplayMode() const   { return this->DisplayMode; }

  static const char* GetDisplayModeString(int mode);

private:
  char Id[256];

  int DisplayMode;
};

#endif // __vgActivityType_h
