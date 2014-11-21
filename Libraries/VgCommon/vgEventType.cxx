/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgEventType.h"

#include <assert.h>
#include <string.h>

//-----------------------------------------------------------------------------
static const char* DisplayModeStrings[vgEventType::NumDisplayModes] =
{
  "Tracks",
  "Regions",
  "TracksAndRegions"
};

//-----------------------------------------------------------------------------
vgEventType::vgEventType() :
  Id(-1), MinTracks(-1), MaxTracks(-1), SecondaryIconIndex(-1),
  DisplayMode(DM_Tracks)
{
}

#ifdef _WIN32
#pragma warning(disable:4996) // 'strncpy' unsafe
#endif

//-----------------------------------------------------------------------------
void vgEventType::GetTrackColor(int trackIndex,
                                double& r, double& g, double& b) const
{
  assert(trackIndex >= 0);

  const double* color = this->Color;;
  if (trackIndex > 0 && this->HasSecondaryColor)
    {
    color = this->SecondaryColor;
    }

  r = color[0];
  g = color[1];
  b = color[2];
}

//-----------------------------------------------------------------------------
int vgEventType::GetTrackIconIndex(int trackIndex) const
{
  assert(trackIndex >= 0);

  int iconIndex = this->IconIndex;
  if (trackIndex > 0 && this->SecondaryIconIndex >= 0)
    {
    iconIndex = this->SecondaryIconIndex;
    }
  return iconIndex;
}

//-----------------------------------------------------------------------------
const char* vgEventType::GetDisplayModeString(int mode)
{
  assert(mode >= 0 && mode < NumDisplayModes);
  return DisplayModeStrings[mode];
}
