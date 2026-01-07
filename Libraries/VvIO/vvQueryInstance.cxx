// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvQueryInstance.h"

QTE_IMPLEMENT_D_FUNC_SHARED(vvQueryInstance)

//-----------------------------------------------------------------------------
class vvQueryInstanceData : public QSharedData
{
public:
  virtual ~vvQueryInstanceData() {}

  virtual vvQueryInstanceData* clone() { return new vvQueryInstanceData; }

  virtual bool isRetrievalQuery() const { return false; }
  virtual bool isSimilarityQuery() const { return false; }

  virtual vvDatabaseQuery* abstractQuery() { return 0; }
  virtual const vvDatabaseQuery* abstractQuery() const { return 0; }
  virtual vvRetrievalQuery* retrievalQuery() { return 0; }
  virtual const vvRetrievalQuery* retrievalQuery() const { return 0; }
  virtual vvSimilarityQuery* similarityQuery() { return 0; }
  virtual const vvSimilarityQuery* similarityQuery() const { return 0; }

protected:
  vvQueryInstanceData() {}

private:
  Q_DISABLE_COPY(vvQueryInstanceData)
};

//-----------------------------------------------------------------------------
class vvDatabaseQueryInstanceData : public vvQueryInstanceData,
                                    public vvDatabaseQuery
{
public:
  vvDatabaseQueryInstanceData() {}
  vvDatabaseQueryInstanceData(const vvDatabaseQuery& other)
    : vvDatabaseQuery(other) {}

  virtual ~vvDatabaseQueryInstanceData() {}

  virtual vvQueryInstanceData* clone()
    { return new vvDatabaseQueryInstanceData(*this->abstractQuery()); }

  virtual vvDatabaseQuery* abstractQuery() { return this; }
  virtual const vvDatabaseQuery* abstractQuery() const { return this; }
};

//-----------------------------------------------------------------------------
class vvRetrievalQueryInstanceData : public vvQueryInstanceData,
                                     public vvRetrievalQuery
{
public:
  vvRetrievalQueryInstanceData() {}
  vvRetrievalQueryInstanceData(const vvRetrievalQuery& other)
    : vvRetrievalQuery(other) {}

  virtual ~vvRetrievalQueryInstanceData() {}

  virtual vvQueryInstanceData* clone()
    { return new vvRetrievalQueryInstanceData(*this->retrievalQuery()); }

  virtual bool isRetrievalQuery() const { return true; }

  virtual vvDatabaseQuery* abstractQuery() { return this; }
  virtual const vvDatabaseQuery* abstractQuery() const { return this; }
  virtual vvRetrievalQuery* retrievalQuery() { return this; }
  virtual const vvRetrievalQuery* retrievalQuery() const { return this; }
};

//-----------------------------------------------------------------------------
class vvSimilarityQueryInstanceData : public vvQueryInstanceData,
                                      public vvSimilarityQuery
{
public:
  vvSimilarityQueryInstanceData() {}
  vvSimilarityQueryInstanceData(const vvSimilarityQuery& other)
    : vvSimilarityQuery(other) {}

  virtual ~vvSimilarityQueryInstanceData() {}

  virtual vvQueryInstanceData* clone()
    { return new vvSimilarityQueryInstanceData(*this->similarityQuery()); }

  virtual bool isSimilarityQuery() const { return true; }

  virtual vvDatabaseQuery* abstractQuery() { return this; }
  virtual const vvDatabaseQuery* abstractQuery() const { return this; }
  virtual vvSimilarityQuery* similarityQuery() { return this; }
  virtual const vvSimilarityQuery* similarityQuery() const { return this; }
};

//-----------------------------------------------------------------------------
template <>
vvQueryInstanceData* QSharedDataPointer<vvQueryInstanceData>::clone()
{
  return d->clone();
}

//-----------------------------------------------------------------------------
vvQueryInstance::vvQueryInstance()
{
}

//-----------------------------------------------------------------------------
vvQueryInstance::~vvQueryInstance()
{
}

//-----------------------------------------------------------------------------
vvQueryInstance::vvQueryInstance(const vvQueryInstance& other)
  : d_ptr(other.d_ptr)
{
}

