// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvKmlLine_h
#define __vvKmlLine_h

#include <qtUtil.h>

#include <QString>

// KML includes.
#include <kml/dom.h>

#include <vgExport.h>

//-----------------------------------------------------------------------------
class VV_IO_EXPORT vvKmlLine
{
public:
  vvKmlLine();
  ~vvKmlLine();

  void setId(const QString& id);
  const QString& getId() const;

  kmldom::LineStringPtr getLineString() const;
  kmldom::LineStylePtr getLineStyle() const;

  void setDescription(const QString& desc);
  const QString& getDescription() const;

  void addPoint(double lat, double lon, double alt);
  void setColor(double r, double g, double b, double a);
  void setWidth(double width);

protected:
  QString Id;
  QString Description;
  kmldom::LineStringPtr Line;
  kmldom::LineStylePtr LineStyle;
  kmldom::CoordinatesPtr Coordinates;
};

#endif
