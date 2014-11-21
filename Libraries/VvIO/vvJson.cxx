/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvJson.h"

#include <vvDescriptor.h>
#include <vvQueryResult.h>
#include <vvTrack.h>
#include <vvUserData.h>

#include <vgGeoTypes.h>
#include <vgTimeStamp.h>

#include <qtStlUtil.h>

namespace vvJson
{

namespace // anonymous
{

template <typename T> qtJson::Array serialize(const std::vector<T>&);
template <typename T> qtJson::Array serialize(const std::set<T>&);

using vvJson::serialize;

//-----------------------------------------------------------------------------
QVariant serialize(float value)
{
  // This seemingly silly function allows us to convert descriptor values
  // entirely leveraging the generic array handler below. Gotta love
  // templates...
  return value;
}

//-----------------------------------------------------------------------------
template <typename T>
qtJson::Array serialize(const std::vector<T>& list)
{
  qtJson::Array result;

  const size_t k = list.size();
  result.reserve(static_cast<int>(k));

  for (size_t i = 0; i < k; ++i)
    {
    result.append(serialize(list[i]));
    }

  return result;
}

//-----------------------------------------------------------------------------
template <typename T>
qtJson::Array serialize(const std::set<T>& set)
{
  qtJson::Array result;

  typedef typename std::set<T>::const_iterator Iterator;
  const Iterator end = set.end();
  for (Iterator iter = set.begin(); iter != end; ++iter)
    {
    result.append(serialize(*iter));
    }

  return result;
}

} // namespace <anonymous>

} // namespace vvJson

//-----------------------------------------------------------------------------
qtJson::Value vvJson::serialize(const vgTimeStamp& ts)
{
  if (!ts.IsValid())
    {
    return qtJson::Value();
    }

  qtJson::Object object;

  if (ts.HasTime())
    {
    object.insert("time", qRound64(ts.Time));
    }
  if (ts.HasFrameNumber())
    {
    object.insert("frame", ts.FrameNumber);
    }

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vgGeoRawCoordinate& coord)
{
  qtJson::Object object;

  object.insert("northing", coord.Northing);
  object.insert("easting", coord.Easting);

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Value vvJson::serialize(const vgGeocodedCoordinate& coord)
{
  if (coord.GCS == -1)
    {
    return qtJson::Value();
    }

  const vgGeoRawCoordinate& rawCoord = coord;
  qtJson::Object object = serialize(rawCoord);

  object.insert("gcs", coord.GCS);

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvImagePointF& point)
{
  qtJson::Object object;

  object.insert("x", point.X);
  object.insert("y", point.Y);

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvImageBoundingBox& box)
{
  qtJson::Object object;

  object.insert("top", box.TopLeft.Y);
  object.insert("left", box.TopLeft.X);
  object.insert("bottom", box.BottomRight.Y);
  object.insert("right", box.BottomRight.X);

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvTrackId& tid)
{
  qtJson::Object object;

  object.insert("source", tid.Source);
  object.insert("serialNumber", tid.SerialNumber);

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvTrackState& state)
{
  qtJson::Object object;

  object.insert("timeStamp", serialize(state.TimeStamp));
  object.insert("imagePoint", serialize(state.ImagePoint));
  object.insert("imageBox", serialize(state.ImageBox));
  object.insert("imageObject", serialize(state.ImageObject));
  object.insert("worldLocation", serialize(state.WorldLocation));

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvTrack& track)
{
  qtJson::Object object;

  object.insert("id", serialize(track.Id));
  object.insert("trajectory", serialize(track.Trajectory));

  qtJson::Object classification;
  typedef vvTrackObjectClassification::const_iterator Iterator;
  const Iterator end = track.Classification.end();
  for (Iterator iter = track.Classification.begin(); iter != end; ++iter)
    {
    classification.insert(qtString(iter->first), iter->second);
    }
  object.insert("classification", classification);

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvDescriptorRegionEntry& region)
{
  qtJson::Object object;

  object.insert("timeStamp", serialize(region.TimeStamp));
  object.insert("imageRegion", serialize(region.ImageRegion));

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvDescriptor& descriptor)
{
  qtJson::Object object;

  object.insert("descriptorName", qtString(descriptor.DescriptorName));
  object.insert("moduleName", qtString(descriptor.ModuleName));
  object.insert("instanceId", descriptor.InstanceId);
  object.insert("confidence", descriptor.Confidence);
  object.insert("values", serialize(descriptor.Values));
  object.insert("trackIds", serialize(descriptor.TrackIds));

  qtJson::Array regions;
  const vvDescriptorRegionMap::const_iterator end = descriptor.Region.end();
  for (vvDescriptorRegionMap::const_iterator iter = descriptor.Region.begin();
       iter != end; ++iter)
    {
    regions.append(serialize(*iter));
    }
  object.insert("regions", regions);

  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvUserData::Data& data)
{
  qtJson::Object object;
  object.insert("flags", static_cast<int>(data.Flags));
  object.insert("notes", qtString(data.Notes));
  return object;
}

//-----------------------------------------------------------------------------
qtJson::Object vvJson::serialize(const vvQueryResult& result)
{
  qtJson::Object object;

  object.insert("instanceId", result.InstanceId);

  object.insert("rank", result.Rank);
  object.insert("relevancyScore", result.RelevancyScore);
  object.insert("preferenceScore", result.PreferenceScore);

  object.insert("queryId", qtString(result.QueryId));
  object.insert("missionId", qtString(result.MissionId));
  object.insert("streamId", qtString(result.StreamId));

  object.insert("startTime", result.StartTime);
  object.insert("endTime", result.EndTime);
  object.insert("location", serialize(result.Location));

  object.insert("tracks", serialize(result.Tracks));
  object.insert("descriptors", serialize(result.Descriptors));

  object.insert("userScore", static_cast<int>(result.UserScore));
  object.insert("userData", serialize(result.UserData));

  return object;
}
