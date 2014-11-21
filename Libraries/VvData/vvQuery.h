/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvQuery_h
#define __vvQuery_h

#include <vgGeoTypes.h>

#include "vvDescriptor.h"

//-----------------------------------------------------------------------------
struct vvDatabaseQuery
{
  enum QueryType
    {
    Abstract = 0,
    Retrieval,
    Similarity
    };

  enum IntersectionType
    {
    Ignore,
    ContainsWholly,
    ContainsAny,
    Intersects, // partly but not wholly contained
    IntersectsInbound, // first does not contain, then contains
    IntersectsOutbound, // first contains, then does not contain
    DoesNotContain
    };

  vvDatabaseQuery()
    : Type(Abstract), SpatialFilter(ContainsAny),
      TemporalLowerLimit(-1), TemporalUpperLimit(-1),
      TemporalFilter(ContainsAny) {}

  const QueryType Type;
  std::string QueryId;
  std::string StreamIdLimit;
  vgGeocodedPoly SpatialLimit;
  IntersectionType SpatialFilter;
  long long TemporalLowerLimit;
  long long TemporalUpperLimit;
  IntersectionType TemporalFilter;

protected:
  vvDatabaseQuery(QueryType t)
    : Type(t), SpatialFilter(ContainsAny),
      TemporalLowerLimit(-1), TemporalUpperLimit(-1),
      TemporalFilter(ContainsAny) {}

  vvDatabaseQuery& operator=(const vvDatabaseQuery& other)
    {
    this->QueryId = other.QueryId;
    this->StreamIdLimit = other.StreamIdLimit;
    this->SpatialLimit = other.SpatialLimit;
    this->SpatialFilter = other.SpatialFilter;
    this->TemporalLowerLimit = other.TemporalLowerLimit;
    this->TemporalUpperLimit = other.TemporalUpperLimit;
    this->TemporalFilter = other.TemporalFilter;
    return *this;
    }
};

//-----------------------------------------------------------------------------
struct vvRetrievalQuery : vvDatabaseQuery
{
  enum EntityType
    {
    Tracks,
    Descriptors,
    TracksAndDescriptors
    };

  vvRetrievalQuery(EntityType et = Descriptors)
    : vvDatabaseQuery(Retrieval), RequestedEntities(et) {}
  explicit vvRetrievalQuery(const vvDatabaseQuery& other)
    : vvDatabaseQuery(Retrieval), RequestedEntities(Descriptors)
    { this->vvDatabaseQuery::operator=(other); }

  EntityType RequestedEntities;
};

//-----------------------------------------------------------------------------
struct vvSimilarityQuery : vvDatabaseQuery
{
  vvSimilarityQuery()
    : vvDatabaseQuery(Similarity), SimilarityThreshold(0.0) {}
  explicit vvSimilarityQuery(const vvDatabaseQuery& other)
    : vvDatabaseQuery(Similarity), SimilarityThreshold(0.0)
    { this->vvDatabaseQuery::operator=(other); }

  std::vector<vvTrack> Tracks;
  std::vector<vvDescriptor> Descriptors;
  std::vector<unsigned char> IqrModel;
  double SimilarityThreshold;
};

#endif
