// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <qtStlUtil.h>

#include "vvKstWriter.h"

QTE_IMPLEMENT_D_FUNC(vvKstWriter)

//-----------------------------------------------------------------------------
const unsigned int vvKstWriter::TracksVersion = 0;
const unsigned int vvKstWriter::DescriptorsVersion = 0;
const unsigned int vvKstWriter::QueryPlanVersion = 1;
const unsigned int vvKstWriter::QueryResultsVersion = 2;
const unsigned int vvKstWriter::EventSetInfoVersion = 0;
const unsigned int vvKstWriter::GeoPolyVersion = 0;

//-----------------------------------------------------------------------------
class vvKstWriterPrivate
{
public:
  enum Part
    {
    Record,
    Array,
    Value,
    String,
    None
    };

  vvKstWriterPrivate(QTextStream*, bool pretty, bool free);
  ~vvKstWriterPrivate() { if (this->freeStream) delete this->stream; }

  QTextStream* stream;
  bool freeStream;

  bool pretty;
  Part part;
  unsigned int indent;
  unsigned int arrayCount;
};

//-----------------------------------------------------------------------------
vvKstWriterPrivate::vvKstWriterPrivate(
  QTextStream* stream, bool pretty, bool free)
  : stream(stream), freeStream(free), pretty(pretty),
    part(None), indent(0), arrayCount(0)
{
}

//-----------------------------------------------------------------------------
vvKstWriter::vvKstWriter(QFile& file, bool pretty)
  : d_ptr(new vvKstWriterPrivate(new QTextStream(&file), pretty, true))
{
}

//-----------------------------------------------------------------------------
vvKstWriter::vvKstWriter(QTextStream& stream, bool pretty)
  : d_ptr(new vvKstWriterPrivate(&stream, pretty, false))
{
}

//-----------------------------------------------------------------------------
vvKstWriter::~vvKstWriter()
{
  QTE_D(vvKstWriter);

  // Terminate open strings
  *this << EndValue;

  // Terminate open arrays
  while (d->arrayCount)
    {
    *this << EndArray;
    }

  // Terminate record
  *this << (d->part == vvKstWriterPrivate::Value ? EndRecord : NoOp);
}

//-----------------------------------------------------------------------------
#define RECT_HEAD       "{ Top, Left }, { Bottom, Right }"
#define TRACK_ID_HEAD   " Source, Serial Number"

#define DESCRIPTOR_HEAD_1   "Descriptor name, Module name, Instance ID," \
                            " Confidence, Values, Region, Track ID's"
#define DESCRIPTOR_HEAD_2   " Frame Number, Time, Image Region" \
                            " { " RECT_HEAD " }"
#define DESCRIPTOR_HEAD_3   TRACK_ID_HEAD

