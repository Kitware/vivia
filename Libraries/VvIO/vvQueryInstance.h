/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvQueryInstance_h
#define __vvQueryInstance_h

#include <QSharedDataPointer>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vvQuery.h>

class vvQueryInstanceData;

class VV_IO_EXPORT vvQueryInstance
{
public:
  vvQueryInstance();
  virtual ~vvQueryInstance();

  vvQueryInstance(const vvQueryInstance&);
  vvQueryInstance& operator=(const vvQueryInstance&);

  explicit vvQueryInstance(vvDatabaseQuery::QueryType);

  bool isValid() const;
  void clear();

  vvDatabaseQuery* abstractQuery();
  const vvDatabaseQuery* constAbstractQuery() const;

  bool isRetrievalQuery() const;
  bool isSimilarityQuery() const;

  vvRetrievalQuery* retrievalQuery();
  const vvRetrievalQuery* constRetrievalQuery() const;
  vvSimilarityQuery* similarityQuery();
  const vvSimilarityQuery* constSimilarityQuery() const;

  vvQueryInstance(const vvDatabaseQuery&);
  vvQueryInstance(const vvRetrievalQuery&);
  vvQueryInstance(const vvSimilarityQuery&);
  vvQueryInstance& operator=(const vvDatabaseQuery&);
  vvQueryInstance& operator=(const vvRetrievalQuery&);
  vvQueryInstance& operator=(const vvSimilarityQuery&);

protected:
  QTE_DECLARE_SHARED_PTR(vvQueryInstance)

private:
  QTE_DECLARE_SHARED(vvQueryInstance)
};

#endif
