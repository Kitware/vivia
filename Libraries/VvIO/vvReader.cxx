/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvReader.h"

#include <QFile>
#include <QRegExp>
#include <QUrl>

#include <vvQueryResult.h>

#include "vvHeader.h"
#include "vvKstReader.h"
#include "vvQueryInstance.h"
#include "vvXmlReader.h"

#define test_and_pass(_cond, _expr) if (_cond) return true

QTE_IMPLEMENT_D_FUNC(vvReader)

//-----------------------------------------------------------------------------
class vvReaderPrivate
{
public:
  template <typename T>
  bool read(T& result, bool (vvReader::*method)(T&));

  QScopedPointer<vvReader> reader;
  QString lastError;
};

//-----------------------------------------------------------------------------
template <typename T>
bool vvReaderPrivate::read(T& result, bool (vvReader::*method)(T&))
{
  if (!this->reader)
    {
    this->lastError = "No data is available";
    return false;
    }
  return (*this->reader.*method)(result);
}

//-----------------------------------------------------------------------------
vvReader::vvReader() : d_ptr(new vvReaderPrivate)
{
}

//-----------------------------------------------------------------------------
vvReader::~vvReader()
{
}

//-----------------------------------------------------------------------------
QString vvReader::error()
{
  QTE_D(vvReader);
  return (d->reader ? d->reader->error() : d->lastError);
}

//-----------------------------------------------------------------------------
bool vvReader::open(const QUrl& uri, vvReader::Format format)
{
  QTE_D(vvReader);

  if (format == Kst)
    {
    d->reader.reset(new vvKstReader);
    return d->reader->open(uri);
    }
  else if (format == Xml)
    {
    d->reader.reset(new vvXmlReader);
    return d->reader->open(uri);
    }
  else
    {
    // No matter what, calling open() resets the reader
    d->reader.reset();
    return this->open(uri, d->lastError);
    }
}

//-----------------------------------------------------------------------------
bool vvReader::open(const QUrl& uri, QString& error)
{
  // First, try to open the file
  QFile file(uri.toLocalFile());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    error = "Unable to open \"" + uri.toString() + "\": "
            + file.errorString();
    return false;
    }

  // Read the data, and hand off to setInput
  const QString data = QString::fromLocal8Bit(file.readAll());
  return this->setInput(data);
}

//-----------------------------------------------------------------------------
bool vvReader::setInput(const QString& data, vvReader::Format format)
{
  QTE_D(vvReader);

  if (format == Kst)
    {
    d->reader.reset(new vvKstReader);
    return d->reader->setInput(data);
    }
  else if (format == Xml)
    {
    d->reader.reset(new vvXmlReader);
    return d->reader->setInput(data);
    }
  else
    {
    // Guess format from data
    format = (data.contains(QRegExp("^\\s*<")) ? Xml : Kst);

    // Attempt to load as guessed format
    if (!this->setInput(data, format))
      {
      d->reader.reset();
      d->lastError = "No handler was able to process the input";
      return false;
      }

    return true;
    }
}

//-----------------------------------------------------------------------------
void vvReader::setHeader(const vvHeader& header)
{
  QTE_D(vvReader);
  if (d->reader)
    d->reader->setHeader(header);
}

//-----------------------------------------------------------------------------
bool vvReader::rewind()
{
  QTE_D(vvReader);
  return (d->reader && d->reader->rewind());
}

//-----------------------------------------------------------------------------
bool vvReader::advance()
{
  QTE_D(vvReader);
  return (d->reader && d->reader->advance());
}

//-----------------------------------------------------------------------------
bool vvReader::atEnd() const
{
  QTE_D_CONST(vvReader);
  return (!d->reader || d->reader->atEnd());
}


//-----------------------------------------------------------------------------
#define vvReader_Implement_Read(_name, _type) \
  bool vvReader::read##_name(_type& data) \
  { \
    QTE_D(vvReader); \
    return d->read(data, &vvReader::read##_name); \
  }

vvReader_Implement_Read(Header,         vvHeader)
vvReader_Implement_Read(Track,          vvTrack)
vvReader_Implement_Read(Descriptor,     vvDescriptor)
vvReader_Implement_Read(QueryPlan,      vvQueryInstance)
vvReader_Implement_Read(QueryPlan,      vvRetrievalQuery)
vvReader_Implement_Read(QueryPlan,      vvSimilarityQuery)
vvReader_Implement_Read(QueryResult,    vvQueryResult)
vvReader_Implement_Read(EventSetInfo,   vvEventSetInfo)

vvReader_Implement_Read(Tracks,               QList<vvTrack>)
vvReader_Implement_Read(Tracks,         std::vector<vvTrack>)
vvReader_Implement_Read(Descriptors,          QList<vvDescriptor>)
vvReader_Implement_Read(Descriptors,    std::vector<vvDescriptor>)
vvReader_Implement_Read(QueryResults,         QList<vvQueryResult>)
vvReader_Implement_Read(QueryResults,   std::vector<vvQueryResult>)

//-----------------------------------------------------------------------------
bool vvReader::readGeoPoly(
  vgGeocodedPoly& poly, vvDatabaseQuery::IntersectionType* filterMode)
{
  QTE_D(vvReader);

  if (!d->reader)
    {
    d->lastError = "No data is available";
    return false;
    }
  return d->reader->readGeoPoly(poly, filterMode);
}
