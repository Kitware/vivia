/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvReader_h
#define __vvReader_h

#include <QList>
#include <QScopedPointer>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vvQuery.h>

class QString;
class QUrl;

struct vvQueryResult;

struct vvEventSetInfo;
struct vvHeader;
class vvQueryInstance;

class vvReaderPrivate;

class VV_IO_EXPORT vvReader
{
public:
  enum Format
    {
    Auto,
    Kst,
    Xml
    };

  vvReader();
  virtual ~vvReader();

  virtual bool open(const QUrl&, Format format = Auto);
  virtual bool setInput(const QString&, Format format = Auto);

  virtual void setHeader(const vvHeader&);

  virtual bool rewind();
  virtual bool advance();
  virtual bool atEnd() const;

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

#define vvReader_ReadArray(_name, _type) \
  virtual bool read##_name(std::vector<_type>& list); \
  virtual bool read##_name(QList<_type>& list)

  vvReader_ReadArray(Tracks, vvTrack);
  vvReader_ReadArray(Descriptors, vvDescriptor);
  vvReader_ReadArray(QueryResults, vvQueryResult);

#undef vvReader_ReadArray

  virtual QString error();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvReader)

  virtual bool open(const QUrl&, QString& error);

private:
  QTE_DECLARE_PRIVATE(vvReader)
  Q_DISABLE_COPY(vvReader)
};

#endif
