/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvKstReader.h"

#include <QDebug>

#include <qtKstReader.h>
#include <qtStlUtil.h>

#include "vvKstWriter.h"

#define die(_msg) return this->abort(_msg)
#define accept(_expr) do { _expr; return true; } while (0)

#define test_or_fail(_cond) if (!(_cond)) return false
#define test_or_die(_cond, _msg) if (!(_cond)) die(_msg)
#define test_and_accept(_cond, _expr) if (_cond) accept(_expr)

#define check_version(_name, _ver) \
  if (version> _ver) \
    die("Unable to read " _name " version " + QString::number(version) \
        + ": latest recognized version is " + QString::number(_ver))

#define BEGIN_MAP_FROM_STRING(_type, _str, _val) \
  do { const QString _mfs_in = _str; _type &_mfs_out = _val; if (0)
#define MAP_FROM_STRING(_str, _val) \
  else if (_mfs_in == _str) _mfs_out = _val
#define END_MAP_FROM_STRING(_expr) \
  else { _expr; } } while (0)

QTE_IMPLEMENT_D_FUNC(vvKstReader)

//BEGIN vvKstReaderPrivate

//-----------------------------------------------------------------------------
class vvKstReaderPrivate
{
public:
  QScopedPointer<qtKstReader> kst;
  vvHeader header;

  QString lastError;

  bool abort(const QString&);

  template <typename T>
  bool read(T& result, vvHeader::FileType type, vvKstReader& q,
            bool (vvKstReader::*method)(qtKstReader&, T&, unsigned int));
  template <typename L, typename T>
  bool read(L& list, qtKstReader& reader, vvKstReader& q, unsigned int version,
            bool (vvKstReader::*method)(qtKstReader&, T&, unsigned int));

  bool readColor(qtKstReader& reader, QColor& color,
                 const QString& itemName, int value = -1);
  bool readTimeStamp(qtKstReader& reader, vgTimeStamp& ts,
                     const QString& itemName, int value = -1);
  bool readImageBoundingBox(qtKstReader& reader, vvImageBoundingBox& box,
                            const QString& itemName, int value = -1);
  bool readTrackState(qtKstReader& reader, vvTrackState& state);
  bool readDescriptorRegion(qtKstReader& reader, vvDescriptor& descriptor);
  bool readGeoPoly(qtKstReader& reader, vgGeocodedPoly& poly,
                   unsigned int version, QString* filterMode = 0);
  bool readQueryCommonHeader(qtKstReader& reader, vvDatabaseQuery& query,
                             unsigned int version);
  bool readQueryCommonLimits(qtKstReader& reader, vvDatabaseQuery& query,
                             unsigned int version);
  bool setFilterMode(vvDatabaseQuery::IntersectionType& out, QString in);
};

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::abort(const QString& error)
{
  qDebug() << "vvKstReader:" << qPrintable(error);
  this->lastError = error;
  return false;
}

//-----------------------------------------------------------------------------
template <typename T>
bool vvKstReaderPrivate::read(
  T& result, vvHeader::FileType type, vvKstReader& q,
  bool (vvKstReader::*method)(qtKstReader&, T&, unsigned int))
{
  test_or_die(this->kst, "No data is available");
  test_or_die(this->header.isValid(),
              "Header is invalid or has not been read/set");
  test_or_die(this->header.type == type,
              "Header type does not match requested data type");

  return (q.*method)(*this->kst, result, this->header.version);
}

