/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvWriter.h"

#include <qtUtil.h>

#include "vvKstWriter.h"
#include "vvXmlWriter.h"

QTE_IMPLEMENT_D_FUNC(vvWriter)

//-----------------------------------------------------------------------------
class vvWriterPrivate
{
public:
  QScopedPointer<vvWriter> writer;

  vvWriterPrivate(QFile&, vvWriter::Format, bool pretty);
  vvWriterPrivate(QTextStream&, vvWriter::Format, bool pretty);
};

//-----------------------------------------------------------------------------
vvWriterPrivate::vvWriterPrivate(
  QFile& file, vvWriter::Format format, bool pretty)
{
  switch (format)
    {
    case vvWriter::Kst:
      this->writer.reset(new vvKstWriter(file, pretty));
      break;
    case vvWriter::Xml:
      this->writer.reset(new vvXmlWriter(file, pretty ? 2 : -1));
      break;
    default:
      break;
    }
}

//-----------------------------------------------------------------------------
vvWriterPrivate::vvWriterPrivate(
  QTextStream& stream, vvWriter::Format format, bool pretty)
{
  switch (format)
    {
    case vvWriter::Kst:
      this->writer.reset(new vvKstWriter(stream, pretty));
      break;
    case vvWriter::Xml:
      this->writer.reset(new vvXmlWriter(stream, pretty ? 2 : -1));
      break;
    default:
      break;
    }
}

//-----------------------------------------------------------------------------
vvWriter::vvWriter(QFile& file, Format format, bool pretty)
  : d_ptr(new vvWriterPrivate(file, format, pretty))
{
}

//-----------------------------------------------------------------------------
vvWriter::vvWriter(QTextStream& stream, Format format, bool pretty)
  : d_ptr(new vvWriterPrivate(stream, format, pretty))
{
}

//-----------------------------------------------------------------------------
vvWriter::vvWriter()
{
}

//-----------------------------------------------------------------------------
vvWriter::~vvWriter()
{
}

//-----------------------------------------------------------------------------
#define writer_sout_operator(_class) \
  vvWriter& vvWriter::operator<<(_class data) \
  { \
    QTE_D(vvWriter); \
    (*d->writer) << data; \
    return *this; \
  }
writer_sout_operator(vvHeader::FileType)
writer_sout_operator(const vvTrack&)
writer_sout_operator(const vvDescriptor&)
writer_sout_operator(const vvQueryInstance&)
writer_sout_operator(const vvRetrievalQuery&)
writer_sout_operator(const vvSimilarityQuery&)
writer_sout_operator(const vvQueryResult&)
writer_sout_operator(const vgGeocodedPoly&)
writer_sout_operator(const vvSpatialFilter&)
writer_sout_operator(const vvEventSetInfo&)
