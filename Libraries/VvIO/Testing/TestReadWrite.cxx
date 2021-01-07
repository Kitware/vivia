// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <qtTest.h>

#include <vvQueryResult.h>

#include "../vvEventSetInfo.h"
#include "../vvQueryInstance.h"

#include "TestReadWrite.h"

//-----------------------------------------------------------------------------
template <typename T> static
const T* data(const std::vector<T>& vector)
{
#ifdef __GLIBCXX__
  // std::vector from gcc libstdc++ has ::data()
  return vector.data();
#else
  // std::vector from MSVC does not have ::data(), so we must fake it
  return (vector.empty() ? 0 : &vector[0]);
#endif
}

//-----------------------------------------------------------------------------
void testRegion(qtTest& testObject, const vvDescriptorRegionEntry& region,
                unsigned int frameNumber, double time,
                int tlY, int tlX, int brY, int brX)
{
  TEST_EQUAL(region.TimeStamp.FrameNumber, frameNumber);
  TEST_EQUAL(region.TimeStamp.Time, time);
  TEST_EQUAL(region.ImageRegion.TopLeft.X, tlX);
  TEST_EQUAL(region.ImageRegion.TopLeft.Y, tlY);
  TEST_EQUAL(region.ImageRegion.BottomRight.X, brX);
  TEST_EQUAL(region.ImageRegion.BottomRight.Y, brY);
}

//-----------------------------------------------------------------------------
void testTrackState(qtTest& testObject, const vvTrackState& state,
                    unsigned int frameNumber, double time,
                    bool expectObjectAndLocation)
{
  TEST_EQUAL(state.TimeStamp.FrameNumber, frameNumber);
  TEST_EQUAL(state.TimeStamp.Time, time);
  TEST_EQUAL(state.ImagePoint.X, 434.5);
  TEST_EQUAL(state.ImagePoint.Y, 241.0);
  TEST_EQUAL(state.ImageBox.TopLeft.X, 428);
  TEST_EQUAL(state.ImageBox.TopLeft.Y, 231);
  TEST_EQUAL(state.ImageBox.BottomRight.X, 441);
  TEST_EQUAL(state.ImageBox.BottomRight.Y, 241);
  if (expectObjectAndLocation)
    {
    if (TEST_EQUAL(state.ImageObject.size(), size_t(4)) == 0)
      {
      TEST_EQUAL(state.ImageObject[0].X, 428);
      TEST_EQUAL(state.ImageObject[0].Y, 231);
      TEST_EQUAL(state.ImageObject[1].X, 441);
      TEST_EQUAL(state.ImageObject[1].Y, 231);
      TEST_EQUAL(state.ImageObject[2].X, 441);
      TEST_EQUAL(state.ImageObject[2].Y, 241);
      TEST_EQUAL(state.ImageObject[3].X, 428);
      TEST_EQUAL(state.ImageObject[3].Y, 241);
      }
    TEST_EQUAL(state.WorldLocation.GCS, 4269);
    TEST_EQUAL(state.WorldLocation.Latitude, 38.2117);
    TEST_EQUAL(state.WorldLocation.Longitude, -81.481);
    }
  else
    {
    TEST_EQUAL(state.ImageObject.size(), size_t(0));
    TEST_EQUAL(state.WorldLocation.GCS, -1);
    }
}

//-----------------------------------------------------------------------------
void testTrack1(qtTest& testObject, const vvTrack& track)
{
  TEST_EQUAL(track.Id.Source, 0);
  TEST_EQUAL(track.Id.SerialNumber, 1LL);
  if (TEST_EQUAL(track.Classification.size(), size_t(2)) == 0)
    {
    typedef vvTrackObjectClassification::const_iterator TocIterator;
    TocIterator iter, notFound = track.Classification.end();

    TEST((iter = track.Classification.find("Person")) != notFound);
    (iter != notFound) && TEST_EQUAL(iter->second, 0.8);
    TEST((iter = track.Classification.find("Vehicle")) != notFound);
    (iter != notFound) && TEST_EQUAL(iter->second, 0.2);
    }
  if (TEST_EQUAL(track.Trajectory.size(), size_t(1)) == 0)
    {
    vvTrackTrajectory::const_iterator i = track.Trajectory.begin();
    TEST_CALL(testTrackState, *(i++), 26, 1222116851008654, true);
    }
}

