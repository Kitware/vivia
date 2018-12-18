/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileEventReader_h
#define __vpFileEventReader_h

#include <string>

class vpEventIO;

class vpFileEventReader
{
public:
  vpFileEventReader(vpEventIO* io);

  bool ReadEventLinks(const std::string& eventLinksFileName) const;

protected:
  vpEventIO* const IO;
};

#endif