#define TRACK_HEAD_1    "Track ID, Classification Entries, Trajectory States"
#define TRACK_HEAD_2    TRACK_ID_HEAD
#define TRACK_HEAD_3    " Type, Probability"
#define TRACK_HEAD_4    " Frame Number, Time, Image Point X, Image Point Y,"
#define TRACK_HEAD_5    " Image Box, Image Object Points, World Location"
#define TRACK_HEAD_6    "  " RECT_HEAD
#define TRACK_HEAD_7    "  X, Y"
#define TRACK_HEAD_8    "  [ GCS, Easting, Northing ]"

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(vvHeader::FileType fileType)
{
  switch (fileType)
    {
    case vvHeader::Tracks:
      *this << BeginValue << vvHeader::TracksTag << EndRecord
            << Comment << TRACK_HEAD_1 << NewLine
            << Comment << TRACK_HEAD_2 << NewLine
            << Comment << TRACK_HEAD_3 << NewLine
            << Comment << TRACK_HEAD_4 << NewLine
            << Comment << TRACK_HEAD_5 << NewLine
            << Comment << TRACK_HEAD_6 << NewLine
            << Comment << TRACK_HEAD_7 << NewLine
            << Comment << TRACK_HEAD_8 << NewLine;
      break;
    case vvHeader::Descriptors:
      *this << BeginValue << vvHeader::DescriptorsTag << EndRecord
            << Comment << DESCRIPTOR_HEAD_1 << NewLine
            << Comment << DESCRIPTOR_HEAD_2 << NewLine
            << Comment << DESCRIPTOR_HEAD_3 << NewLine;
      break;
    case vvHeader::QueryPlan:
      *this << BeginValue << vvHeader::QueryPlanTag
            << BeginValue << vvKstWriter::QueryPlanVersion << EndRecord
            << Comment << "Query Type" << NewLine
            << Comment << "Query ID [, Stream ID Limit ]" << NewLine
            << Comment << "Descriptors [, Tracks ] | Entity Type" << NewLine
            << Comment << " " DESCRIPTOR_HEAD_1 << NewLine
            << Comment << " " DESCRIPTOR_HEAD_2 << NewLine
            << Comment << " " DESCRIPTOR_HEAD_3 << NewLine
            << Comment << " " TRACK_HEAD_1 << NewLine
            << Comment << " " TRACK_HEAD_2 << NewLine
            << Comment << " " TRACK_HEAD_3 << NewLine
            << Comment << " " TRACK_HEAD_4 << NewLine
            << Comment << " " TRACK_HEAD_5 << NewLine
            << Comment << " " TRACK_HEAD_6 << NewLine
            << Comment << " " TRACK_HEAD_7 << NewLine
            << Comment << " " TRACK_HEAD_8 << NewLine
            << Comment << "{ [ Temporal Lower, Temporal Upper"
                          " [, Temporal Filter ] ] }," << NewLine
            << Comment << "{ [ GCS, Geospatial Limit Points"
                          " [, Geospatial Filter ] ] }" << NewLine
            << Comment << "  { Easting, Northing }" << NewLine
            << Comment << "[ Similarity Threshold, [ IQR Model ] ]" << NewLine;
      break;
    case vvHeader::QueryResults:
      *this << BeginValue << vvHeader::QueryResultsTag
            << BeginValue << vvKstWriter::QueryResultsVersion << EndRecord
            << Comment << "Mission ID, Query ID, Stream ID, Instance ID,"
                          " Start time, End time," << NewLine
            << Comment << "Location, Score, Descriptors, Tracks"
                          " [, User Notes ]" << NewLine
            << Comment << " GCS, Easting, Northing" << NewLine
            << Comment << " Rank, Relevancy, User Classification,"
                          " User Flags" << NewLine
            << Comment << " " DESCRIPTOR_HEAD_1 << NewLine
            << Comment << " " DESCRIPTOR_HEAD_2 << NewLine
            << Comment << " " DESCRIPTOR_HEAD_3 << NewLine;
      break;
    case vvHeader::EventSetInfo:
      *this << BeginValue << vvHeader::EventSetInfoTag << EndRecord
            << Comment << "Name, Pen Color, Foreground Color,"
                          " Background Color, Initial Filter Threshold"
                       << NewLine;
      break;
    default:
      break;
    }
  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(vvKstWriter::Marker marker)
{
  QTE_D(vvKstWriter);

  if (d->part == vvKstWriterPrivate::String)
    {
    d->part = vvKstWriterPrivate::Value;
    *this << '"';
    }

  switch (marker)
    {
    case EndValue:
      // Any marker ends a value, so this is essentially a no-op... useful for
      // things like '(condition ? NewLine : EndValue)'
    case NoOp:
      break;

    case NewLine:
      if (d->part == vvKstWriterPrivate::Value)
        {
        *this << ',';
        }
      if (d->part != vvKstWriterPrivate::None)
        {
        d->indent = qMax(d->indent, 1u);
        d->part = vvKstWriterPrivate::Record;
        }
      if (d->pretty)
        {
        *this << '\n' << QString(d->indent * 2, ' ');
        }
      break;

    case BeginRecord:
      if (d->part != vvKstWriterPrivate::None)
        {
        *this << EndRecord;
        }
      d->part = vvKstWriterPrivate::Record;
      break;

    case EndRecord:
      while (d->arrayCount)
        {
        *this << EndArray;
        }
      if (d->pretty)
        {
        *this << ";\n";
        }
      else
        {
        *this << ';';
        }
      d->part = vvKstWriterPrivate::None;
      d->indent = 0;
      break;

    case BeginArray:
      *this << BeginValue << '[';
      ++d->arrayCount;
      ++d->indent;
      d->part = vvKstWriterPrivate::Array;
      break;

    case EndArray:
      if (!d->arrayCount)
        {
        break;
        }
      if (d->part == vvKstWriterPrivate::Value && d->pretty)
        {
        *this << ' ';
        }
      *this << ']';
      --d->indent;
      --d->arrayCount;
      d->part = vvKstWriterPrivate::Value;
      break;

    case EndArrayNewLine:
      if (!d->arrayCount)
        {
        break;
        }
      if (d->pretty)
        {
        *this << '\n' << QString(d->indent * 2, ' ');
        }
      *this << ']';
      --d->indent;
      --d->arrayCount;
      d->part = vvKstWriterPrivate::Value;
      break;

    case BeginValue:
      if (d->part == vvKstWriterPrivate::Value)
        {
        if (d->pretty)
          {
          *this << ", ";
          }
        else
          {
          *this << ',';
          }
        }
      else if (d->part == vvKstWriterPrivate::Array && d->pretty)
        {
        *this << ' ';
        }
      d->part = vvKstWriterPrivate::Value;
      break;

    case BeginString:
      *this << BeginValue << '"';
      d->part = vvKstWriterPrivate::String;
      break;

    case Comment:
      *this << "# ";
      break;
    }

  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vvTrack& track)
{
  // Write ID
  *this << track.Id << NewLine;

  // Write classification
  *this << BeginArray;
  int n = 0;
  typedef vvTrackObjectClassification::const_iterator ClassificationIterator;
  foreach_iter (ClassificationIterator, iter, track.Classification)
    {
    *this << (n ? NewLine : NoOp) << BeginArray
          << BeginString << iter->first
          << BeginValue << iter->second
          << EndArray;
    ++n;
    }
  *this << EndArray << NewLine;

  // Write trajectory
  *this << BeginArray;
  n = 0;
  foreach_iter (vvTrackTrajectory::const_iterator, iter, track.Trajectory)
    {
    *this << (n ? NewLine : NoOp) << *iter;
    ++n;
    }
  *this << (n ? EndArrayNewLine : EndArray);

  // Terminate record, unless this is an array
  QTE_D(vvKstWriter);
  *this << (!d->arrayCount ? EndRecord : EndValue);

  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vvDescriptor& descriptor)
{
  size_t i;

  // Write descriptor header
  *this << BeginString << descriptor.DescriptorName
        << BeginString << descriptor.ModuleName
        << BeginValue << descriptor.InstanceId
        << BeginValue << descriptor.Confidence
        << NewLine;

  // Write descriptor values
  *this << BeginArray;
  for (i = 0; i < descriptor.Values.size(); ++i)
    {
    *this << (i ? NewLine : NoOp) << BeginArray;
    for (size_t j = 0; j < descriptor.Values[i].size(); ++j)
      {
      *this << BeginValue << descriptor.Values[i][j];
      }
    *this << EndArray;
    }
  *this << EndArray << NewLine;

  // Write descriptor image regions
  *this << BeginArray;
  foreach_iter (vvDescriptorRegionMap::const_iterator, iter, descriptor.Region)
    {
    *this << (iter != descriptor.Region.begin() ? NewLine : NoOp)
          << BeginArray << BeginValue;
    (iter->TimeStamp.HasFrameNumber()
     ? *this << iter->TimeStamp.FrameNumber
     : *this << -1);
    *this << BeginValue << static_cast<long long>(iter->TimeStamp.Time)
          << iter->ImageRegion
          << EndArray;
    }
  *this << EndArray << NewLine;

  // Write descriptor tracks
  *this << BeginArray;
  for (i = 0; i < descriptor.TrackIds.size(); ++i)
    {
    *this << descriptor.TrackIds[i];
    }
  *this << EndArray;

  // Terminate record, unless this is an array
  QTE_D(vvKstWriter);
  *this << (!d->arrayCount ? EndRecord : EndValue);

  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vvQueryInstance& query)
{
  if (query.isRetrievalQuery())
    {
    *this << "RETRIEVAL" << EndRecord;
    *this << *query.constRetrievalQuery();
    }
  else if (query.isSimilarityQuery())
    {
    *this << "SIMILARITY" << EndRecord;
    *this << *query.constSimilarityQuery();
    }
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vvRetrievalQuery& query)
{
  // Write query ID
  *this << BeginString << query.QueryId;
  if (!query.StreamIdLimit.empty())
    {
    *this << NewLine << BeginString << query.StreamIdLimit;
    }
  *this << EndRecord;

  // Write retrieval entity type
  switch (query.RequestedEntities)
    {
    case vvRetrievalQuery::Tracks:
      *this << BeginValue << "TRACKS" << EndRecord;
      break;
    case vvRetrievalQuery::Descriptors:
      *this << BeginValue << "TRACKS" << EndRecord;
      break;
    default:
      *this << BeginValue << "ALL" << EndRecord;
      break;
    }

  // Write temporal and geospatial limits
  *this << static_cast<const vvDatabaseQuery&>(query);

  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vvSimilarityQuery& query)
{
  size_t n;

  // Write query ID
  *this << BeginString << query.QueryId;
  if (!query.StreamIdLimit.empty())
    {
    *this << NewLine << BeginString << query.StreamIdLimit;
    }
  *this << EndRecord;

  // Write descriptors and tracks
  *this << BeginArray;
  for (n = 0; n < query.Descriptors.size(); ++n)
    {
    *this << NewLine << BeginArray << query.Descriptors[n];
    *this << EndArray;
    }
  if (!query.Tracks.empty())
    {
    *this << EndArray << NewLine << BeginArray;
    for (n = 0; n < query.Tracks.size(); ++n)
      {
      *this << NewLine << BeginArray << query.Tracks[n];
      *this << EndArray;
      }
    }
  *this << EndRecord;

  // Write temporal and geospatial limits
  *this << static_cast<const vvDatabaseQuery&>(query);

  // Write similarity
  *this << BeginValue << query.SimilarityThreshold;

  // Convert IQR model from std::vector<uchar>
  const char* rawModel =
    reinterpret_cast<const char*>(
      query.IqrModel.empty() ? 0 : &query.IqrModel[0]);
  const int modelSize = static_cast<int>(query.IqrModel.size());
  const QByteArray model = QByteArray::fromRawData(rawModel, modelSize);

  // Write IQR model
  *this << BeginValue << '\"' << model.toBase64().constData() << '\"'
        << EndRecord;

  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vvQueryResult& result)
{
  size_t n, k;

  // Write out result header
  *this << BeginString << result.MissionId
        << BeginString << result.QueryId << NewLine
        << BeginString << result.StreamId << NewLine
        << BeginValue << result.InstanceId
        << BeginValue << result.StartTime
        << BeginValue << result.EndTime << NewLine;
  *this << BeginArray
        << BeginValue << result.Location.GCS
        << BeginValue << result.Location.Easting
        << BeginValue << result.Location.Northing
        << EndArray;
  *this << BeginArray
        << BeginValue << result.Rank
        << BeginValue << result.RelevancyScore
        << BeginValue << static_cast<int>(result.UserScore)
        << BeginValue << static_cast<int>(result.UserData.Flags)
        << EndArray;

  // Write out descriptors
  *this << NewLine << BeginArray;
  for (n = 0, k = result.Descriptors.size(); n < k; ++n)
    {
    *this << NewLine << BeginArray << result.Descriptors[n];
    *this << EndArray;
    }
  *this << EndArrayNewLine;

  // Write out tracks
  *this << BeginArray;
  for (n = 0, k = result.Tracks.size(); n < k; ++n)
    {
    *this << NewLine << BeginArray << result.Tracks[n];
    *this << EndArray;
    }
  *this << (result.Tracks.empty() ? EndArray : EndArrayNewLine);

  // Write out notes
  if (!result.UserData.Notes.empty())
    {
    *this << BeginString << result.UserData.Notes;
    }

  *this << EndRecord;
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vgGeocodedPoly& poly)
{
  return *this << vvSpatialFilter(poly);
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vvSpatialFilter& sf)
{
  *this << BeginArray;
  if (sf.poly.Coordinate.size())
    {
    *this << BeginValue << sf.poly.GCS << NewLine << BeginArray;
    for (size_t n = 0; n < sf.poly.Coordinate.size(); ++n)
      {
      const vgGeoRawCoordinate& coord = sf.poly.Coordinate[n];
      *this << (n ? NewLine : NoOp)
            << BeginArray
            << BeginValue << coord.Northing
            << BeginValue << coord.Easting
            << EndArray;
      }
    *this << EndArray;
    if (sf.filter)
      {
      *this << NewLine << BeginValue << *sf.filter;
      }
    }
  *this << EndArray;
  return *this;
}

//-----------------------------------------------------------------------------
vvWriter& vvKstWriter::operator<<(const vvEventSetInfo& info)
{
  *this << BeginString << info.Name
        << BeginValue << info.PenColor
        << BeginValue << info.ForegroundColor
        << BeginValue << info.BackgroundColor
        << BeginValue << info.DisplayThreshold
        << EndRecord;
  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(const vvImageBoundingBox& box)
{
  *this << BeginArray << BeginArray
        << BeginValue << box.TopLeft.Y
        << BeginValue << box.TopLeft.X
        << EndArray << BeginArray
        << BeginValue << box.BottomRight.Y
        << BeginValue << box.BottomRight.X
        << EndArray << EndArray;
  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(const vvTrackId& tid)
{
  *this << BeginArray
        << BeginValue << tid.Source << BeginValue << tid.SerialNumber
        << EndArray;
  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(const vvTrackState& state)
{
  // Write time, image point, and image box
  *this << BeginArray
        << BeginValue << state.TimeStamp.FrameNumber
        << BeginValue << static_cast<long long>(state.TimeStamp.Time)
        << BeginValue << state.ImagePoint.X
        << BeginValue << state.ImagePoint.Y
        << state.ImageBox;

  // Write image object
  size_t n;
  *this << NewLine << BeginArray;
  for (n = 0; n < state.ImageObject.size(); ++n)
    {
    const vvImagePointF& p = state.ImageObject[n];
    *this << BeginArray << BeginValue << p.X << BeginValue << p.Y << EndArray;
    *this << ((n + 1) % 5 ? EndValue : NewLine);
    }
  *this << EndArray << (n ? NewLine : EndValue);

  // Write world location
  *this << BeginArray;
  if (state.WorldLocation.GCS != -1)
    {
    *this << BeginValue << state.WorldLocation.GCS
          << BeginValue << state.WorldLocation.Northing
          << BeginValue << state.WorldLocation.Easting;
    }
  *this << EndArray << EndArray;

  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(const vvDatabaseQuery& query)
{
  // Write temporal limit
  *this << BeginArray;
  if (query.TemporalLowerLimit != -1 || query.TemporalUpperLimit != -1)
    {
    *this << BeginValue << query.TemporalLowerLimit
          << BeginValue << query.TemporalUpperLimit
          << BeginValue << query.TemporalFilter;
    }
  *this << EndArray;

  // Write geospatial limit
  *this << (query.SpatialLimit.Coordinate.size() ? NewLine : NoOp)
        << vvSpatialFilter(query.SpatialLimit, query.SpatialFilter);
  *this << EndRecord;

  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(vvDatabaseQuery::IntersectionType filter)
{
  switch (filter)
    {
    case vvDatabaseQuery::Ignore:
      *this << "IGNORE";
      break;
    case vvDatabaseQuery::ContainsWholly:
      *this << "CONTAINS_WHOLLY";
      break;
    case vvDatabaseQuery::ContainsAny:
      *this << "CONTAINS_ANY";
      break;
    case vvDatabaseQuery::Intersects:
      *this << "INTERSECTS";
      break;
    case vvDatabaseQuery::IntersectsInbound:
      *this << "INTERSECTS_INBOUND";
      break;
    case vvDatabaseQuery::IntersectsOutbound:
      *this << "INTERSECTS_OUTBOUND";
      break;
    case vvDatabaseQuery::DoesNotContain:
      *this << "DOES_NOT_CONTAIN";
      break;
    }
  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(const QColor& color)
{
  QTE_D(vvKstWriter);
  (*d->stream) << QString("0x%1").arg(color.rgba(), 8, 16, QChar('0'));
  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(const char* data)
{
  QTE_D(vvKstWriter);
  if (d->part == vvKstWriterPrivate::String)
    {
    *this << QString::fromLocal8Bit(data);
    }
  else
    {
    (*d->stream) << data;
    }
  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(const std::string& data)
{
  QTE_D(vvKstWriter);
  if (d->part == vvKstWriterPrivate::String)
    {
    *this << qtString(data);
    }
  else
    {
    (*d->stream) << qtString(data);
    }
  return *this;
}

//-----------------------------------------------------------------------------
vvKstWriter& vvKstWriter::operator<<(const QString& data)
{
  QTE_D(vvKstWriter);

  QString edata = data;
  if (d->part == vvKstWriterPrivate::String)
    {
    edata.replace("\\", "\\\\");
    edata.replace("\"", "\\\"");
    }

  *d->stream << edata;
  return *this;
}

//-----------------------------------------------------------------------------
#define writer_sout_operator(_class) \
  vvKstWriter& vvKstWriter::operator<<(_class data) \
  { \
    QTE_D(vvKstWriter); \
    (*d->stream) << data; \
    return *this; \
  }
writer_sout_operator(const QChar&)
writer_sout_operator(char)
writer_sout_operator(qint32)
writer_sout_operator(qint64)
writer_sout_operator(quint32)
writer_sout_operator(quint64)
writer_sout_operator(float)
writer_sout_operator(double)
