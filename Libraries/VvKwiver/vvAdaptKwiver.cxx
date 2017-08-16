/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvAdaptKwiver.h"

#include <vvQueryFormulation.h>
#include <vvQueryInstance.h>

#include <vvDescriptor.h>
#include <vvQueryResult.h>

#include <qtStlUtil.h>

#include <vital/types/database_query.h>
#include <vital/types/descriptor_request.h>
#include <vital/types/iqr_feedback.h>
#include <vital/types/query_result.h>
#include <vital/types/track_descriptor.h>

#include <algorithm>

namespace // anonymous
{

//-----------------------------------------------------------------------------
template <typename T>
std::vector<T> flatten(std::vector<std::vector<T>> const& in)
{
  size_t size = 0;
  for (auto const& v : in)
  {
    size += v.size();
  }

  std::vector<T> out;
  out.reserve(size);
  for (auto const& v : in)
  {
    out.insert(out.end(), v.begin(), v.end());
  }

  return out;
}

//-----------------------------------------------------------------------------
vvImagePointF pointFromBox(vvImageBoundingBox const& box)
{
  auto const x = 0.5 * static_cast<double>(box.TopLeft.X + box.BottomRight.X);
  auto const y = static_cast<double>(box.BottomRight.Y);
  return {x, y};
}

//-----------------------------------------------------------------------------
kwiver::vital::timestamp utcTimestamp(long long usecs)
{
  auto result = kwiver::vital::timestamp{};

  if (usecs != -1)
  {
    result.set_time_usec(usecs);
  }

  return result;
}

//-----------------------------------------------------------------------------
vvImagePoint fromKwiver(Eigen::Matrix<double, 2, 1> const& in)
{
  return {static_cast<int>(in[0]), static_cast<int>(in[1])};
}

//-----------------------------------------------------------------------------
vgTimeStamp fromKwiver(kwiver::vital::timestamp const& in)
{
  vgTimeStamp out;

  if (in.has_valid_time())
  {
    out.Time = static_cast<double>(in.get_time_usec());
  }
  if (in.has_valid_frame())
  {
    out.FrameNumber = static_cast<unsigned>(in.get_frame());
  }

  return out;
}

//-----------------------------------------------------------------------------
kwiver::vital::timestamp toKwiver(vgTimeStamp const& in)
{
  kwiver::vital::timestamp out;

  if (in.HasTime())
  {
    out.set_time_usec(static_cast<int64_t>(in.Time));
  }
  if (in.HasFrameNumber())
  {
    out.set_frame(in.FrameNumber);
  }

  return out;
}

//-----------------------------------------------------------------------------
vvImageBoundingBox fromKwiver(kwiver::vital::bounding_box_d const& in)
{
  return {fromKwiver(in.upper_left()), fromKwiver(in.lower_right())};
}

//-----------------------------------------------------------------------------
vvDescriptorRegionEntry fromKwiver(
  kwiver::vital::track_descriptor::history_entry const& in)
{
  vvDescriptorRegionEntry out;
  out.TimeStamp = fromKwiver(in.get_timestamp());
  out.ImageRegion = fromKwiver(in.get_image_location());
  return out;
}

//-----------------------------------------------------------------------------
kwiver::vital::track_descriptor::history_entry toKwiver(
  vvDescriptorRegionEntry const& in)
{
  return {toKwiver(in.TimeStamp),
          {{in.ImageRegion.TopLeft.X, in.ImageRegion.TopLeft.Y},
           {in.ImageRegion.BottomRight.X, in.ImageRegion.BottomRight.Y}}};
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate fromKwiver(kwiver::vital::geo_point const& in)
{
  vgGeocodedCoordinate out;

  if (!in.is_empty())
  {
    auto const& l = in.location();
    out.GCS = in.crs();
    out.Easting = l[0];
    out.Northing = l[1];
  }

  return out;
}

//-----------------------------------------------------------------------------
kwiver::vital::geo_polygon toKwiver(vgGeocodedPoly const& in)
{
  kwiver::vital::geo_polygon::geo_raw_polygon_t poly;
  for (auto const& p : in.Coordinate)
  {
    poly.push_back({p.Easting, p.Northing});
  }
  return {poly, in.GCS};
}

//-----------------------------------------------------------------------------
kwiver::vital::query_filter toKwiver(vvDatabaseQuery::IntersectionType in)
{
  using namespace kwiver::vital;

  switch (in)
  {
    case vvDatabaseQuery::ContainsWholly:
      return query_filter::CONTAINS_WHOLLY;
    case vvDatabaseQuery::ContainsAny:
      return query_filter::CONTAINS_PARTLY;
    case vvDatabaseQuery::Intersects:
      return query_filter::INTERSECTS;
    case vvDatabaseQuery::IntersectsInbound:
      return query_filter::INTERSECTS_INBOUND;
    case vvDatabaseQuery::IntersectsOutbound:
      return query_filter::INTERSECTS_OUTBOUND;
    case vvDatabaseQuery::DoesNotContain:
      return query_filter::DOES_NOT_CONTAIN;
    default:
      return query_filter::IGNORE;
  }
}

//-----------------------------------------------------------------------------
vvTrackState fromKwiver(kwiver::vital::object_track_state const& in)
{
  vvTrackState out;

  out.TimeStamp.FrameNumber = in.frame();

  auto const& dp = in.detection;
  if (dp)
  {
    auto const& d = *dp;
    out.ImageBox = fromKwiver(d.bounding_box());
    out.ImagePoint = pointFromBox(out.ImageBox);
  }

  return out;
}

//-----------------------------------------------------------------------------
vvTrack fromKwiver(kwiver::vital::track const& in)
{
  vvTrack out;
  kwiver::vital::detected_object_type* type = nullptr;

  // Set track ID
  out.Id = {0, in.id()};

  // Copy track states
  for (auto const& s : in)
  {
    auto const& os = static_cast<kwiver::vital::object_track_state const&>(*s);
    out.Trajectory.insert(fromKwiver(os));

    // Store detected type for later application
    if (os.detection)
    {
      auto const& t = os.detection->type();
      type = (t ? t.get() : type);
    }
  }

  // Copy detection type
  if (type)
  {
    // FIXME this is likely very inefficient; need iterators over type!
    for (auto const& n : type->all_class_names())
    {
      if (type->has_class_name(n))
      {
        auto const p = type->score(n);
        out.Classification.emplace(n, p);
      }
    }
  }

  return out;
}

} // end namespace <anonymous>

//-----------------------------------------------------------------------------
kwiver::vital::descriptor_request_sptr toKwiver(vvProcessingRequest const& in)
{
  auto outPtr = std::make_shared<kwiver::vital::descriptor_request>();
  auto& out = *outPtr;

  out.set_id({in.QueryId});

  auto const& st = utcTimestamp(in.StartTime);
  auto const& et = utcTimestamp(in.EndTime);
  out.set_temporal_bounds(st, et);

  auto const& videoUri = qtUrl(in.VideoUri);
  out.set_data_location(stdString(videoUri.toLocalFile()));

  return outPtr;
}

//-----------------------------------------------------------------------------
vvDescriptor fromKwiver(kwiver::vital::track_descriptor const& in)
{
  vvDescriptor out;

  out.ModuleName = "KWIVER";
  out.DescriptorName = in.get_type();

  // Copy descriptor values
  std::vector<float> values;
  for (auto v : *(in.get_descriptor()))
  {
    values.push_back(static_cast<float>(v));
  }
  out.Values.push_back(values);

  // Copy tracks
  for (auto t : in.get_track_ids())
  {
    out.TrackIds.emplace_back(0, static_cast<long long>(t));
  }

  // Copy regions
  for (auto const& h : in.get_history())
  {
    out.Region.insert(fromKwiver(h));
  }

  return out;
}

//-----------------------------------------------------------------------------
kwiver::vital::track_descriptor_sptr toKwiver(vvDescriptor const& in)
{
  auto outPtr = kwiver::vital::track_descriptor::create(in.DescriptorName);
  auto& out = *outPtr;

  // Copy descriptor values
  auto const& values = flatten(in.Values);
  auto const ds = values.size();
  auto d = std::make_shared<kwiver::vital::descriptor_dynamic<double>>(ds);
  std::copy(values.begin(), values.end(), d->raw_data());

  // Copy tracks
  for (auto const& t : in.TrackIds)
  {
    out.add_track_id(static_cast<uint64_t>(t.SerialNumber));
  }

  // Copy regions
  for (auto const& r : in.Region)
  {
    out.add_history_entry(toKwiver(r));
  }

  return outPtr;
}

//-----------------------------------------------------------------------------
kwiver::vital::database_query_sptr toKwiver(vvQueryInstance const& in)
{
  auto const& abstract = *in.constAbstractQuery();
  auto outPtr = std::make_shared<kwiver::vital::database_query>();
  auto& out = *outPtr;

  // Set base attributes
  out.set_id(abstract.QueryId);

  out.set_stream_filter(abstract.StreamIdLimit);

  // Copy temporal limits
  out.set_temporal_bounds(utcTimestamp(abstract.TemporalLowerLimit),
                          utcTimestamp(abstract.TemporalUpperLimit));
  out.set_temporal_filter(toKwiver(abstract.TemporalFilter));

  // Copy spatial limits
  out.set_spatial_region(toKwiver(abstract.SpatialLimit));
  out.set_spatial_filter(toKwiver(abstract.SpatialFilter));

  if (in.isSimilarityQuery())
  {
    auto const& sq = *in.constSimilarityQuery();

    // Set query type
    out.set_type(kwiver::vital::database_query::SIMILARITY);
    out.set_threshold(sq.SimilarityThreshold);

    // Copy descriptors
    auto descriptors = std::make_shared<kwiver::vital::track_descriptor_set>();
    descriptors->reserve(sq.Descriptors.size());
    for (auto const& d : sq.Descriptors)
    {
      descriptors->push_back(toKwiver(d));
    }
    out.set_descriptors(descriptors);
  }
  else if (in.isRetrievalQuery())
  {
    // Set query type
    out.set_type(kwiver::vital::database_query::SIMILARITY);
    // TODO set retrieval entity filter (not yet implemented in KWIVER)
  }

  return outPtr;
}

//-----------------------------------------------------------------------------
vvQueryResult fromKwiver(kwiver::vital::query_result const& in)
{
  vvQueryResult out;

  // Copy base attributes
  out.InstanceId = in.instance_id();
  out.QueryId = in.query_id().value();
  out.StreamId = in.stream_id();
  out.RelevancyScore = in.relevancy_score();
  out.Location = fromKwiver(in.location());

  // Copy temporal bounds
  if (in.start_time().has_valid_time())
  {
    out.StartTime = in.start_time().get_time_usec();
  }
  if (in.end_time().has_valid_time())
  {
    out.EndTime = in.end_time().get_time_usec();
  }

  // Copy track states
  auto const& tsp = in.tracks();
  if (tsp)
  {
    for (auto const& tp : tsp->tracks())
    {
      out.Tracks.push_back(fromKwiver(*tp));
    }
  }

  // Copy descriptors
  auto const& dsp = in.descriptors();
  if (dsp)
  {
    for (auto const& dp : *dsp)
    {
      out.Descriptors.push_back(fromKwiver(*dp));
    }
  }

  return out;
}

//-----------------------------------------------------------------------------
kwiver::vital::iqr_feedback_sptr toKwiver(
  std::string const& queryId,
  vvIqr::ScoringClassifiers const& feedback)
{
  auto outPtr = std::make_shared<kwiver::vital::iqr_feedback>();
  auto& out = *outPtr;

  std::vector<unsigned> negatives;
  std::vector<unsigned> positives;

  for (auto iter = feedback.begin(); iter != feedback.end(); ++iter)
  {
    switch (iter.value())
    {
      case vvIqr::PositiveExample:
        positives.push_back(static_cast<unsigned>(iter.key()));
        break;
      case vvIqr::NegativeExample:
        negatives.push_back(static_cast<unsigned>(iter.key()));
        break;
      default:
        break;
    }
  }

  out.set_query_id({queryId});
  out.set_negative_ids(negatives);
  out.set_positive_ids(positives);
  return outPtr;
}
