// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvKmlLine.h"

#include <QColor>

// KML includes.
#include <kml/dom.h>

using kmldom::KmlFactory;
using kmldom::LineStringPtr;
using kmldom::LineStylePtr;

//-----------------------------------------------------------------------------
vvKmlLine::vvKmlLine()
{
  KmlFactory* factory = KmlFactory::GetFactory();
  this->Line = factory->CreateLineString();
  this->LineStyle = factory->CreateLineStyle();
  this->Coordinates = factory->CreateCoordinates();

  this->Line->set_coordinates(this->Coordinates);

  // Set default width to 2.0
  this->setWidth(2.0);
}

//-----------------------------------------------------------------------------
vvKmlLine::~vvKmlLine()
{
}

//-----------------------------------------------------------------------------
void vvKmlLine::setId(const QString& id)
{
  this->Id = id;
}

//-----------------------------------------------------------------------------
const QString& vvKmlLine::getId() const
{
  return this->Id;
}

//-----------------------------------------------------------------------------
LineStringPtr vvKmlLine::getLineString() const
{
  return this->Line;
}

//-----------------------------------------------------------------------------
LineStylePtr vvKmlLine::getLineStyle() const
{
  return this->LineStyle;
}

//-----------------------------------------------------------------------------
void vvKmlLine::setDescription(const QString& desc)
{
  this->Description = desc;
}

//-----------------------------------------------------------------------------
const QString& vvKmlLine::getDescription() const
{
  return this->Description;
}

//-----------------------------------------------------------------------------
void vvKmlLine::addPoint(double lat, double lon, double alt)
{
  this->Coordinates->add_latlngalt(lat, lon, alt);
}

//-----------------------------------------------------------------------------
void vvKmlLine::setColor(double r, double g, double b, double a)
{
  QColor color;
  color.setRgbF(r, g, b, a);
  QRgb argbColor = color.rgba();
  kmlbase::Color32 kmlColor;
  kmlColor.set_color_argb(argbColor);
  this->LineStyle->set_color(kmlColor);
}

//-----------------------------------------------------------------------------
void vvKmlLine::setWidth(double width)
{
  this->LineStyle->set_width(width);
}
