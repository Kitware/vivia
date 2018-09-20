/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgGeodesy_h
#define __vgGeodesy_h

#include <vgExport.h>

#include <string>

struct vgGeocodedCoordinate;
struct vgGeocodedPoly;

template <int K> struct vgGeocodedFixedPoly;
using vgGeocodedTile = vgGeocodedFixedPoly<4>;

namespace vgGeodesy
{
  enum GCS
    {
    LatLon_Wgs84 = 4326,
    LatLon_Nad83 = 4269,
    UTM_Wgs84North = 32600, // Add zone number to get zoned GCS
    UTM_Wgs84South = 32700, // Add zone number to get zoned GCS
    UTM_Nad83NorthEast = 3313, // Add zone number to get zoned GCS (59N - 60N)
    UTM_Nad83NorthWest = 26900, // Add zone number to get zoned GCS (1N - 23N)
    };

  VG_COMMON_EXPORT vgGeocodedCoordinate regionCenter(const vgGeocodedPoly&);

  VG_COMMON_EXPORT vgGeocodedPoly generateRegion(
    const vgGeocodedCoordinate& center, double diameter);

  VG_COMMON_EXPORT vgGeocodedPoly regionFromMgrs(const char*);

  VG_COMMON_EXPORT vgGeocodedCoordinate convertGcs(
    const vgGeocodedCoordinate&, int desiredGcs);

  VG_COMMON_EXPORT vgGeocodedTile convertGcs(
    const vgGeocodedTile&, int desiredGcs);

  VG_COMMON_EXPORT vgGeocodedPoly convertGcs(
    const vgGeocodedPoly&, int desiredGcs);

  VG_COMMON_EXPORT std::string mgrs(
    const vgGeocodedCoordinate&, int precision = 5);
}

#endif
