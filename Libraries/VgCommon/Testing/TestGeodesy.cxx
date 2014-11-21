/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <qtTest.h>

#include "../vgGeodesy.h"
#include "../vgGeoTypes.h"

static const double TestNorthingLatLonWgs84 = 42.849787;
static const double TestEastingLatLonWgs84 = -73.758967;

static const double TestNorthingUtmWgs84 = 4744881.0280075;
static const double TestEastingUtmWgs84 =   601401.2262888;

const int TestUtmZoneGcs = vgGeodesy::UTM_Wgs84North + 18;

//-----------------------------------------------------------------------------
bool operator!=(const vgGeocodedCoordinate& a, const vgGeocodedCoordinate& b)
{
  return (a.GCS != b.GCS ||
          !qFuzzyCompare(a.Northing, b.Northing) ||
          !qFuzzyCompare(a.Easting, b.Easting));
}

//-----------------------------------------------------------------------------
QDebug& operator<<(QDebug& dbg, const vgGeocodedCoordinate& coord)
{
  if (coord.GCS != -1)
    {
    dbg << '{' << coord.GCS
        << coord.Northing << 'N'
        << coord.Easting << 'E'
        << '}';
    }
  else
    {
    dbg << "(invalid)";
    }
  return dbg;
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate llCoord()
{
  return vgGeocodedCoordinate(TestNorthingLatLonWgs84, TestEastingLatLonWgs84,
                              vgGeodesy::LatLon_Wgs84);
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate utmCoord()
{
  return vgGeocodedCoordinate(TestNorthingUtmWgs84, TestEastingUtmWgs84,
                              TestUtmZoneGcs);
}

//-----------------------------------------------------------------------------
int testRegionPoint(qtTest& testObject, const vgGeocodedPoly& region,
                    size_t index, double radius)
{
  static const vgGeocodedCoordinate center = utmCoord();

  vgGeocodedCoordinate c(region.Coordinate[index], region.GCS);
  c = vgGeodesy::convertGcs(c, TestUtmZoneGcs);

  const double dN = qAbs(c.Northing - center.Northing);
  const double dE = qAbs(c.Easting - center.Easting);
  const double epsilon = 0.01 * radius;

  int failed = 0;
  failed |= TEST(qAbs(dN - radius) < epsilon);
  failed |= TEST(qAbs(dE - radius) < epsilon);

  if (failed)
    {
    qDebug() << "validation of region point" << c << "at index" << index
             << "failed";
    qDebug() << "  expected distance from center:" << radius;
    qDebug() << "  actual distances:" << dN << 'N' << dE << 'E';
    }

  return failed;
}

//-----------------------------------------------------------------------------
int testNoopConversion(qtTest& testObject)
{
  const vgGeocodedCoordinate c1 = llCoord();
  TEST_EQUAL(vgGeodesy::convertGcs(c1, vgGeodesy::LatLon_Wgs84), c1);

  const vgGeocodedCoordinate c2 = utmCoord();
  TEST_EQUAL(vgGeodesy::convertGcs(c2, TestUtmZoneGcs), c2);

  return 0;
}

//-----------------------------------------------------------------------------
int testLatLonToUtmConversion(qtTest& testObject)
{
  const vgGeocodedCoordinate cLL = llCoord();
  TEST_EQUAL(vgGeodesy::convertGcs(cLL, TestUtmZoneGcs), utmCoord());

  return 0;
}

//-----------------------------------------------------------------------------
int testUtmToLatLonConversion(qtTest& testObject)
{
  const vgGeocodedCoordinate cUTM = utmCoord();
  TEST_EQUAL(vgGeodesy::convertGcs(cUTM, vgGeodesy::LatLon_Wgs84), llCoord());

  return 0;
}

//-----------------------------------------------------------------------------
int testRoundTripConversion(qtTest& testObject)
{
  const vgGeocodedCoordinate c1 =
    vgGeodesy::convertGcs(llCoord(), TestUtmZoneGcs);
  TEST_EQUAL(vgGeodesy::convertGcs(c1, vgGeodesy::LatLon_Wgs84), llCoord());

  const vgGeocodedCoordinate c2 =
    vgGeodesy::convertGcs(utmCoord(), vgGeodesy::LatLon_Wgs84);
  TEST_EQUAL(vgGeodesy::convertGcs(c2, TestUtmZoneGcs), utmCoord());

  return 0;
}

//-----------------------------------------------------------------------------
int testCreateRegion(qtTest& testObject)
{
  static const double radius = 100.0;
  const vgGeocodedPoly region =
    vgGeodesy::generateRegion(llCoord(), 2.0 * radius);

  if (TEST_EQUAL(region.Coordinate.size(), static_cast<size_t>(4)))
    return 1;

  TEST_CALL(testRegionPoint, region, 0, radius);
  TEST_CALL(testRegionPoint, region, 1, radius);
  TEST_CALL(testRegionPoint, region, 2, radius);
  TEST_CALL(testRegionPoint, region, 3, radius);

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  qtTest testObject;

  testObject.runSuite("No-op Conversion Test", testNoopConversion);
  testObject.runSuite("LL -> UTM Conversion Test", testLatLonToUtmConversion);
  testObject.runSuite("UTM -> LL Conversion Test", testUtmToLatLonConversion);
  testObject.runSuite("Round Trip Conversion Test", testRoundTripConversion);
  testObject.runSuite("Generate Region Test", testCreateRegion);
//   testObject.runSuite("Region Center Test", testRegionCenter);
  return testObject.result();
}