//-----------------------------------------------------------------------------
void testTrack2(qtTest& testObject, const vvTrack& track)
{
  TEST_EQUAL(track.Id.Source, 0);
  TEST_EQUAL(track.Id.SerialNumber, 2LL);
  if (TEST_EQUAL(track.Classification.size(), size_t(3)) == 0)
    {
    typedef vvTrackObjectClassification::const_iterator TocIterator;
    TocIterator iter, notFound = track.Classification.end();

    TEST((iter = track.Classification.find("Person")) != notFound);
    (iter != notFound) && TEST_EQUAL(iter->second, 0.6);
    TEST((iter = track.Classification.find("Vehicle")) != notFound);
    (iter != notFound) && TEST_EQUAL(iter->second, 0.3);
    TEST((iter = track.Classification.find("Other")) != notFound);
    (iter != notFound) && TEST_EQUAL(iter->second, 0.1);
    }
  if (TEST_EQUAL(track.Trajectory.size(), size_t(2)) == 0)
    {
    vvTrackTrajectory::const_iterator i = track.Trajectory.begin();
    TEST_CALL(testTrackState, *(i++), 30, 1222116851409054, true);
    TEST_CALL(testTrackState, *(i++), 31, 1222116851509154, true);
    }
}

//-----------------------------------------------------------------------------
void testTrack3(qtTest& testObject, const vvTrack& track)
{
  TEST_EQUAL(track.Id.Source, 0);
  TEST_EQUAL(track.Id.SerialNumber, 1LL);
  TEST_EQUAL(track.Classification.size(), size_t(0));
  if (TEST_EQUAL(track.Trajectory.size(), size_t(1)) == 0)
    {
    vvTrackTrajectory::const_iterator i = track.Trajectory.begin();
    TEST_CALL(testTrackState, *(i++), 26, 1222116851008654, false);
    }
}

//-----------------------------------------------------------------------------
void testDescriptor1(qtTest& testObject, const vvDescriptor& descriptor)
{
  TEST_EQUAL(descriptor.DescriptorName, std::string("TEST1"));
  TEST_EQUAL(descriptor.ModuleName, std::string("sample"));
  TEST_EQUAL(descriptor.InstanceId, 0LL);
  TEST_EQUAL(descriptor.Confidence, 0.3);
  if (TEST_EQUAL(descriptor.Values.size(), size_t(2)) == 0)
    {
    if (TEST_EQUAL(descriptor.Values[0].size(), size_t(2)) == 0)
      {
      TEST_EQUAL(descriptor.Values[0][0], 1.0f);
      TEST_EQUAL(descriptor.Values[0][1], 42.0f);
      }
    if (TEST_EQUAL(descriptor.Values[1].size(), size_t(1)) == 0)
      {
      TEST_EQUAL(descriptor.Values[1][0], 3.1416f);
      }
    }
  if (TEST_EQUAL(descriptor.Region.size(), size_t(3)) == 0)
    {
    vvDescriptorRegionMap::const_iterator i = descriptor.Region.begin();
    TEST_CALL(testRegion, *(i++), 13, 1302035289300000, 230, 430, 240, 440);
    TEST_CALL(testRegion, *(i++), 14, 1302035289400000, 230, 430, 240, 440);
    TEST_CALL(testRegion, *(i++), 15, 1302035289500000, 230, 430, 240, 440);
    }
}

