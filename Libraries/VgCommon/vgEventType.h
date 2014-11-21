/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgEventType_h
#define __vgEventType_h

#include "vgEntityType.h"

class VG_COMMON_EXPORT vgEventType : public vgEntityType
{
public:
  enum DisplayMode
    {
    DM_Tracks,
    DM_Regions,
    DM_TracksAndRegions,
    NumDisplayModes
    };

public:
  vgEventType();

  void SetId(int id) { this->Id = id; }
  int  GetId() const { return this->Id; }

  void SetMinTracks(int mt) { this->MinTracks = mt; }
  int  GetMinTracks() const { return this->MinTracks; }

  void SetMaxTracks(int mt) { this->MaxTracks = mt; }
  int  GetMaxTracks() const { return this->MaxTracks; }

  void SetSecondaryIconIndex(int index) { this->SecondaryIconIndex = index; }
  int  GetSecondaryIconIndex() const    { return this->SecondaryIconIndex; }

  void GetTrackColor(int trackIndex, double& r, double& g, double& b) const;

  int  GetTrackIconIndex(int trackIndex) const;

  void SetDisplayMode(int mode) { this->DisplayMode = mode; }
  int  GetDisplayMode() const   { return this->DisplayMode; }

  static const char* GetDisplayModeString(int mode);

private:
  int Id;

  int MinTracks;
  int MaxTracks;

  int SecondaryIconIndex;

  int DisplayMode;
};

#endif // __vgEventType_h
