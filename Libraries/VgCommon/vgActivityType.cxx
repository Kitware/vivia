/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgActivityType.h"

#include <assert.h>
#include <string.h>

//-----------------------------------------------------------------------------
static const char* DisplayModeStrings[vgActivityType::NumDisplayModes] =
{
  "Events",
  "Bounds",
};

//-----------------------------------------------------------------------------
vgActivityType::vgActivityType()
{
  this->UseRandomColors = true;

  this->Id[0] = '\0';
  this->DisplayMode = DM_Events;
}

#ifdef _WIN32
#pragma warning(disable:4996) // 'strncpy' unsafe
#endif

//-----------------------------------------------------------------------------
void vgActivityType::SetId(const char* id)
{
  strncpy(this->Id, id, sizeof(this->Id) - 1);
  this->Id[sizeof(this->Id) - 1] = '\0';
}

//-----------------------------------------------------------------------------
const char* vgActivityType::GetDisplayModeString(int mode)
{
  assert(mode >= 0 && mode < NumDisplayModes);
  return DisplayModeStrings[mode];
}