//-----------------------------------------------------------------------------
void testDescriptor2(qtTest& testObject, const vvDescriptor& descriptor)
{
  TEST_EQUAL(descriptor.DescriptorName, std::string("TEST2"));
  TEST_EQUAL(descriptor.ModuleName, std::string("sample"));
  TEST_EQUAL(descriptor.InstanceId, 1LL);
  TEST_EQUAL(descriptor.Confidence, 0.1);
  if (TEST_EQUAL(descriptor.Values.size(), size_t(1)) == 0)
    {
    if (TEST_EQUAL(descriptor.Values[0].size(), size_t(3)) == 0)
      {
      TEST_EQUAL(descriptor.Values[0][0], 2.0f);
      TEST_EQUAL(descriptor.Values[0][1], 37.0f);
      TEST_EQUAL(descriptor.Values[0][2], 2.71828f);
      }
    }
  if (TEST_EQUAL(descriptor.Region.size(), size_t(5)) == 0)
    {
    vvDescriptorRegionMap::const_iterator i = descriptor.Region.begin();
    TEST_CALL(testRegion, *(i++), 27, 1302035290700000, 110, 350, 120, 360);
    TEST_CALL(testRegion, *(i++), 28, 1302035290800000, 110, 350, 120, 360);
    TEST_CALL(testRegion, *(i++), 29, 1302035290900000, 110, 350, 120, 360);
    TEST_CALL(testRegion, *(i++), 30, 1302035291000000, 110, 350, 120, 360);
    TEST_CALL(testRegion, *(i++), 31, 1302035291100000, 110, 350, 120, 360);
    }
}

//-----------------------------------------------------------------------------
void testDescriptor3(qtTest& testObject, const vvDescriptor& descriptor)
{
  TEST_EQUAL(descriptor.DescriptorName, std::string("TEST3"));
  TEST_EQUAL(descriptor.ModuleName, std::string("sample"));
  TEST_EQUAL(descriptor.InstanceId, 2LL);
  TEST_EQUAL(descriptor.Confidence, 0.7);
  if (TEST_EQUAL(descriptor.Values.size(), size_t(3)) == 0)
    {
    if (TEST_EQUAL(descriptor.Values[0].size(), size_t(1)) == 0)
      {
      TEST_EQUAL(descriptor.Values[0][0], 3.0f);
      }
    if (TEST_EQUAL(descriptor.Values[1].size(), size_t(2)) == 0)
      {
      TEST_EQUAL(descriptor.Values[1][0], 1.618f);
      TEST_EQUAL(descriptor.Values[1][1], 9.80665f);
      }
    if (TEST_EQUAL(descriptor.Values[2].size(), size_t(2)) == 0)
      {
      TEST_EQUAL(descriptor.Values[2][0], 49.0f);
      TEST_EQUAL(descriptor.Values[2][1], 12.0f);
      }
    }
  if (TEST_EQUAL(descriptor.Region.size(), size_t(2)) == 0)
    {
    vvDescriptorRegionMap::const_iterator i = descriptor.Region.begin();
    TEST_CALL(testRegion, *(i++), 63, 1302035294300000, 460, 180, 470, 190);
    TEST_CALL(testRegion, *(i++), 64, 1302035294400000, 460, 180, 470, 190);
    }
}

//-----------------------------------------------------------------------------
void testGeoPoly1(qtTest& testObject, const vgGeocodedPoly& geoPoly)
{
  TEST_EQUAL(geoPoly.GCS, 4269);
  if (TEST_EQUAL(geoPoly.Coordinate.size(), size_t(4)) == 0)
    {
    TEST_EQUAL(geoPoly.Coordinate[0].Latitude, 38.2);
    TEST_EQUAL(geoPoly.Coordinate[1].Latitude, 38.3);
    TEST_EQUAL(geoPoly.Coordinate[2].Latitude, 38.3);
    TEST_EQUAL(geoPoly.Coordinate[3].Latitude, 38.2);
    TEST_EQUAL(geoPoly.Coordinate[0].Longitude, -81.4);
    TEST_EQUAL(geoPoly.Coordinate[1].Longitude, -81.4);
    TEST_EQUAL(geoPoly.Coordinate[2].Longitude, -81.6);
    TEST_EQUAL(geoPoly.Coordinate[3].Longitude, -81.6);
    }
}

//-----------------------------------------------------------------------------
void testGeoPoly2(qtTest& testObject, const vgGeocodedPoly& geoPoly)
{
  TEST_EQUAL(geoPoly.GCS, -1);
  TEST_EQUAL(geoPoly.Coordinate.size(), size_t(0));
}

