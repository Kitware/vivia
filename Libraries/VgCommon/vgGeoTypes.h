/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgGeoTypes_h
#define __vgGeoTypes_h

#include <vector>

//-----------------------------------------------------------------------------
struct vgGeoSystem
{
  /*implicit*/ vgGeoSystem(int gcs = -1) : GCS(gcs) {}
  int GCS; // EPSG CRS (coordinate reference system) code
};

//-----------------------------------------------------------------------------
struct vgGeoRawCoordinate
{
  vgGeoRawCoordinate() : Northing(0), Easting(0) {}
  vgGeoRawCoordinate(double northing, double easting)
    : Northing(northing), Easting(easting) {}
  union
    {
    double Northing;
    double Latitude;
    };
  union
    {
    double Easting;
    double Longitude;
    };
};

//-----------------------------------------------------------------------------
struct vgGeocodedCoordinate : vgGeoSystem, vgGeoRawCoordinate
{
  vgGeocodedCoordinate() : vgGeoSystem(), vgGeoRawCoordinate() {}
  vgGeocodedCoordinate(vgGeoRawCoordinate coord, vgGeoSystem gcs)
    : vgGeoSystem(gcs), vgGeoRawCoordinate(coord) {}
  vgGeocodedCoordinate(double northing, double easting, vgGeoSystem gcs)
    : vgGeoSystem(gcs), vgGeoRawCoordinate(northing, easting) {}
};

//-----------------------------------------------------------------------------
template <int K>
struct vgGeocodedFixedPoly : vgGeoSystem
{
  vgGeoRawCoordinate Coordinate[K];
};

typedef vgGeocodedFixedPoly<4> vgGeocodedTile;

//-----------------------------------------------------------------------------
struct vgGeocodedPoly : vgGeoSystem
{
  std::vector<vgGeoRawCoordinate> Coordinate;
};

#endif
