/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvXmlWriter_h
#define __vvXmlWriter_h

#include <vgExport.h>

#include "vvWriter.h"

class QDomDocument;

class vvXmlWriterPrivate;

class VV_IO_EXPORT vvXmlWriter : public vvWriter
{
public:
  explicit vvXmlWriter(QFile&, int indent = 2);
  explicit vvXmlWriter(QTextStream&, int indent = 2);
  virtual ~vvXmlWriter();

  QDomDocument document() const;

  virtual vvWriter& operator<<(vvHeader::FileType);

  virtual vvWriter& operator<<(const vvTrack&);
  virtual vvWriter& operator<<(const vvDescriptor&);
  virtual vvWriter& operator<<(const vvQueryInstance&);
  virtual vvWriter& operator<<(const vvRetrievalQuery&);
  virtual vvWriter& operator<<(const vvSimilarityQuery&);
  virtual vvWriter& operator<<(const vvQueryResult&);
  virtual vvWriter& operator<<(const vgGeocodedPoly&);
  virtual vvWriter& operator<<(const vvSpatialFilter&);
  virtual vvWriter& operator<<(const vvEventSetInfo&);

  using vvWriter::operator<<; // Copy template overload

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvXmlWriter)

private:
  QTE_DECLARE_PRIVATE(vvXmlWriter)
  Q_DISABLE_COPY(vvXmlWriter)
};

#endif
