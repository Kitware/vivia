// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqExporter_h
#define __vqExporter_h

#include <vvQueryResult.h>

class vqExporter
{
public:
  vqExporter() {}
  virtual ~vqExporter() {}

  virtual bool exportResults(const QList<vvQueryResult>& results) = 0;
};

#endif // __vqExporter_h
