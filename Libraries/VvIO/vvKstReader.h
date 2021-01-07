// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvKstReader_h
#define __vvKstReader_h

#include <vgExport.h>

#include "vvReader.h"

class qtKstReader;

class vvKstReaderPrivate;

class VV_IO_EXPORT vvKstReader : public vvReader
{
public:
  vvKstReader();
  virtual ~vvKstReader();

  virtual QString error();

  // Data binding
  virtual bool open(const QUrl&, Format format = Kst);
  virtual bool setInput(const QString&, Format format = Kst);

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
  bool readHeader(qtKstReader& reader, vvHeader& header);

  bool readTrack(qtKstReader& reader,
                 vvTrack& track,
                 unsigned int version);

  bool readDescriptor(qtKstReader& reader,
                      vvDescriptor& descriptor,
                      unsigned int version);

  bool readQueryPlan(qtKstReader& reader,
                     vvQueryInstance& query,
                     unsigned int version);

  bool readQueryPlan(qtKstReader& reader,
                     vvRetrievalQuery& query,
                     unsigned int version);

  bool readQueryPlan(qtKstReader& reader,
                     vvSimilarityQuery& query,
                     unsigned int version);

  bool readQueryResult(qtKstReader& reader,
                       vvQueryResult& result,
                       unsigned int version);

  bool readGeoPoly(qtKstReader& reader,
                   vgGeocodedPoly& poly,
                   unsigned int version,
                   vvDatabaseQuery::IntersectionType* filterMode = 0);

  bool readEventSetInfo(qtKstReader& reader,
                        vvEventSetInfo& info,
                        unsigned int version);

  // Array helpers
#define vvKstReader_ReadArray(_name, _type) \
  virtual bool read##_name(std::vector<_type>& list); \
  virtual bool read##_name(QList<_type>& list); \
  bool read##_name(qtKstReader& reader, \
                   std::vector<_type>& list, \
                   unsigned int version); \
  bool read##_name(qtKstReader& reader, \
                   QList<_type>& list, \
                   unsigned int version)

  vvKstReader_ReadArray(Tracks, vvTrack);
  vvKstReader_ReadArray(Descriptors, vvDescriptor);
  vvKstReader_ReadArray(QueryResults, vvQueryResult);

#undef vvKstReader_ReadArray

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvKstReader)

  using vvReader::open;

  bool abort(const QString&);

private:
  QTE_DECLARE_PRIVATE(vvKstReader)
  Q_DISABLE_COPY(vvKstReader)
};

#endif