//-----------------------------------------------------------------------------
void testGeoPoly3(qtTest& testObject, const vgGeocodedPoly& geoPoly)
{
  TEST_EQUAL(geoPoly.GCS, 4269);
  if (TEST_EQUAL(geoPoly.Coordinate.size(), size_t(4)) == 0)
    {
    TEST_EQUAL(geoPoly.Coordinate[0].Latitude, 42.8);
    TEST_EQUAL(geoPoly.Coordinate[1].Latitude, 42.9);
    TEST_EQUAL(geoPoly.Coordinate[2].Latitude, 42.9);
    TEST_EQUAL(geoPoly.Coordinate[3].Latitude, 42.8);
    TEST_EQUAL(geoPoly.Coordinate[0].Longitude, -73.7);
    TEST_EQUAL(geoPoly.Coordinate[1].Longitude, -73.7);
    TEST_EQUAL(geoPoly.Coordinate[2].Longitude, -73.8);
    TEST_EQUAL(geoPoly.Coordinate[3].Longitude, -73.8);
    }
}

//-----------------------------------------------------------------------------
void testQueryPlan1(qtTest& testObject, const vvQueryInstance& qi)
{
  if (TEST(qi.isSimilarityQuery()) != 0)
    return;

  const vvSimilarityQuery& query = *qi.constSimilarityQuery();
  TEST_EQUAL(query.QueryId, std::string("QUERY-EXAMPLE-12345"));
  TEST_EQUAL(query.StreamIdLimit,
             std::string("file:///example/path/to/video.mpg"));
  if (TEST_EQUAL(query.Descriptors.size(), size_t(2)) == 0)
    {
    TEST_CALL(testDescriptor1, query.Descriptors[0]);
    TEST_CALL(testDescriptor2, query.Descriptors[1]);
    }
  if (TEST_EQUAL(query.Tracks.size(), size_t(1)) == 0)
    {
    TEST_CALL(testTrack1, query.Tracks[0]);
    }
  TEST_EQUAL(query.TemporalLowerLimit, -1LL);
  TEST_EQUAL(query.TemporalUpperLimit, -1LL);
  TEST_EQUAL(query.TemporalFilter, vvDatabaseQuery::Ignore);
  TEST_CALL(testGeoPoly2, query.SpatialLimit);
  TEST_EQUAL(query.SpatialFilter, vvDatabaseQuery::Ignore);
  TEST_EQUAL(query.SimilarityThreshold, 0.1);

  const char* rawModel =
    reinterpret_cast<const char*>(data(query.IqrModel));
  const int modelSize = static_cast<int>(query.IqrModel.size());

  const QByteArray expected = QByteArray("Hello, world!").toBase64();
  const QByteArray actual = QByteArray::fromRawData(rawModel, modelSize);

  TEST_EQUAL(actual.toBase64(), expected);
}

//-----------------------------------------------------------------------------
void testQueryPlan2(qtTest& testObject, const vvQueryInstance& qi)
{
  if (TEST(qi.isSimilarityQuery()) != 0)
    return;

  const vvSimilarityQuery& query = *qi.constSimilarityQuery();
  TEST_EQUAL(query.QueryId, std::string("QUERY-EXAMPLE-9876"));
  TEST_EQUAL(query.StreamIdLimit, std::string());
  if (TEST_EQUAL(query.Descriptors.size(), size_t(1)) == 0)
    {
    TEST_CALL(testDescriptor3, query.Descriptors[0]);
    }
  TEST_EQUAL(query.TemporalLowerLimit, 1302213938000000LL);
  TEST_EQUAL(query.TemporalUpperLimit, 1302215277000000LL);
  TEST_EQUAL(query.TemporalFilter, vvDatabaseQuery::ContainsAny);
  TEST_CALL(testGeoPoly1, query.SpatialLimit);
  TEST_EQUAL(query.SpatialFilter, vvDatabaseQuery::ContainsAny);
  TEST_EQUAL(query.SimilarityThreshold, 0.0);
}

//-----------------------------------------------------------------------------
void testQueryPlan3(qtTest& testObject, const vvQueryInstance& qi)
{
  if (TEST(qi.isRetrievalQuery()) != 0)
    {
    return;
    }

  const vvRetrievalQuery& query = *qi.constRetrievalQuery();
  TEST_EQUAL(query.QueryId, std::string("QUERY-EXAMPLE-24680"));
  TEST_EQUAL(query.StreamIdLimit, std::string());
  TEST_EQUAL(query.RequestedEntities, vvRetrievalQuery::Tracks);
  TEST_EQUAL(query.TemporalLowerLimit, 1313790109000000LL);
  TEST_EQUAL(query.TemporalUpperLimit, 1313847211000000LL);
  TEST_EQUAL(query.TemporalFilter, vvDatabaseQuery::ContainsWholly);
  TEST_CALL(testGeoPoly3, query.SpatialLimit);
  TEST_EQUAL(query.SpatialFilter, vvDatabaseQuery::Intersects);
}

