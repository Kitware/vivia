/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqExporter_h
#define __vqExporter_h

#include <vvQueryResult.h>

template <typename T> class QList;

class vqExporter
{
public:
  vqExporter() {}
  virtual ~vqExporter() {}

  virtual bool exportResults(const QList<vvQueryResult>& results) = 0;
};

#endif
