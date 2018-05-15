/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqNoaaCsvExporter_h
#define __vqNoaaCsvExporter_h

#include "vqExporter.h"

class vqNoaaCsvExporter : public vqExporter
{
public:
  vqNoaaCsvExporter() {}
  ~vqNoaaCsvExporter() {}

  virtual bool exportResults(const QList<vvQueryResult>& results);
};

#endif // __vqNoaaCsvExporter_h
