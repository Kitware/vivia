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

  static bool ReadSupplementalFiles(vpTrackIO* io,
                                    const std::string& tracksFileName);

  static bool ImportSupplementalFiles(vpTrackIO* io,
                                      const std::string& tracksFileName,
                                      float offsetX, float offsetY);
};

#endif // __vpFileTrackIOImpl_h
