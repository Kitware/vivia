// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