//-----------------------------------------------------------------------------
void testQueryResult1(qtTest& testObject, const vvQueryResult& result,
                      unsigned int version)
{
  TEST_EQUAL(result.MissionId, std::string("MISSION-6734"));
  TEST_EQUAL(result.QueryId, std::string("QUERY-EXAMPLE-2298537"));
  TEST_EQUAL(result.StreamId, std::string("INGEST-769235514"));
  TEST_EQUAL(result.InstanceId, 0LL);
  TEST_EQUAL(result.StartTime, 1302035289300000LL);
  TEST_EQUAL(result.EndTime, 1302035291100000LL);
  TEST_EQUAL(result.Location.GCS, 4326);
  TEST_EQUAL(result.Location.Latitude, 38.27);
  TEST_EQUAL(result.Location.Longitude, -81.51);
  TEST_EQUAL(result.Rank, 0LL);
  TEST_EQUAL(result.RelevancyScore, 0.6);
  TEST_EQUAL(result.UserScore, vvIqr::UnclassifiedExample);
  if (TEST_EQUAL(result.Descriptors.size(), size_t(2)) == 0)
    {
    TEST_CALL(testDescriptor1, result.Descriptors[0]);
    TEST_CALL(testDescriptor2, result.Descriptors[1]);
    }
  if (version > 1)
    {
    TEST_EQUAL(result.Tracks.size(), size_t(0));
    TEST_EQUAL(result.UserData.Flags.testFlag(vvUserData::Starred), true);
    TEST_EQUAL(result.UserData.Notes,
               std::string("This result is fascinating!"));
    }
}

//-----------------------------------------------------------------------------
void testQueryResult2(qtTest& testObject, const vvQueryResult& result,
                      unsigned int version)
{
  TEST_EQUAL(result.MissionId, std::string("MISSION-6734"));
  TEST_EQUAL(result.QueryId, std::string("QUERY-EXAMPLE-2298537"));
  TEST_EQUAL(result.StreamId, std::string("INGEST-769235514"));
  TEST_EQUAL(result.InstanceId, 1LL);
  TEST_EQUAL(result.StartTime, 1302035294300000LL);
  TEST_EQUAL(result.EndTime, 1302035294400000LL);
  TEST_EQUAL(result.Location.GCS, 4326);
  TEST_EQUAL(result.Location.Latitude, 38.23);
  TEST_EQUAL(result.Location.Longitude, -81.42);
  TEST_EQUAL(result.Rank, 1LL);
  TEST_EQUAL(result.RelevancyScore, 0.4);
  TEST_EQUAL(result.UserScore, vvIqr::PositiveExample);
  if (TEST_EQUAL(result.Descriptors.size(), size_t(1)) == 0)
    {
    TEST_CALL(testDescriptor3, result.Descriptors[0]);
    }
  if (version > 1)
    {
    if (TEST_EQUAL(result.Tracks.size(), size_t(2)) == 0)
      {
      TEST_CALL(testTrack1, result.Tracks[0]);
      TEST_CALL(testTrack2, result.Tracks[1]);
      }
    TEST_EQUAL(result.UserData.Flags.testFlag(vvUserData::Starred), false);
    TEST_EQUAL(result.UserData.Notes, std::string(""));
    }
}

//-----------------------------------------------------------------------------
void testEventSetInfo(qtTest& testObject, const vvEventSetInfo& info)
{
  TEST_EQUAL(info.Name, QString("Example"));
  TEST_EQUAL(info.PenColor, QColor(Qt::red));
  TEST_EQUAL(info.ForegroundColor, QColor(Qt::white));
  TEST_EQUAL(info.BackgroundColor, QColor::fromRgb(0, 0, 255, 128));
  TEST_EQUAL(info.DisplayThreshold, 0.1);
}
