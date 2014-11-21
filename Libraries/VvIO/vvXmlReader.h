/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvXmlReader_h
#define __vvXmlReader_h

#include <vgExport.h>

#include "vvReader.h"

class QDomNode;

class vvXmlReaderPrivate;

class VV_IO_EXPORT vvXmlReader : public vvReader
{
public:
  vvXmlReader();
  virtual ~vvXmlReader();

  virtual QString error();

  // Data binding
  virtual bool open(const QUrl&, Format format = Xml);
  virtual bool setInput(const QString&, Format format = Xml);

  virtual void setHeader(const vvHeader&);

  // Random access
  virtual bool rewind();
  virtual bool advance();
  virtual bool atEnd() const;

  // Bound data readers
  virtual bool readHeader(vvHeader& header);

  virtual bool readTrack(vvTrack& track);
  virtual bool readDescriptor(vvDescriptor& descriptor);

  virtual bool readQueryPlan(vvQueryInstance& query);
  virtual bool readQueryPlan(vvRetrievalQuery& query);
  virtual bool readQueryPlan(vvSimilarityQuery& query);

  virtual bool readQueryResult(vvQueryResult& result);

  virtual bool readGeoPoly(vgGeocodedPoly& poly,
                           vvDatabaseQuery::IntersectionType* filterMode = 0);

  virtual bool readEventSetInfo(vvEventSetInfo& info);

  // External data readers
  bool readHeader(QDomNode& node, vvHeader& header);

  bool readTrack(QDomNode& node, vvTrack& track);
  bool readDescriptor(QDomNode& node, vvDescriptor& descriptor);

  bool readQueryPlan(QDomNode& node, vvQueryInstance& query);
  bool readQueryPlan(QDomNode& node, vvRetrievalQuery& query);
  bool readQueryPlan(QDomNode& node, vvSimilarityQuery& query);

  bool readQueryResult(QDomNode& node, vvQueryResult& result);

  bool readGeoPoly(QDomNode& node, vgGeocodedPoly& poly,
                   vvDatabaseQuery::IntersectionType* filterMode = 0);

  bool readEventSetInfo(QDomNode& node, vvEventSetInfo& info);

  // Array helpers
#define vvXmlReader_ReadArray(_name, _type) \
  virtual bool read##_name(std::vector<_type>& list); \
  virtual bool read##_name(QList<_type>& list); \
  bool read##_name(QDomNode& node, std::vector<_type>& list); \
  bool read##_name(QDomNode& node, QList<_type>& list)

  vvXmlReader_ReadArray(Tracks, vvTrack);
  vvXmlReader_ReadArray(Descriptors, vvDescriptor);
  vvXmlReader_ReadArray(QueryResults, vvQueryResult);

#undef vvXmlReader_ReadArray

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvXmlReader)

  using vvReader::open;

  bool abort(const QString&);

private:
  QTE_DECLARE_PRIVATE(vvXmlReader)
  Q_DISABLE_COPY(vvXmlReader)
};

#endif
