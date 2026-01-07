// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvXmlReader.h"

#include <QDomElement>
#include <QDebug>

#include <vvQueryResult.h>

#include "vvHeader.h"
#include "vvXmlUtil.h"

#define die(_msg) return this->abort(_msg)

#define test_or_fail(_cond) if (!(_cond)) return false
#define test_or_die(_cond, _msg) if (!(_cond)) die(_msg)

#define BEGIN_MAP_FROM_STRING(_type, _str, _val) \
  do { const QString _mfs_in = _str; _type &_mfs_out = _val; if (0)
#define MAP_FROM_STRING(_str, _val) \
  else if (_mfs_in == _str) _mfs_out = _val
#define END_MAP_FROM_STRING(_expr) \
  else { _expr; } } while (0)

QTE_IMPLEMENT_D_FUNC(vvXmlReader)

//BEGIN vvXmlReaderPrivate

//-----------------------------------------------------------------------------
class vvXmlReaderPrivate
{
public:
  QDomDocument document;
  QDomNode currentNode;

  QString lastError;

  bool abort(const QString&);

  template <typename T>
  bool read(T& result, vvXmlReader& q,
            bool (vvXmlReader::*method)(QDomNode&, T&));
  template <typename L, typename T>
  bool read(L& list, QDomNode& node, vvXmlReader& q,
            bool (vvXmlReader::*method)(QDomNode&, T&));
  template <typename T>
  bool read(QDomNode& node, T& out, const char* type);

  bool firstElement(QDomElement& elem, QDomNode node, const char* type);
};

//-----------------------------------------------------------------------------
bool vvXmlReaderPrivate::abort(const QString& error)
{
  qDebug() << "vvXmlReader:" << qPrintable(error);
  this->lastError = error;
  return false;
}

//-----------------------------------------------------------------------------
template <typename T>
bool vvXmlReaderPrivate::read(
  T& result, vvXmlReader& q, bool (vvXmlReader::*method)(QDomNode&, T&))
{
  test_or_die(!this->document.isNull(), "No data is available");
  return (q.*method)(this->currentNode, result);
}

//-----------------------------------------------------------------------------
template <typename L, typename T>
bool vvXmlReaderPrivate::read(
  L& list, QDomNode& node, vvXmlReader& q,
  bool (vvXmlReader::*method)(QDomNode&, T&))
{
  test_or_die(!this->document.isNull(), "No data is available");
  test_or_die(!node.isNull(), "Read position is at end of data");

  list.clear();
  while (!node.isNull())
    {
    // Read item
    T item;
    test_or_fail((q.*method)(node, item));
    list.push_back(item);
    }

  return true;
}

