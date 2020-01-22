/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgGeoUtil.h"

#include <vgGeodesy.h>
#include <vgGeoTypes.h>

#include <qtKstReader.h>
#include <qtMath.h>
#include <qtStlUtil.h>

#include <QDebug>
#include <QSettings>
#include <QStringList>

namespace // anonymous
{

#define die(_msg_) do { \
  if (error) *error = _msg_; \
  return vgGeocodedCoordinate(); \
  } while(0)

//-----------------------------------------------------------------------------
const auto UTM_REGEXP = QStringLiteral(
  "(?:[1-9]|[1-5][0-9]|60)[NS]\\s+\\d+(?:\\.\\d*)?[,\\s]\\s*\\d+(?:\\.\\d*)?");
const auto UPS_REGEXP = QStringLiteral(
  "[ABYZ]\\s+\\d+(?:\\.\\d*)?[,\\s]\\s*\\d+(?:\\.\\d*)?");
const auto LLC_REGEXP = QStringLiteral(
  "([+-]?\\d+):(\\d+):(\\d+(?:\\.\\d*)?)\\s*([nsew])?");

//-----------------------------------------------------------------------------
void resolveFormatMode(vgGeodesy::CoordFormatMode& fm)
{
  if (fm == vgGeodesy::FormatFromSettings)
    {
    const int ifm =
      QSettings().value(QStringLiteral("CoordFormatMode"),
                        vgGeodesy::FormatDmsLatin1).toInt();
    fm = static_cast<vgGeodesy::CoordFormatMode>(ifm);
    }
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate parseUcs(const QString& text, QString& zone)
{
  const QStringList parts =
    text.split(QRegExp(QStringLiteral("[,\\s]")), QString::SkipEmptyParts);

  if (parts.count() != 3)
    {
    return vgGeocodedCoordinate();
    }

  // Extract northing and easting
  vgGeocodedCoordinate result;
  result.Easting = parts[1].toDouble();
  result.Northing = parts[2].toDouble();

  // Extract zone
  zone = parts[0];

  // Return result
  return result;
}

//-----------------------------------------------------------------------------
bool parseLlPart(QString part, double& out)
{
  QRegExp re(LLC_REGEXP);
  if (re.exactMatch(part))
    {
    part = QStringLiteral("%1d%2'%3\"%4").arg(re.cap(1), re.cap(2),
                                              re.cap(3), re.cap(4));
    }
  qtKstReader kst(part + QStringLiteral(";\n"));
  return kst.readReal(out);
}

//-----------------------------------------------------------------------------
qint64 decompose(double& value, double divisor, double tol)
{
  // Test for integer value
  if (fabs(round(value) - value) < tol)
    {
    // Close to integer; return value as integer and set remainder to 0
    const qint64 result = qRound64(value);
    value = 0.0;
    return result;
    }
  else
    {
    // Not an integer; get integer part and calculate remainder
    const double ipart = trunc(value);
    value = (value - ipart) * divisor;
    return qRound64(ipart);
    }
}

//-----------------------------------------------------------------------------
QString dmsString(double coord, const QString& dp, const QString& dn,
                  vgGeodesy::CoordFormatMode fm)
{
  QString resultTemplate;

  switch (fm)
    {
    case vgGeodesy::FormatDmsColon:
      resultTemplate = QStringLiteral("%1:%2:%3 %4");
      break;
    case vgGeodesy::FormatDmsAscii:
      resultTemplate = QStringLiteral("%1d%2\'%3\"%4");
      break;
    case vgGeodesy::FormatDmsLatin1:
      resultTemplate = QStringLiteral(u"%1\u00b0%2\'%3\"%4");
      break;
    case vgGeodesy::FormatDmsUnicode:
      resultTemplate = QStringLiteral(u"%1\u00b0%2\u2032%3\u2033%4");
      break;
    default:
      qWarning().nospace() << __FUNCTION__ << ": invalid format mode " << fm;
      return QString();
    }

  // Decompose into D,M,S parts, using a 'close to integer' tolerance roughly
  // equal to our display precision, so that we avoid showing silly things
  // like '19:59:60'
  const QString& ds = (coord > 0.0 ? dp : dn);
  double sp = fabs(coord);
  const qint64 dv = decompose(sp, 60.0, 0.005 / 3600.0);
  const qint64 mv = decompose(sp, 60.0, 0.005 / 60.0);
  sp = 0.01 * round(sp * 100.0);

  return resultTemplate.arg(dv).arg(mv).arg(sp, 0, 'f', 2).arg(ds);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vgGeocodedCoordinate vgGeodesy::parseCoordinate(
  const QString& text, int gcs, QString* error)
{
  // Ensure coordinate system is valid
  if (gcs != LatLon_Wgs84 && gcs != LatLon_Nad83)
    {
    die(QStringLiteral("Coordinate system not recognized."));
    }

  // Check if this looks like UTM
  const QString utext = text.toUpper();
  if (QRegExp(UTM_REGEXP).exactMatch(utext))
    {
    QString zone;
    vgGeocodedCoordinate result = parseUcs(text, zone);

    // Extract zone
    const bool north = (zone.endsWith('N'));
    const int zoneNum = zone.mid(0, zone.length() - 1).toInt();

    if (gcs == LatLon_Nad83)
      {
      // Handle NAD '83 zone (different base for 1-23, 59-60; others invalid)
      if (north && zoneNum <= 23)
        {
        result.GCS = UTM_Nad83NorthWest + zoneNum;
        }
      else if (north && zoneNum >= 59)
        {
        result.GCS = UTM_Nad83NorthEast + zoneNum;
        }
      else
        {
        static const auto format =
          QStringLiteral("UTM zone %1 not supported with NAD '83.");
        die(format.arg(zoneNum));
        }
      }
    else
      {
      // Handle WGS '84 zone
      const int base = (north ? UTM_Wgs84North : UTM_Wgs84South);
      result.GCS = base + zoneNum ;
      }

    return result;
    }

  // Check if this looks like UPS
  if (QRegExp(UPS_REGEXP).exactMatch(utext))
    {
    if (gcs == LatLon_Nad83)
      {
      die(QStringLiteral("UPS coordinates not supported with NAD '83."));
      }

    QString zone;
    vgGeocodedCoordinate result = parseUcs(text, zone);
    // \TODO set coordinate GCS and return result; might have to adjust
    //       depending on zone
    Q_UNUSED(result);
    die(QStringLiteral("UPS coordinates not supported at this time"));
    }

  // Try to parse as lat/long, which we do by taking advantage of qtKstParser's
  // built-in support for arc-length number forms; must have exactly two parts!
  const QStringList llParts =
    text.toLower().split(QRegExp("[,\\s]"), QString::SkipEmptyParts);
  if (llParts.count() == 2)
    {
    vgGeocodedCoordinate result;
    double& lat = result.Latitude, &lon = result.Longitude;

    // Check if coordinates are named in reverse
    const bool switched =
      llParts[0].endsWith('e') || llParts[0].endsWith('w');

    // Attempt to extract coordinates
    if (parseLlPart(llParts[0], switched ? lon : lat) &&
        parseLlPart(llParts[1], switched ? lat : lon))
      {
      // Success
      result.GCS = gcs;
      return result;
      }
    }

  die(QStringLiteral("Coordinate syntax not recognized."));
}

//-----------------------------------------------------------------------------
vgGeocodedPoly vgGeodesy::regionFromMgrs(const QString& mgrs)
{
  return regionFromMgrs(qPrintable(mgrs));
}

//-----------------------------------------------------------------------------
QString vgGeodesy::coordString(
  double latitude, double longitude, CoordFormatMode fm)
{
  resolveFormatMode(fm);

  if (fm == FormatDecimal)
    {
    return QString().sprintf("%+.6f, %+.6f", latitude, longitude);
    }
  else if (fm == FormatMgrs)
    {
    const vgGeocodedCoordinate c(latitude, longitude, LatLon_Wgs84);
    return qtString(vgGeodesy::mgrs(c));
    }
  else
    {
    return dmsString(latitude,  "N", "S", fm) + ", " +
           dmsString(longitude, "E", "W", fm);
    }
}

//-----------------------------------------------------------------------------
QString vgGeodesy::coordString(
  const vgGeocodedCoordinate& coord, bool* isValid, CoordFormatMode fm)
{
  resolveFormatMode(fm);

  bool dummy;
  bool& valid = (isValid ? *isValid : dummy);
  valid = true;

  if (fm == FormatMgrs)
    {
    // MGRS conversion handles GCS conversion
    return qtString(mgrs(coord));
    }

  // TODO handle UTM coordinates
  switch (coord.GCS)
    {
    case LatLon_Wgs84:
      return coordString(coord.Latitude, coord.Longitude, fm) + " (WGS '84)";
    case LatLon_Nad83:
      return coordString(coord.Latitude, coord.Longitude, fm) + " (NAD '83)";
    case -1:
      valid = false;
      return QStringLiteral("(empty or invalid)");
    default:
      // TODO convert to lat/lon?
      valid = false;
      return QStringLiteral("(Unable to convert from GCS %1)").arg(coord.GCS);
    }
}

//-----------------------------------------------------------------------------
QString vgGeodesy::coordString(
  const vgGeocodedCoordinate& coord, vgGeodesy::CoordFormatMode fm)
{
  return coordString(coord, 0, fm);
}
