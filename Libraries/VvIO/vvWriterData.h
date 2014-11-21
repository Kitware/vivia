/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvWriterData_h
#define __vvWriterData_h

#include <vvQueryResult.h>

#include "vvEventSetInfo.h"
#include "vvHeader.h"
#include "vvQueryInstance.h"

struct vvSpatialFilter
{
  vvSpatialFilter() : filter(0) {}
  vvSpatialFilter(const vgGeocodedPoly& p) : poly(p), filter(0) {}
  vvSpatialFilter(const vgGeocodedPoly& p,
                  vvDatabaseQuery::IntersectionType f)
    : poly(p), filter(new vvDatabaseQuery::IntersectionType(f)) {}
  ~vvSpatialFilter() { delete filter; }

  vgGeocodedPoly poly;
  vvDatabaseQuery::IntersectionType* filter;
};

#endif
