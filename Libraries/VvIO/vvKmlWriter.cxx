/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvKmlWriter.h"

// #include <QColor>
#include <QDebug>
#include <QFile>
// #include <QTextStream>

#include <qtStlUtil.h>

// KML includes.
#include <kml/dom.h>

#include "vvKmlLine.h"

using kmldom::DocumentPtr;
using kmldom::KmlPtr;
using kmldom::KmlFactory;
using kmldom::PlacemarkPtr;
using kmldom::StylePtr;

//-----------------------------------------------------------------------------
vvKmlWriter::vvKmlWriter()
{
  // Do nothing
}

//-----------------------------------------------------------------------------
vvKmlWriter::~vvKmlWriter()
{
  foreach (vvKmlLine* line, Lines)
    {
    delete line;
    }

  this->Lines.clear();
}

//-----------------------------------------------------------------------------
bool vvKmlWriter::isEmpty() const
{
  // As of now we only support lines
  return this->Lines.isEmpty();
}

//-----------------------------------------------------------------------------
vvKmlLine* vvKmlWriter::createLine()
{
  vvKmlLine* kmlLine = new vvKmlLine();

  this->Lines.push_back(kmlLine);

  return kmlLine;
}

//-----------------------------------------------------------------------------
int vvKmlWriter::write(QFile& file)
{
  // First create the factory.
  KmlFactory* factory = KmlFactory::GetFactory();

  // Create KML.
  KmlPtr kml = factory->CreateKml();

  // Create document.
  DocumentPtr doc = factory->CreateDocument();

  // Now set document as feature.
  kml->set_feature(doc);

  foreach (vvKmlLine* line, Lines)
    {
    PlacemarkPtr linePlacemark = factory->CreatePlacemark();
    linePlacemark->set_name(stdString(line->getId()));
    linePlacemark->set_geometry(line->getLineString());
    linePlacemark->set_description(stdString(line->getDescription()));

    if (line->getLineStyle())
      {
      StylePtr style = factory->CreateStyle();
      QString styleId = QString("LineStyle%1").arg(line->getId());
      style->set_id(stdString(styleId));
      style->set_linestyle(line->getLineStyle());
      doc->add_styleselector(style);
      QString styleUrl = QString("#").append(styleId);
      linePlacemark->set_styleurl(stdString(styleUrl));
      }

    doc->add_feature(linePlacemark);
    }

  QTextStream out(&file);

  if (out.status() == QTextStream::Ok)
    {
    out << qtString(kmldom::SerializePretty(kml));
    return 0;
    }
  else
    {
    qDebug() << "Invalid text stream. Failed to write write KML";
    return 1;
    }
}
