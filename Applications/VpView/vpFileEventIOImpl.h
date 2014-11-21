/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileEventIOImpl_h
#define __vpFileEventIOImpl_h

#include <string>

class vpEventIO;

class vpFileEventIOImpl
{
public:
  static bool ReadEventLinks(vpEventIO* io,
                             const std::string& eventLinksFileName);
};

#endif // __vpFileEventIOImpl_h