//-----------------------------------------------------------------------------
template <typename L, typename T>
bool vvKstReaderPrivate::read(
  L& list, qtKstReader& reader, vvKstReader& q, unsigned int version,
  bool (vvKstReader::*method)(qtKstReader&, T&, unsigned int))
{
  test_or_die(reader.isValid(), "No data is available");
  test_or_die(!reader.isEndOfFile(), "Read position is at end of data");

  list.clear();
  while (!reader.isEndOfFile())
    {
    // Read item
    T item;
    test_or_fail((q.*method)(reader, item, version));
    list.push_back(item);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::readColor(
  qtKstReader& reader, QColor& color, const QString& itemName, int value)
{
  long long rawValue;
  test_or_die(reader.readLong(rawValue, value),
              "Error reading " + itemName + " color");
  color = QColor::fromRgba(static_cast<QRgb>(rawValue));
  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::readTimeStamp(
  qtKstReader& reader, vgTimeStamp& ts, const QString& itemName, int value)
{
  (value < 0) && (value = reader.currentValue());

  long long frameNumber, timeCode;
  test_or_die(reader.readLong(frameNumber, value),
              "Error reading " + itemName + " frame number");
  ts.FrameNumber = (frameNumber > 0
                    ? static_cast<unsigned int>(frameNumber)
                    : vgTimeStamp::InvalidFrameNumber());
  test_or_die(reader.readLong(timeCode, value + 1),
              "Error reading " + itemName + " time");
  ts.Time = static_cast<double>(timeCode);
  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::readImageBoundingBox(
  qtKstReader& reader, vvImageBoundingBox& box,
  const QString& itemName, int value)
{
  qtKstReader boxReader;
  test_or_die(reader.readTable(boxReader, value),
              "Error reading " + itemName);
  test_or_die(boxReader.readInt(box.TopLeft.Y, 0, 0),
              "Error reading " + itemName + " top");
  test_or_die(boxReader.readInt(box.TopLeft.X, 1, 0),
              "Error reading " + itemName + " left");
  test_or_die(boxReader.readInt(box.BottomRight.Y, 0, 1),
              "Error reading " + itemName + " bottom");
  test_or_die(boxReader.readInt(box.BottomRight.X, 1, 1),
              "Error reading " + itemName + " right");
  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::readTrackState(
  qtKstReader& reader, vvTrackState& state)
{
  // Read timestamp
  test_or_fail(
    this->readTimeStamp(reader, state.TimeStamp, "track trajectory", 0));

  // Read image point
  test_or_die(reader.readReal(state.ImagePoint.X, 2),
              "Error reading track trajectory image point X");
  test_or_die(reader.readReal(state.ImagePoint.Y, 3),
              "Error reading track trajectory image point Y");

  // Read image box
  test_or_fail(this->readImageBoundingBox(reader, state.ImageBox,
                                          "track trajectory image box", 4));

  // Read image object
  if (!reader.isArrayEmpty(5))
    {
    qtKstReader tableReader;
    test_or_die(reader.readTable(tableReader, 5),
                "Error reading track trajectory image object point list");
    while (!tableReader.isEndOfFile())
      {
      vvImagePointF pt;
      test_or_die(tableReader.readReal(pt.X, 0),
                  "Error reading track trajectory image object point X");
      test_or_die(tableReader.readReal(pt.Y, 1),
                  "Error reading track trajectory image object point Y");
      state.ImageObject.push_back(pt);
      tableReader.nextRecord();
      }
    }

  // Read world location
  if (!reader.isArrayEmpty(6))
    {
    qtKstReader arrayReader;
    test_or_die(reader.readArray(arrayReader, 6),
                "Error reading track trajectory world location");
    test_or_die(arrayReader.readInt(state.WorldLocation.GCS, 0),
                "Error reading track trajectory world location GCS");
    test_or_die(arrayReader.readReal(state.WorldLocation.Northing, 1),
                "Error reading track trajectory world location northing");
    test_or_die(arrayReader.readReal(state.WorldLocation.Easting, 2),
                "Error reading track trajectory world location easting");
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::readDescriptorRegion(
  qtKstReader& reader, vvDescriptor& descriptor)
{
  while (!reader.isEndOfFile())
    {
    vvDescriptorRegionEntry re;

    // Read entry time stamp
    test_or_fail(this->readTimeStamp(reader, re.TimeStamp,
                                     "descriptor region entry", 0));

    // Read entry image region
    test_or_fail(
      this->readImageBoundingBox(reader, re.ImageRegion,
                                 "descriptor region entry image region", 2));

    // Continue to next record
    descriptor.Region.insert(re);
    reader.nextRecord();
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::readGeoPoly(
  qtKstReader& reader, vgGeocodedPoly& poly, unsigned int version,
  QString* filterMode)
{
  Q_UNUSED(version); // For now there is no difference

  if (reader.isValueEmpty() || reader.isArrayEmpty())
    {
    poly.GCS = -1;
    poly.Coordinate.clear();
    filterMode && (*filterMode = "IGNORE", true);
    return true;
    }

  qtKstReader boundReader;
  test_or_die(reader.readArray(boundReader),
              "Error reading geospatial polygon");

  test_or_die(boundReader.readInt(poly.GCS, 0),
              "Error reading geospatial polygon GCS");

  // Read coordinate list
  qtKstReader polyReader;
  test_or_die(boundReader.readTable(polyReader, 1),
              "Error reading geospatial polygon coordinate list");
  poly.Coordinate.clear();
  while (!polyReader.isEndOfFile())
    {
    // Read coordinate
    vgGeoRawCoordinate coord;
    test_or_die(polyReader.readReal(coord.Northing, 0),
                "Error reading geospatial polygon coordinate northing");
    test_or_die(polyReader.readReal(coord.Easting, 1),
                "Error reading geospatial polygon coordinate easting");
    poly.Coordinate.push_back(coord);

    // Continue to next coordinate
    polyReader.nextRecord();
    }

  if (filterMode)
    {
    test_and_accept(boundReader.isEndOfRecord(2),
                    *filterMode = "CONTAINS_ANY");

    test_or_die(boundReader.readString(*filterMode, 2),
                "Error reading geospatial filter mode");
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::readQueryCommonHeader(
  qtKstReader& reader, vvDatabaseQuery& query, unsigned int version)
{
  Q_UNUSED(version); // For now there is no difference

  // Read query ID
  QString string;
  test_or_die(reader.readString(string, 0), "Error reading query ID");
  query.QueryId = stdString(string);

  // Read stream ID limit
  if (!reader.isEndOfRecord(1))
    {
    test_or_die(reader.readString(string, 1),
                "Error reading query stream ID limit");
    query.StreamIdLimit = stdString(string);
    }
  else
    {
    query.StreamIdLimit = std::string();
    }

  return reader.nextRecord();
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::readQueryCommonLimits(
  qtKstReader& reader, vvDatabaseQuery& query, unsigned int version)
{
  Q_UNUSED(version); // For now there is no difference

  // Read result limits
  if (!reader.isEndOfFile())
    {
    QString mode;
    if (!(reader.isValueEmpty(0) || reader.isArrayEmpty(0)))
      {
      qtKstReader tkst;
      test_or_die(reader.readArray(tkst, 0),
                  "Error reading query temporal limit");
      test_or_die(tkst.valueCount() >= 2 && tkst.valueCount() <= 3,
                  "Error reading query temporal limit");
      test_or_die(tkst.readLong(query.TemporalLowerLimit, 0),
                  "Error reading query temporal lower limit");
      test_or_die(tkst.readLong(query.TemporalUpperLimit, 1),
                  "Error reading query temporal lower limit");
      if (!tkst.isEndOfRecord(2))
        {
        test_or_die(tkst.readString(mode, 2),
                    "Error reading query temporal filter mode");
        test_or_fail(this->setFilterMode(query.TemporalFilter, mode));
        }
      }
    else
      {
      query.TemporalFilter = vvDatabaseQuery::Ignore;
      }

    reader.seek(reader.currentRecord(), 1);
    test_or_die(this->readGeoPoly(reader, query.SpatialLimit, 0, &mode),
                "Error reading query geospatial limit");
    if (!mode.isEmpty())
      {
      test_or_fail(this->setFilterMode(query.SpatialFilter, mode));
      }
    }

  return reader.nextRecord();
}

//-----------------------------------------------------------------------------
bool vvKstReaderPrivate::setFilterMode(
  vvDatabaseQuery::IntersectionType& out, QString in)
{
  in = in.toUpper();

  test_and_accept(in == "IGNORE",
                  out = vvDatabaseQuery::Ignore);
  test_and_accept(in == "CONTAINS_WHOLLY",
                  out = vvDatabaseQuery::ContainsWholly);
  test_and_accept(in == "CONTAINS_ANY",
                  out = vvDatabaseQuery::ContainsAny);
  test_and_accept(in == "INTERSECTS",
                  out = vvDatabaseQuery::Intersects);
  test_and_accept(in == "INTERSECTS_INBOUND",
                  out = vvDatabaseQuery::IntersectsInbound);
  test_and_accept(in == "INTERSECTS_OUTBOUND",
                  out = vvDatabaseQuery::IntersectsOutbound);
  test_and_accept(in == "DOES_NOT_CONTAIN",
                  out = vvDatabaseQuery::DoesNotContain);

  // No match
  die("Unrecognized filter mode " + in);
}

//END vvKstReaderPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvKstReader general

//-----------------------------------------------------------------------------
vvKstReader::vvKstReader() : d_ptr(new vvKstReaderPrivate)
{
}

//-----------------------------------------------------------------------------
vvKstReader::~vvKstReader()
{
}

//-----------------------------------------------------------------------------
bool vvKstReader::abort(const QString& error)
{
  QTE_D(vvKstReader);
  return d->abort(error);
}

//-----------------------------------------------------------------------------
QString vvKstReader::error()
{
  QTE_D(vvKstReader);
  return d->lastError;
}

//-----------------------------------------------------------------------------
bool vvKstReader::open(const QUrl& uri, vvReader::Format format)
{
  test_or_die(format == Auto || format == Kst,
              "Requested format not supported");

  QString error;
  test_or_die(this->open(uri, error), error);
  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReader::setInput(const QString& data, vvReader::Format format)
{
  test_or_die(format == Auto || format == Kst,
              "Requested format not supported");

  QTE_D(vvKstReader);
  d->kst.reset(new qtKstReader(data));
  if (!d->kst->isValid())
    {
    d->kst.reset();
    return this->abort("Unable to parse KST");
    }

  return true;
}

//-----------------------------------------------------------------------------
void vvKstReader::setHeader(const vvHeader& header)
{
  QTE_D(vvKstReader);
  d->header = header;
}

//-----------------------------------------------------------------------------
bool vvKstReader::rewind()
{
  QTE_D(vvKstReader);
  return (d->kst && d->kst->seek());
}

//-----------------------------------------------------------------------------
bool vvKstReader::advance()
{
  QTE_D(vvKstReader);
  return (d->kst && d->kst->nextRecord());
}

//-----------------------------------------------------------------------------
bool vvKstReader::atEnd() const
{
  QTE_D_CONST(vvKstReader);
  return (!d->kst || d->kst->isEndOfFile());
}

//END vvKstReader general

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvKstReader struct readers

//-----------------------------------------------------------------------------
bool vvKstReader::readHeader(qtKstReader& reader, vvHeader& header)
{
  // Read type
  QString type;
  test_or_die(reader.isValid() && reader.readString(type),
              "Unable to read header");

  BEGIN_MAP_FROM_STRING(vvHeader::FileType, type, header.type);
  MAP_FROM_STRING(vvHeader::TracksTag,        vvHeader::Tracks);
  MAP_FROM_STRING(vvHeader::DescriptorsTag,   vvHeader::Descriptors);
  MAP_FROM_STRING(vvHeader::QueryPlanTag,     vvHeader::QueryPlan);
  MAP_FROM_STRING(vvHeader::QueryResultsTag,  vvHeader::QueryResults);
  MAP_FROM_STRING(vvHeader::EventSetInfoTag,  vvHeader::EventSetInfo);
  // For backwards compatibility, "ALERT" is considered to be EventSetInfo
  MAP_FROM_STRING("ALERT",                    vvHeader::EventSetInfo);
  END_MAP_FROM_STRING(die("Unrecognized file format " + type));

  // Read version
  int version = 0;
  test_or_die(reader.isEndOfRecord(1) || reader.readInt(version, 1),
              "Unable to read header");
  header.version = version;

  // Advance reader
  test_or_die(reader.nextRecord(), "Unable to read header");

  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReader::readTrack(
  qtKstReader& reader, vvTrack& track, unsigned int version)
{
  // Check that we understand the version
  check_version("track", vvKstWriter::TracksVersion);

  // Read track ID
  qtKstReader tidReader;
  test_or_die(reader.readArray(tidReader, 0), "Error reading track ID");
  test_or_die(tidReader.readInt(track.Id.Source, 0),
              "Error reading track source");
  test_or_die(tidReader.readLong(track.Id.SerialNumber, 1),
              "Error reading track serial number");

  // Read track classification
  if (!reader.isArrayEmpty(1))
    {
    qtKstReader tableReader;
    test_or_die(reader.readTable(tableReader, 1),
                "Error reading track classification");
    while (!tableReader.isEndOfFile())
      {
      QString type;
      double probability;
      test_or_die(tableReader.readString(type, 0),
                  "Error reading track classification entry type");
      test_or_die(tableReader.readReal(probability, 1),
                  "Error reading track classification entry probability");
      track.Classification.insert(
        std::make_pair(stdString(type), probability));
      tableReader.nextRecord();
      }
    }

  // Read track trajectory
  if (!reader.isArrayEmpty(2))
    {
    QTE_D(vvKstReader);

    qtKstReader tableReader;
    test_or_die(reader.readTable(tableReader, 2),
                "Error reading track trajectory");
    while (!tableReader.isEndOfFile())
      {
      vvTrackState state;
      test_or_fail(d->readTrackState(tableReader, state));
      track.Trajectory.insert(state);
      tableReader.nextRecord();
      }
    }

  reader.nextRecord();
  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReader::readDescriptor(
  qtKstReader& reader, vvDescriptor& descriptor, unsigned int version)
{
  // Check that we understand the version
  check_version("descriptor", vvKstWriter::DescriptorsVersion);

  QString string;
  QList<double> array;

  // Read descriptor
  test_or_die(reader.readString(string, 0),
              "Error reading descriptor name");
  descriptor.DescriptorName = stdString(string);
  test_or_die(reader.readString(string, 1),
              "Error reading descriptor module name");
  descriptor.ModuleName = stdString(string);
  test_or_die(reader.readLong(descriptor.InstanceId, 2),
              "Error reading descriptor instance ID");
  test_or_die(reader.readReal(descriptor.Confidence, 3),
              "Error reading descriptor confidence");

  // Read values
  if (!(reader.isValueEmpty(4) || reader.isArrayEmpty(4)))
    {
    qtKstReader arrayReader;
    test_or_die(reader.readArray(arrayReader, 4),
                "Error reading descriptor values");
    reader.nextValue();
    while (!arrayReader.isEndOfRecord())
      {
      test_or_die(arrayReader.readRealArray(array),
                  "Error reading descriptor values");
      std::vector<float> a;
      foreach (double v, array)
        {
        a.push_back(static_cast<float>(v));
        }
      descriptor.Values.push_back(a);
      arrayReader.nextValue();
      }
    }

  // Read region
  if (!(reader.isValueEmpty(5) || reader.isArrayEmpty(5)))
    {
    qtKstReader tableReader;
    test_or_die(reader.readTable(tableReader, 5),
                "Error reading descriptor region");
    QTE_D(vvKstReader);
    test_or_fail(d->readDescriptorRegion(tableReader, descriptor));
    }

  // Read track ID's
  if (!(reader.isValueEmpty(6) || reader.isArrayEmpty(6)))
    {
    qtKstReader tableReader;
    test_or_die(reader.readTable(tableReader, 6),
                "Error reading descriptor tracks");
    while (!tableReader.isEndOfFile())
      {
      vvTrackId t;
      test_or_die(tableReader.readInt(t.Source, 0),
                  "Error reading descriptor track source");
      test_or_die(tableReader.readLong(t.SerialNumber, 1),
                  "Error reading descriptor track serial number");
      descriptor.TrackIds.push_back(t);
      tableReader.nextRecord();
      }
    }

  reader.nextRecord();
  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReader::readQueryPlan(
  qtKstReader& reader, vvQueryInstance& query, unsigned int version)
{
  // Check that we understand the version
  // NOTE: The query plan version was briefly changed to 2, before being
  // changed again in a way that is incompatible with that format, but forwards
  // compatible with v1 (which is again the current version). Temporarily
  // accept plans marked v2.
  check_version("query plan", 2);

  // Read query type
  QString type = "SIMILARITY";
  if (version > 0)
    {
    test_or_die(reader.readString(type, 0), "Error reading query type");
    reader.nextRecord();
    type = type.toUpper();
    }

  // Hand off to type-specific reader
  if (type == "RETRIEVAL")
    {
    vvRetrievalQuery rq;
    test_and_accept(this->readQueryPlan(reader, rq, version), query = rq);
    }
  else if (type == "SIMILARITY")
    {
    vvSimilarityQuery sq;
    test_and_accept(this->readQueryPlan(reader, sq, version), query = sq);
    }
  else
    {
    die("Unknown query type " + type);
    }

  // If we get here, reading the specific query type went wrong
  return false;
}

//-----------------------------------------------------------------------------
bool vvKstReader::readQueryPlan(
  qtKstReader& reader, vvRetrievalQuery& query, unsigned int version)
{
  // Check that we understand the version
  // NOTE: The query plan version was briefly changed to 2, before being
  // changed again in a way that is incompatible with that format, but forwards
  // compatible with v1 (which is again the current version). Temporarily
  // accept plans marked v2.
  check_version("query plan", 2);

  QTE_D(vvKstReader);

  // Read abstract query (part 1)
  test_or_fail(d->readQueryCommonHeader(reader, query, version));

  // Read query entity type
  QString entityType;
  test_or_die(reader.readString(entityType),
              "Error reading query retrieval entity type");

  BEGIN_MAP_FROM_STRING(vvRetrievalQuery::EntityType, entityType.toUpper(),
                        query.RequestedEntities);
  MAP_FROM_STRING("TRACKS", vvRetrievalQuery::Tracks);
  MAP_FROM_STRING("DESCRIPTORS", vvRetrievalQuery::Descriptors);
  MAP_FROM_STRING("ALL", vvRetrievalQuery::TracksAndDescriptors);
  END_MAP_FROM_STRING(
    die("Unrecognized retrieval query entity type " + entityType));

  // Read abstract query (part 2)
  reader.nextRecord();
  return d->readQueryCommonLimits(reader, query, version);
}

//-----------------------------------------------------------------------------
bool vvKstReader::readQueryPlan(
  qtKstReader& reader, vvSimilarityQuery& query, unsigned int version)
{
  // Check that we understand the version
  // NOTE: The query plan version was briefly changed to 2, before being
  // changed again in a way that is incompatible with that format, but forwards
  // compatible with v1 (which is again the current version). Temporarily
  // accept plans marked v2.
  check_version("query plan", 2);

  QTE_D(vvKstReader);

  // Read abstract query (part 1)
  test_or_fail(d->readQueryCommonHeader(reader, query, version));

  // Read query descriptors
  qtKstReader descriptorReader;
  test_or_die(reader.readTable(descriptorReader),
              "Error reading query descriptors");
  test_or_die(this->readDescriptors(descriptorReader, query.Descriptors, 0),
              "Error reading query descriptors");

  // Read query tracks, if present
  reader.nextValue();
  if (!reader.isEndOfRecord())
    {
    qtKstReader trackReader;
    test_or_die(reader.readTable(trackReader),
                "Error reading query tracks");
    test_or_die(this->readTracks(trackReader, query.Tracks, 0),
                "Error reading query tracks");
    }

  // Read abstract query (part 2)
  reader.nextRecord();
  test_or_fail(d->readQueryCommonLimits(reader, query, version));

  // Read similarity threshold and IQR model, if present
  if (!reader.isEndOfFile())
    {
    test_or_die(reader.readReal(query.SimilarityThreshold, 0),
                "Error reading query similarity threshold");
    if (!reader.isEndOfRecord(1))
      {
      // Read IQR model
      QString encodedModel;
      test_or_die(reader.readString(encodedModel, 1),
                  "Error reading query IQR model");
      QByteArray model = QByteArray::fromBase64(encodedModel.toLatin1());

      // Convert to std::vector<uchar>... oh, the pain...
      const uchar* rawModel =
        reinterpret_cast<const uchar*>(model.constData());
      int n = model.size();
      query.IqrModel.resize(n);
      while (n--)
        {
        query.IqrModel[n] = rawModel[n];
        }
      reader.nextRecord();
      }
    else
      {
      query.SimilarityThreshold = 0.0;
      }
    }

  // NOTE: This code for reading v2 tracks should be temporary; nothing is
  // going to write tracks in this way. (The old v2 writer code was also
  // broken; this will only read a single track.)
  if (version == 2 && !reader.isEndOfFile())
    {
    qtKstReader trackReader;
    test_or_die(reader.readTable(trackReader),
                "Error reading query tracks");
    test_or_die(this->readTracks(trackReader, query.Tracks, 0),
                "Error reading query tracks");
    reader.nextRecord();
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReader::readQueryResult(
  qtKstReader& reader, vvQueryResult& result, unsigned int version)
{
  // Check that we understand the version
  check_version("query result", vvKstWriter::QueryResultsVersion);

  int vi = 0;
  QString string;

  // Read result
  test_or_die(reader.readString(string, vi++),
              "Error reading result mission ID");
  result.MissionId = stdString(string);

  if (version > 0)
    {
    test_or_die(reader.readString(string, vi++),
                "Error reading result query ID");
    result.QueryId = stdString(string);
    test_or_die(reader.readString(string, vi++),
                "Error reading result stream ID");
    result.StreamId = stdString(string);
    test_or_die(reader.readLong(result.InstanceId, vi++),
                "Error reading result instance ID");
    }

  test_or_die(reader.readLong(result.StartTime, vi++),
              "Error reading result time range");
  test_or_die(reader.readLong(result.EndTime, vi++),
              "Error reading result time range");

  int userScore = -1;
  if (version > 1)
    {
    // Read location
    if (!reader.isArrayEmpty(vi))
      {
      qtKstReader arrayReader;
      test_or_die(reader.readArray(arrayReader, vi),
                  "Error reading result location");
      test_or_die(arrayReader.readInt(result.Location.GCS, 0),
                  "Error reading result location GCS");
      test_or_die(arrayReader.readReal(result.Location.Easting, 1),
                  "Error reading result location easting");
      test_or_die(arrayReader.readReal(result.Location.Northing, 2),
                  "Error reading result location northing");
      }
    ++vi;

    // Read score
    if (!reader.isArrayEmpty(vi))
      {
      qtKstReader arrayReader;
      test_or_die(reader.readArray(arrayReader, vi),
                  "Error reading result score");
      test_or_die(arrayReader.readLong(result.Rank, 0),
                  "Error reading result rank");
      test_or_die(arrayReader.readReal(result.RelevancyScore, 1),
                  "Error reading result relevancy");
      test_or_die(arrayReader.readInt(userScore, 2),
                  "Error reading result user classification");
      // Read user flags
      if (!arrayReader.isEndOfRecord(3))
        {
        int flags;
        test_or_die(arrayReader.readInt(flags, 3),
                    "Error reading result user flags");
        result.UserData.Flags = static_cast<vvUserData::Flag>(flags);
        }
      }
    ++vi;
    }
  else
    {
    // Read location
    test_or_die(reader.readInt(result.Location.GCS, vi++),
                "Error reading result GCS");
    test_or_die(reader.readReal(result.Location.Easting, vi++),
                "Error reading result easting");
    test_or_die(reader.readReal(result.Location.Northing, vi++),
                "Error reading result northing");

    // Read score
    test_or_die(reader.readReal(result.RelevancyScore, vi++),
                "Error reading result relevancy score");
    test_or_die(reader.readInt(userScore, vi++),
                "Error reading result user score");
    result.Rank = -1; // rank not available
    }

  // Translate user score to enum value
  switch (userScore)
    {
    case static_cast<int>(vvIqr::PositiveExample):
      result.UserScore = vvIqr::PositiveExample;
      break;
    case static_cast<int>(vvIqr::NegativeExample):
      result.UserScore = vvIqr::NegativeExample;
      break;
    default:
      result.UserScore = vvIqr::UnclassifiedExample;
      break;
    }

  // Read descriptors
  if (!(reader.isValueEmpty(vi) || reader.isArrayEmpty(vi)))
    {
    qtKstReader descriptorReader;
    test_or_die(reader.readTable(descriptorReader, vi),
                "Error reading result descriptors");
    const bool okay =
      this->readDescriptors(descriptorReader, result.Descriptors, 0);
    test_or_die(okay, "Error reading result descriptors");
    }
  ++vi;

  // Read tracks
  if (!(reader.isValueEmpty(vi) || reader.isArrayEmpty(vi)))
    {
    qtKstReader trackReader;
    test_or_die(reader.readTable(trackReader, vi),
                "Error reading result tracks");
    test_or_die(this->readTracks(trackReader, result.Tracks, 0),
                "Error reading result tracks");
    }
  ++vi;

  // Read user notes
  if (!reader.isEndOfRecord(vi))
    {
    QString notes;
    test_or_die(reader.readString(notes, vi++),
                "Error reading result user notes");
    result.UserData.Notes = stdString(notes);
    }

  reader.nextRecord();
  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReader::readGeoPoly(
  qtKstReader& reader, vgGeocodedPoly& poly, unsigned int version,
  vvDatabaseQuery::IntersectionType* filterMode)
{
  // Check that we understand the version
  check_version("geo-poly", vvKstWriter::GeoPolyVersion);

  QTE_D(vvKstReader);
  if (filterMode)
    {
    QString mode;
    test_or_fail(d->readGeoPoly(reader, poly, version, &mode));
    test_or_fail(d->setFilterMode(*filterMode, mode));
    }
  else
    {
    test_or_fail(d->readGeoPoly(reader, poly, version));
    }

  reader.nextRecord();
  return true;
}

//-----------------------------------------------------------------------------
bool vvKstReader::readEventSetInfo(
  qtKstReader& reader, vvEventSetInfo& info, unsigned int version)
{
  // Check that we understand the version
  check_version("event set info", vvKstWriter::GeoPolyVersion);

  QTE_D(vvKstReader);

  // Read name and colors
  test_or_die(reader.readString(info.Name, 0),
              "Error reading event set name");
  test_or_fail(d->readColor(reader, info.PenColor,
                            "event set pen", 1));
  test_or_fail(d->readColor(reader, info.ForegroundColor,
                            "event set foreground", 2));
  test_or_fail(d->readColor(reader, info.BackgroundColor,
                            "event set background", 3));

  // Check for threshold (will be missing in old alerts)
  if (!reader.isEndOfRecord(4))
    {
    // Read threshold
    test_or_die(reader.readReal(info.DisplayThreshold, 4),
                "Error reading event set display threshold");
    }
  else
    {
    // If no threshold present, use 0.0
    info.DisplayThreshold = 0.0;
    }

  reader.nextRecord();
  return true;
}

//END vvKstReader struct readers

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvKstReader bound data readers
#define vvKstReader_Implement_Read(_name, _type, _htype) \
  bool vvKstReader::read##_name(_type& data) \
  { \
    QTE_D(vvKstReader); \
    bool (vvKstReader::*method)(qtKstReader&, _type&, unsigned int) = \
      &vvKstReader::read##_name; \
    return d->read(data, vvHeader::_htype, *this, method); \
  }

vvKstReader_Implement_Read(Track,         vvTrack,            Tracks)
vvKstReader_Implement_Read(Descriptor,    vvDescriptor,       Descriptors)
vvKstReader_Implement_Read(QueryPlan,     vvQueryInstance,    QueryPlan)
vvKstReader_Implement_Read(QueryPlan,     vvRetrievalQuery,   QueryPlan)
vvKstReader_Implement_Read(QueryPlan,     vvSimilarityQuery,  QueryPlan)
vvKstReader_Implement_Read(QueryResult,   vvQueryResult,      QueryResults)
vvKstReader_Implement_Read(EventSetInfo,  vvEventSetInfo,     EventSetInfo)

//-----------------------------------------------------------------------------
bool vvKstReader::readHeader(vvHeader& header)
{
  QTE_D(vvKstReader);

  test_or_die(d->kst, "No data is available");
  if (this->readHeader(*d->kst, header))
    {
    d->header = header;
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vvKstReader::readGeoPoly(
  vgGeocodedPoly& poly, vvDatabaseQuery::IntersectionType* filterMode)
{
  QTE_D(vvKstReader);

  test_or_die(d->kst, "No data is available");
  return this->readGeoPoly(*d->kst, poly, d->header.version, filterMode);
}

//END vvKstReader bound data readers

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvKstReader array helpers

#define vvKstReader_Implement_ReadTypedArray(_name, _type, _list) \
  bool vvKstReader::read##_name##s( \
    qtKstReader& reader, _list<_type>& list, unsigned int version) \
  { \
    QTE_D(vvKstReader); \
    bool (vvKstReader::*method)(qtKstReader&, _type&, unsigned int) = \
      &vvKstReader::read##_name; \
    return d->read(list, reader, *this, version, method); \
  }

#define vvKstReader_Implement_ReadArray(_name, _type) \
  vvKstReader_Implement_Read(_name##s, QList<_type>, _name##s) \
  vvKstReader_Implement_Read(_name##s, std::vector<_type>, _name##s) \
  vvKstReader_Implement_ReadTypedArray(_name, _type, std::vector) \
  vvKstReader_Implement_ReadTypedArray(_name, _type, QList)

vvKstReader_Implement_ReadArray(Track, vvTrack)
vvKstReader_Implement_ReadArray(Descriptor, vvDescriptor)
vvKstReader_Implement_ReadArray(QueryResult, vvQueryResult)

//END vvKstReader array helpers