//-----------------------------------------------------------------------------
vvQueryInstance& vvQueryInstance::operator=(const vvQueryInstance& other)
{
  this->d_ptr = other.d_ptr;
  return *this;
}

//-----------------------------------------------------------------------------
vvQueryInstance::vvQueryInstance(vvDatabaseQuery::QueryType type)
{
  switch (type)
    {
    case vvDatabaseQuery::Similarity:
      this->d_ptr = new vvSimilarityQueryInstanceData;
      break;
    case vvDatabaseQuery::Retrieval:
      this->d_ptr = new vvRetrievalQueryInstanceData;
      break;
    case vvDatabaseQuery::Abstract:
      this->d_ptr = new vvDatabaseQueryInstanceData;
      break;
    default:
      break;
    }
}

//-----------------------------------------------------------------------------
bool vvQueryInstance::isValid() const
{
  return this->d_ptr;
}

//-----------------------------------------------------------------------------
void vvQueryInstance::clear()
{
  this->d_ptr = 0;
}

//-----------------------------------------------------------------------------
vvDatabaseQuery* vvQueryInstance::abstractQuery()
{
  QTE_D_MUTABLE(vvQueryInstance);
  return (d ? d->abstractQuery() : 0);
}

//-----------------------------------------------------------------------------
const vvDatabaseQuery* vvQueryInstance::constAbstractQuery() const
{
  QTE_D_SHARED(vvQueryInstance);
  return (d ? d->abstractQuery() : 0);
}

//-----------------------------------------------------------------------------
bool vvQueryInstance::isRetrievalQuery() const
{
  QTE_D_SHARED(vvQueryInstance);
  return (d && d->isRetrievalQuery());
}

//-----------------------------------------------------------------------------
bool vvQueryInstance::isSimilarityQuery() const
{
  QTE_D_SHARED(vvQueryInstance);
  return (d && d->isSimilarityQuery());
}

//-----------------------------------------------------------------------------
vvRetrievalQuery* vvQueryInstance::retrievalQuery()
{
  QTE_D_MUTABLE(vvQueryInstance);
  return (d ? d->retrievalQuery() : 0);
}

//-----------------------------------------------------------------------------
const vvRetrievalQuery* vvQueryInstance::constRetrievalQuery() const
{
  QTE_D_SHARED(vvQueryInstance);
  return (d ? d->retrievalQuery() : 0);
}

//-----------------------------------------------------------------------------
vvSimilarityQuery* vvQueryInstance::similarityQuery()
{
  QTE_D_MUTABLE(vvQueryInstance);
  return (d ? d->similarityQuery() : 0);
}

//-----------------------------------------------------------------------------
const vvSimilarityQuery* vvQueryInstance::constSimilarityQuery() const
{
  QTE_D_SHARED(vvQueryInstance);
  return (d ? d->similarityQuery() : 0);
}

//-----------------------------------------------------------------------------
vvQueryInstance::vvQueryInstance(const vvDatabaseQuery& other)
  : d_ptr(new vvDatabaseQueryInstanceData(other))
{
}

//-----------------------------------------------------------------------------
vvQueryInstance::vvQueryInstance(const vvRetrievalQuery& other)
  : d_ptr(new vvRetrievalQueryInstanceData(other))
{
}

//-----------------------------------------------------------------------------
vvQueryInstance::vvQueryInstance(const vvSimilarityQuery& other)
  : d_ptr(new vvSimilarityQueryInstanceData(other))
{
}

//-----------------------------------------------------------------------------
vvQueryInstance& vvQueryInstance::operator=(const vvDatabaseQuery& other)
{
  this->d_ptr = new vvDatabaseQueryInstanceData(other);
  return *this;
}

//-----------------------------------------------------------------------------
vvQueryInstance& vvQueryInstance::operator=(const vvRetrievalQuery& other)
{
  this->d_ptr = new vvRetrievalQueryInstanceData(other);
  return *this;
}

//-----------------------------------------------------------------------------
vvQueryInstance& vvQueryInstance::operator=(const vvSimilarityQuery& other)
{
  this->d_ptr = new vvSimilarityQueryInstanceData(other);
  return *this;
}
