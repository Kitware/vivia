/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
