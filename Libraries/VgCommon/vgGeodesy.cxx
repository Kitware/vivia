/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgGeodesy.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#include <proj_api.h>

#include <GeographicLib/GeoCoords.hpp>
#include <GeographicLib/MGRS.hpp>
#include <GeographicLib/UTMUPS.hpp>

#include "vgGeoTypes.h"

namespace // anonymous
{

//-----------------------------------------------------------------------------
projPJ adaptEPSG(int gcs)
{
  std::stringstream proj4Arg;
  proj4Arg << "+init=epsg:" << gcs;
  return pj_init_plus(proj4Arg.str().c_str());
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate transformCoordinate(
  const vgGeocodedCoordinate& inCoord,
  const projPJ& fromProj, const projPJ& toProj, int toGcs)
{
  vgGeocodedCoordinate out;

  double x = inCoord.Easting;
  double y = inCoord.Northing;
  double z = 0.0;

  if (pj_is_latlong(fromProj))
    {
    x *= DEG_TO_RAD;
    y *= DEG_TO_RAD;
    }

  int err = pj_transform(fromProj, toProj, 1, 1, &x, &y, &z);

  if (err == 0)
    {
    out.GCS = toGcs;
    if (pj_is_latlong(toProj))
      {
      out.Longitude = x * RAD_TO_DEG;
      out.Latitude = y * RAD_TO_DEG;
      }
    else
      {
      out.Easting = x;
      out.Northing = y;
      }
    }
  else
    {
    std::cerr << "Coordinate transformation failed: error " << err << '\n';
    }

  return out;
}

//-----------------------------------------------------------------------------
projPJ createUTMProj(const vgGeocodedCoordinate& inCoord, int& outGcs)
{
  projPJ inProj = adaptEPSG(inCoord.GCS);

  static const char* baseInit = "+proj=utm +ellps=WGS84 +datum=WGS84 +units=m";
  if (inCoord.GCS < 0)
    {
    // Default
    return pj_init_plus(baseInit);
    }
  else if (inProj && pj_is_latlong(inProj))
    {
    int zone;
    bool northp;
    double x, y;

    try
      {
      GeographicLib::UTMUPS::Forward(inCoord.Latitude, inCoord.Longitude,
                                     zone, northp, x, y);
      }
    catch (std::exception& e)
      {
      std::cerr << "Caught exception: " << e.what() << '\n';
      return 0;
      }

    std::stringstream proj4Arg;
    proj4Arg << baseInit << " +zone=" << zone;

    // This is a must as it adds a false northing of 10,000,000m
    if (!northp)
      {
      proj4Arg << " +south";
      outGcs = vgGeodesy::UTM_Wgs84South + zone;
      }
    else
      {
      outGcs = vgGeodesy::UTM_Wgs84North + zone;
      }

    return pj_init_plus(proj4Arg.str().c_str());
    }
  else
    {
    projPJ outProj = adaptEPSG(vgGeodesy::LatLon_Wgs84);

    if (!inProj || !outProj)
      {
      return 0;
      }

    const vgGeocodedCoordinate coord =
      transformCoordinate(inCoord, inProj, outProj, vgGeodesy::LatLon_Wgs84);

    return createUTMProj(coord, outGcs);
    }
}

//-----------------------------------------------------------------------------
void createRegion(vgGeocodedPoly& poly, vgGeocodedCoordinate centerInUTM,
                  double diameter)
{
  double radius = diameter * 0.5;

  vgGeoRawCoordinate tl, tr, br, bl;

  tl.Easting  = centerInUTM.Easting - radius;
  tl.Northing = centerInUTM.Northing + radius;

  tr.Easting  = centerInUTM.Easting + radius;
  tr.Northing = centerInUTM.Northing + radius;

  br.Easting  = centerInUTM.Easting + radius;
  br.Northing = centerInUTM.Northing - radius;

  bl.Easting  = centerInUTM.Easting - radius;
  bl.Northing = centerInUTM.Northing - radius;

  poly.Coordinate.push_back(tl);
  poly.Coordinate.push_back(tr);
  poly.Coordinate.push_back(br);
  poly.Coordinate.push_back(bl);

  poly.GCS = centerInUTM.GCS;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vgGeocodedCoordinate vgGeodesy::regionCenter(const vgGeocodedPoly& region)
{
  vgGeocodedCoordinate center;
  center.Latitude = center.Longitude = 0.0;
  center.GCS = region.GCS;

  const double f = 1.0 / static_cast<double>(region.Coordinate.size());
  size_t n = region.Coordinate.size();
  while (n--)
    {
    center.Latitude += f * region.Coordinate[n].Latitude;
    center.Longitude += f * region.Coordinate[n].Longitude;
    }

  return center;
}

//-----------------------------------------------------------------------------
vgGeocodedPoly vgGeodesy::generateRegion(
  const vgGeocodedCoordinate& center, double diameter)
{
  vgGeocodedPoly out;
  int outGcs;

  projPJ fromProj = adaptEPSG(center.GCS);
  projPJ toProj = createUTMProj(center, outGcs);

  if (!fromProj || !toProj)
    {
    return out;
    }

  const vgGeocodedCoordinate utmCoord =
    transformCoordinate(center, fromProj, toProj, outGcs);

  createRegion(out, utmCoord, diameter);

  // Release resources
  pj_free(fromProj);
  pj_free(toProj);

  return out;
}

//-----------------------------------------------------------------------------
vgGeocodedPoly vgGeodesy::regionFromMgrs(const char* mgrs)
{
  // First convert MGRS to UTM
  int zone, prec;
  bool northp;
  double x, y;

  try
    {
    GeographicLib::MGRS::Reverse(mgrs, zone, northp, x, y, prec);

    // Then UTM to lat lon
    vgGeocodedCoordinate coord;
    double lat, lon;
    GeographicLib::UTMUPS::Reverse(zone, northp, x, y, lat, lon);

    coord.Latitude = lat;
    coord.Longitude = lon;

    // UTMUPS returns lat lon using WGS84 ellps and datum
    coord.GCS = vgGeodesy::LatLon_Wgs84;

    // (100m * 1km) / 1e<prec> == 1e5 * 1e<-prec> = 1e(5 - prec)
    double diameter = std::pow(10.0, 5 - prec);

    return generateRegion(coord, diameter);
    }
  catch (const std::exception& e)
    {
    std::cerr << "Caught exception: " << e.what() << '\n';

    return vgGeocodedPoly();
    }
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate vgGeodesy::convertGcs(
  const vgGeocodedCoordinate& in, int desiredGcs)
{
  // Validate input
  if (in.GCS < 0)
    return vgGeocodedCoordinate();

  // Check for no-op
  if (in.GCS == desiredGcs)
    return in;

  projPJ fromProj = adaptEPSG(in.GCS);
  projPJ toProj = adaptEPSG(desiredGcs);

  if (!fromProj || !toProj)
    {
    return vgGeocodedCoordinate();
    }

  const vgGeocodedCoordinate out =
    transformCoordinate(in, fromProj, toProj, desiredGcs);

  // Release resources
  pj_free(fromProj);
  pj_free(toProj);

  return out;
}

//-----------------------------------------------------------------------------
vgGeocodedTile vgGeodesy::convertGcs(const vgGeocodedTile& in, int desiredGcs)
{
  // Validate input
  if (in.GCS < 0)
    return {};

  // Check for no-op
  if (in.GCS == desiredGcs)
    return in;

  // Prepare for conversion
  vgGeocodedCoordinate temp;
  vgGeoRawCoordinate& raw = temp;
  temp.GCS = in.GCS;
  vgGeocodedTile out;

  // Convert points
  for (int n = 0; n < in.Size; ++n)
    {
    raw = in.Coordinate[n];
    out.Coordinate[n] = convertGcs(temp, desiredGcs);
    }

  out.GCS = desiredGcs;

  // Return converted result
  return out;
}

//-----------------------------------------------------------------------------
vgGeocodedPoly vgGeodesy::convertGcs(const vgGeocodedPoly& in, int desiredGcs)
{
  // Validate input
  if (in.GCS < 0)
    return vgGeocodedPoly();

  // Check for no-op
  if (in.GCS == desiredGcs)
    return in;

  // Prepare for conversion
  vgGeocodedCoordinate temp;
  vgGeoRawCoordinate& raw = temp;
  temp.GCS = in.GCS;
  vgGeocodedPoly out;

  // Convert points
  for (size_t n = 0, k = in.Coordinate.size(); n < k; ++n)
    {
    raw = in.Coordinate[n];
    out.Coordinate.push_back(convertGcs(temp, desiredGcs));
    }

  out.GCS = desiredGcs;

  // Return converted result
  return out;
}

//-----------------------------------------------------------------------------
std::string vgGeodesy::mgrs(const vgGeocodedCoordinate& in, int precision)
{
  // Convert to WGS84
  // TODO optimize for input already in UTM
  const vgGeocodedCoordinate cc = convertGcs(in, LatLon_Wgs84);

  // Test if input is valid (do after conversion so we only need to test once
  // to handle also failed conversion; conversion will have failed quickly if
  // the original input was invalid)
  if (cc.GCS < 0)
    {
    return "(invalid)";
    }

  // Convert to UTM
  int zone;
  bool northp;
  double x, y;
  GeographicLib::UTMUPS::Forward(cc.Latitude, cc.Longitude, zone, northp, x, y);

  // Convert to MGRS
  std::string mgrs;
  GeographicLib::MGRS::Forward(zone, northp, x, y, precision, mgrs);
  return mgrs;
}
