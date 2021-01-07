// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
