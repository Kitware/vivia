/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileTrackIOImpl_h
#define __vpFileTrackIOImpl_h

#include <string>

class vpTrackIO;

class vpFileTrackIOImpl
{
public:
  static bool ReadTrackTraits(vpTrackIO* io,
                              const std::string& trackTraitsFileName);

  static bool ReadRegionsFile(vpTrackIO* io,
                              const std::string& tracksFileName,
                              int frameOffset,
                              float offsetX = 0.0f, float offsetY = 0.0f);

  static void ReadTypesFile(vpTrackIO* io, const std::string& tracksFileName);
};

#endif // __vpFileTrackIOImpl_h
