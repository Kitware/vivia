// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
