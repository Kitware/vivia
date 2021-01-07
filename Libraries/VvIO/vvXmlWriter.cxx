// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvXmlWriter.h"

#include <QDomDocument>

#include <qtUtil.h>

#include "vvXmlUtil.h"

QTE_IMPLEMENT_D_FUNC(vvXmlWriter)

//-----------------------------------------------------------------------------
class vvXmlWriterPrivate
{
public:
  vvXmlWriterPrivate(QTextStream*, int indent, bool free);
  ~vvXmlWriterPrivate() { if (this->freeStream) delete this->stream; }

  QDomDocument document;
  QDomElement rootNode;

  QTextStream* stream;
  bool freeStream;

  int indent;
};

//-----------------------------------------------------------------------------
vvXmlWriterPrivate::vvXmlWriterPrivate(
  QTextStream* stream, int indent, bool free)
  : document("VVXML"), stream(stream), freeStream(free), indent(indent)
{
  // \TODO set document DTD?

  // Create root node
  this->rootNode = document.createElement("xml");
  this->document.appendChild(this->rootNode);
}

//-----------------------------------------------------------------------------
vvXmlWriter::vvXmlWriter(QFile& file, int indent)
  : d_ptr(new vvXmlWriterPrivate(new QTextStream(&file), indent, true))
{
}

//-----------------------------------------------------------------------------
vvXmlWriter::vvXmlWriter(QTextStream& stream, int indent)
  : d_ptr(new vvXmlWriterPrivate(&stream, indent, false))
{
}

//-----------------------------------------------------------------------------
vvXmlWriter::~vvXmlWriter()
{
  QTE_D(vvXmlWriter);

  // Serialize the document into the stream
  (*d->stream) << d->document.toString(d->indent);
}

//-----------------------------------------------------------------------------
QDomDocument vvXmlWriter::document() const
{
  QTE_D_CONST(vvXmlWriter);
  return d->document;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(vvHeader::FileType)
{
  // Header is not needed for XML
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vvTrack& track)
{
  QTE_D(vvXmlWriter);
  d->rootNode.appendChild(vvXmlUtil::makeNode(d->document, track));
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vvDescriptor& descriptor)
{
  QTE_D(vvXmlWriter);
  d->rootNode.appendChild(vvXmlUtil::makeNode(d->document, descriptor));
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vvQueryInstance& query)
{
  if (query.isSimilarityQuery())
    {
    return *this << *query.constSimilarityQuery();
    }
  else if (query.isRetrievalQuery())
    {
    return *this << *query.constRetrievalQuery();
    }
  else if (query.isValid())
    {
    QTE_D(vvXmlWriter);
    const vvDatabaseQuery& abstractQuery = *query.constAbstractQuery();
    QDomNode node = vvXmlUtil::makeNode(d->document, abstractQuery);
    d->rootNode.appendChild(node);
    }
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vvRetrievalQuery& query)
{
  QTE_D(vvXmlWriter);
  d->rootNode.appendChild(vvXmlUtil::makeNode(d->document, query));
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vvSimilarityQuery& query)
{
  QTE_D(vvXmlWriter);
  d->rootNode.appendChild(vvXmlUtil::makeNode(d->document, query));
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vvQueryResult& result)
{
  QTE_D(vvXmlWriter);
  d->rootNode.appendChild(vvXmlUtil::makeNode(d->document, result));
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vgGeocodedPoly& poly)
{
  QTE_D(vvXmlWriter);
  d->rootNode.appendChild(vvXmlUtil::makeNode(d->document, poly));
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vvSpatialFilter& filter)
{
  QTE_D(vvXmlWriter);
  d->rootNode.appendChild(vvXmlUtil::makeNode(d->document, filter));
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvXmlWriter::operator<<(const vvEventSetInfo& info)
{
  QTE_D(vvXmlWriter);
  d->rootNode.appendChild(vvXmlUtil::makeNode(d->document, info));
  return *this;
}
