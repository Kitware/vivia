// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgGeoUtil_h
#define __vgGeoUtil_h

#include <vgExport.h>

class QString;

struct vgGeocodedCoordinate;
struct vgGeocodedPoly;

namespace vgGeodesy
{
  enum CoordFormatMode
    {
    FormatFromSettings = -1,
    FormatDecimal = 0,
    FormatDmsAscii,     // Use only ASCII characters for separators
    FormatDmsLatin1,    // Use Latin-1 degree symbol only
    FormatDmsUnicode,   // Use UNICODE separators, including (double) prime
    FormatDmsColon,     // Use colons for separators
    FormatMgrs
    // TODO should use QFlags to separate formatting of already-lat-lon versus
    //      forcing GCS conversion
    };

  QTVG_COMMON_EXPORT vgGeocodedCoordinate parseCoordinate(
    const QString&, int gcs, QString* error);

  QTVG_COMMON_EXPORT vgGeocodedPoly regionFromMgrs(const QString&);

  QTVG_COMMON_EXPORT QString coordString(
    double latitude, double longitude, CoordFormatMode = FormatFromSettings);

  QTVG_COMMON_EXPORT QString coordString(
    const vgGeocodedCoordinate&, bool* isValid = 0,
    CoordFormatMode = FormatFromSettings);

  QTVG_COMMON_EXPORT QString coordString(
    const vgGeocodedCoordinate&, CoordFormatMode);
}

#endif
