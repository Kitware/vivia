/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileEventReader_h
#define __vpFileEventReader_h

class QString;

class vpEventIO;

class vpFileEventReader
{
public:
  vpFileEventReader(vpEventIO* io);

  bool ReadEventLinks(const QString& eventLinksFileName) const;

protected:
  vpEventIO* const IO;
};

#endif
