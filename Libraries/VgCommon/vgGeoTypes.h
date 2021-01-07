// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
  constexpr static int Size = K;
};

using vgGeocodedTile = vgGeocodedFixedPoly<4>;

//-----------------------------------------------------------------------------
struct vgGeocodedPoly : vgGeoSystem
{
  std::vector<vgGeoRawCoordinate> Coordinate;
};

#endif
