/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvKstWriter_h
#define __vvKstWriter_h

#include <vgExport.h>

#include "vvWriter.h"

class vvKstWriterPrivate;

class VV_IO_EXPORT vvKstWriter : public vvWriter
{
public:
  explicit vvKstWriter(QFile&, bool pretty = true);
  explicit vvKstWriter(QTextStream&, bool pretty = true);
  virtual ~vvKstWriter();

  static const unsigned int TracksVersion;
  static const unsigned int DescriptorsVersion;
  static const unsigned int QueryPlanVersion;
  static const unsigned int QueryResultsVersion;
  static const unsigned int EventSetInfoVersion;
  static const unsigned int GeoPolyVersion;

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
  QTE_DECLARE_PRIVATE_RPTR(vvKstWriter)

  enum Marker
    {
    NoOp,
    NewLine,
    BeginRecord,
    EndRecord,
    BeginArray,
    EndArray,
    EndArrayNewLine,
    BeginValue,
    BeginString,
    EndValue,
    Comment
    };

  virtual vvKstWriter& operator<<(Marker);
  virtual vvKstWriter& operator<<(const vvImageBoundingBox&);
  virtual vvKstWriter& operator<<(const vvTrackId&);
  virtual vvKstWriter& operator<<(const vvTrackState&);
  virtual vvKstWriter& operator<<(const vvDatabaseQuery&);
  virtual vvKstWriter& operator<<(vvDatabaseQuery::IntersectionType);

  virtual vvKstWriter& operator<<(const QColor&);
  virtual vvKstWriter& operator<<(const QChar&);
  virtual vvKstWriter& operator<<(const QString&);
  virtual vvKstWriter& operator<<(char);
  virtual vvKstWriter& operator<<(const char*);
  virtual vvKstWriter& operator<<(const std::string&);
  virtual vvKstWriter& operator<<(qint32);
  virtual vvKstWriter& operator<<(qint64);
  virtual vvKstWriter& operator<<(quint32);
  virtual vvKstWriter& operator<<(quint64);
  virtual vvKstWriter& operator<<(float);
  virtual vvKstWriter& operator<<(double);

private:
  QTE_DECLARE_PRIVATE(vvKstWriter)
  Q_DISABLE_COPY(vvKstWriter)
};

#endif
