/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
