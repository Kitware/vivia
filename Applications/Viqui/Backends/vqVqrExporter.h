// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqVqrExporter_h
#define __vqVqrExporter_h

#include "vqExporter.h"

class vqVqrExporter : public vqExporter
{
public:
  vqVqrExporter() {}
  ~vqVqrExporter() {}

  virtual bool exportResults(const QList<vvQueryResult>& results);
};

#endif // __vqVqrExporter_h