//-----------------------------------------------------------------------------
template <typename T>
bool vvXmlReaderPrivate::read(QDomNode& node, T& out, const char* type)
{
  // Read data
  QDomElement elem;
  test_or_fail(this->firstElement(elem, node, type));
  test_or_fail(vvXmlUtil::readNode(elem, out, &this->lastError));

  // Advance read position to next node
  node = elem.nextSibling();

  // Done
  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlReaderPrivate::firstElement(
  QDomElement& elem, QDomNode node, const char* type)
{
  test_or_die(!node.isNull(),
              "No data available or read position at end of data");
  if (!node.isElement())
    {
    node = node.nextSiblingElement();
    test_or_die(!node.isNull(),
                QString("End of data while looking for %1").arg(type));
    }

  elem = node.toElement();
  return true;
}

//END vvXmlReaderPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvXmlReader general

//-----------------------------------------------------------------------------
vvXmlReader::vvXmlReader() : d_ptr(new vvXmlReaderPrivate)
{
}

//-----------------------------------------------------------------------------
vvXmlReader::~vvXmlReader()
{
}

//-----------------------------------------------------------------------------
bool vvXmlReader::abort(const QString& error)
{
  QTE_D(vvXmlReader);
  return d->abort(error);
}

//-----------------------------------------------------------------------------
QString vvXmlReader::error()
{
  QTE_D(vvXmlReader);
  return d->lastError;
}

//-----------------------------------------------------------------------------
bool vvXmlReader::open(const QUrl& uri, vvReader::Format format)
{
  test_or_die(format == Auto || format == Xml,
              "Requested format not supported");

  QString error;
  test_or_die(this->open(uri, error), error);
  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlReader::setInput(const QString& data, vvReader::Format format)
{
  test_or_die(format == Auto || format == Xml,
              "Requested format not supported");

  QTE_D(vvXmlReader);

  d->document = QDomDocument();
  d->currentNode = QDomNode();

  QString em;
  int el, ec;

  if (!d->document.setContent(data, &em, &el, &ec))
    {
    d->document = QDomDocument();
    const QString format("Unable to parse XML: at %1:%2: %3");
    die(format.arg(el).arg(ec).arg(em));
    }

  return this->rewind();
}

//-----------------------------------------------------------------------------
void vvXmlReader::setHeader(const vvHeader&)
{
  // XML reader doesn't care about headers
}

//-----------------------------------------------------------------------------
bool vvXmlReader::rewind()
{
  QTE_D(vvXmlReader);

  QDomElement root = d->document.firstChildElement();
  if (root.isNull() || root.tagName() != "xml")
    {
    d->document = QDomDocument();
    die("Invalid or missing XML root node");
    }

  d->currentNode = root.firstChildElement();
  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlReader::advance()
{
  test_or_die(!this->atEnd(), "Already at EOF");

  QTE_D(vvXmlReader);

  QDomElement next;
  while (!d->currentNode.isNull())
    {
    // See if we have a next sibling
    next = d->currentNode.nextSiblingElement();
    if (!next.isNull())
      {
      // If yes, move to that node
      d->currentNode = next;
      return true;
      }

    // If not, move up to parent and keep looking
    d->currentNode = d->currentNode.parentNode();
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlReader::atEnd() const
{
  QTE_D_CONST(vvXmlReader);
  return d->currentNode.isNull();
}

//END vvKstReader general

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvKstReader struct readers

//-----------------------------------------------------------------------------
bool vvXmlReader::readHeader(QDomNode& node, vvHeader& header)
{
  QTE_D(vvXmlReader);

  QDomElement elem;
  test_or_fail(d->firstElement(elem, node, "header"));

  const QString type = elem.tagName();

  BEGIN_MAP_FROM_STRING(vvHeader::FileType, type, header.type);
  MAP_FROM_STRING("track",          vvHeader::Tracks);
  MAP_FROM_STRING("descriptor",     vvHeader::Descriptors);
  MAP_FROM_STRING("query",          vvHeader::QueryPlan);
  MAP_FROM_STRING("query_result",   vvHeader::QueryResults);
  MAP_FROM_STRING("event_meta",     vvHeader::EventSetInfo);
  END_MAP_FROM_STRING(die("Unrecognized file format " + type));

  header.version = 0;
  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlReader::readQueryPlan(QDomNode& node, vvRetrievalQuery& query)
{
  // Read generic query
  vvQueryInstance qi;
  test_or_fail(this->readQueryPlan(node, qi));

  // Verify query type
  test_or_die(qi.isRetrievalQuery(), "Node is not a retrieval query");

  // Done
  query = *qi.constRetrievalQuery();
  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlReader::readQueryPlan(QDomNode& node, vvSimilarityQuery& query)
{
  // Read generic query
  vvQueryInstance qi;
  test_or_fail(this->readQueryPlan(node, qi));

  // Verify query type
  test_or_die(qi.isSimilarityQuery(), "Node is not a similarity query");

  // Done
  query = *qi.constSimilarityQuery();
  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlReader::readGeoPoly(QDomNode& node, vgGeocodedPoly& poly,
                              vvDatabaseQuery::IntersectionType* filterMode)
{
  QTE_D(vvXmlReader);

  // Read data
  QDomElement elem;
  test_or_fail(d->firstElement(elem, node, "geo_poly"));
  test_or_fail(vvXmlUtil::readNode(elem, poly, filterMode, &d->lastError));

  // Advance read position to next node
  node = elem.nextSibling();

  // Done
  return true;
}

//-----------------------------------------------------------------------------
#define vvXmlReader_Implement_Read(_name, _type, _tag) \
  bool vvXmlReader::read##_name(QDomNode& node, _type& data) \
  { \
    QTE_D(vvXmlReader); \
    return d->read(node, data, _tag); \
  }

vvXmlReader_Implement_Read(Track,         vvTrack,          "track")
vvXmlReader_Implement_Read(Descriptor,    vvDescriptor,     "descriptor")
vvXmlReader_Implement_Read(QueryPlan,     vvQueryInstance,  "query")
vvXmlReader_Implement_Read(QueryResult,   vvQueryResult,    "query_result")
vvXmlReader_Implement_Read(EventSetInfo,  vvEventSetInfo,   "event_meta")

//END vvXmlReader struct readers

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvXmlReader bound data readers
#define vvXmlReader_Implement_BoundRead(_name, _type) \
  bool vvXmlReader::read##_name(_type& data) \
  { \
    QTE_D(vvXmlReader); \
    bool (vvXmlReader::*method)(QDomNode&, _type&) = \
      &vvXmlReader::read##_name; \
    return d->read(data, *this, method); \
  }

vvXmlReader_Implement_BoundRead(Header,       vvHeader)
vvXmlReader_Implement_BoundRead(Track,        vvTrack)
vvXmlReader_Implement_BoundRead(Descriptor,   vvDescriptor)
vvXmlReader_Implement_BoundRead(QueryPlan,    vvQueryInstance)
vvXmlReader_Implement_BoundRead(QueryPlan,    vvRetrievalQuery)
vvXmlReader_Implement_BoundRead(QueryPlan,    vvSimilarityQuery)
vvXmlReader_Implement_BoundRead(QueryResult,  vvQueryResult)
vvXmlReader_Implement_BoundRead(EventSetInfo, vvEventSetInfo)

//-----------------------------------------------------------------------------
bool vvXmlReader::readGeoPoly(
  vgGeocodedPoly& poly, vvDatabaseQuery::IntersectionType* filterMode)
{
  QTE_D(vvXmlReader);

  test_or_die(!d->document.isNull(), "No data is available");
  return this->readGeoPoly(d->currentNode, poly, filterMode);
}

//END vvKstReader bound data readers

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvKstReader array helpers

#define vvXmlReader_Implement_ReadTypedArray(_name, _type, _list) \
  bool vvXmlReader::read##_name##s(QDomNode& node, _list<_type>& list) \
  { \
    QTE_D(vvXmlReader); \
    bool (vvXmlReader::*method)(QDomNode&, _type&) = \
      &vvXmlReader::read##_name; \
    return d->read(list, node, *this, method); \
  }

#define vvXmlReader_Implement_ReadArray(_name, _type) \
  vvXmlReader_Implement_BoundRead(_name##s, QList<_type>) \
  vvXmlReader_Implement_BoundRead(_name##s, std::vector<_type>) \
  vvXmlReader_Implement_ReadTypedArray(_name, _type, std::vector) \
  vvXmlReader_Implement_ReadTypedArray(_name, _type, QList)

vvXmlReader_Implement_ReadArray(Track, vvTrack)
vvXmlReader_Implement_ReadArray(Descriptor, vvDescriptor)
vvXmlReader_Implement_ReadArray(QueryResult, vvQueryResult)

//END vvXmlReader array helpers
