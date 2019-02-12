/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvXmlUtil.h"

#include <QDomDocument>
#include <QStringList>

#include <qtStlUtil.h>

//BEGIN reader private helper functions

namespace // anonymous
{

#define die(_msg) do { if (error) *error = _msg; return false; } while (0)

#define test_or_fail(_cond) if (!(_cond)) return false
#define test_or_die(_cond, _msg) if (!(_cond)) die(_msg)

#define init_elem(_e, _n, _tag, _msg) \
  test_or_die(_n.isElement(), "Node is not an XML element"); \
  const QDomElement _e = _n.toElement(); \
  test_or_die(elem.tagName() == _tag, _msg)

#define read_attr(_v, _e, _a, _n) \
  test_or_fail(readAttribute(_v, _e, _a, _n, error))

#define read_attr_opt(_v, _e, _a, _n) \
  if (_e.hasAttribute(_a)) \
    test_or_fail(readAttribute(_v, _e, _a, _n, error))

#define foreach_child_element(_i, _p, _tn) \
  for(QDomElement _i = _p.firstChildElement(_tn); \
      !_i.isNull(); _i = _i.nextSiblingElement(_tn))

#define BEGIN_MAP_FROM_STRING(_type, _str, _val) \
  do { const QString _mfs_in = _str; _type &_mfs_out = _val; if (0)
#define MAP_FROM_STRING(_str, _val) \
  else if (_mfs_in == _str) _mfs_out = _val
#define END_MAP_FROM_STRING(_msg) \
  else { die(_msg); } } while (0)

//-----------------------------------------------------------------------------
bool readColorComponent(qreal& out, const QString& value)
{
  bool okay;
  if (value.endsWith('%'))
    {
    int n = value.left(value.length() - 1).toInt(&okay);
    out = qBound(0.0, n * 0.01, 1.0);
    }
  else
    {
    int n = value.toInt(&okay);
    out = qBound(0, n, 255) / 255.0;
    }

  return okay;
}

//-----------------------------------------------------------------------------
bool readColor(
  QColor& out, QString value, bool allowAlpha,
  void (QColor::*setColor)(qreal, qreal, qreal, qreal))
{
  // Extract components
  const int s = value.indexOf('(') + 1, e = value.indexOf(')');
  value = value.mid(s, e - s);

  QStringList values = value.split(',');
  test_or_fail(values.count() == (allowAlpha ? 4 : 3));

  // Parse main components
  qreal component[4];
  test_or_fail(readColorComponent(component[0], values[0]));
  test_or_fail(readColorComponent(component[1], values[1]));
  test_or_fail(readColorComponent(component[2], values[2]));

  // Parse alpha component, if applicable
  if (allowAlpha)
    {
    bool okay;
    component[3] = values[3].toDouble(&okay);
    test_or_fail(okay);
    }
  else
    {
    component[3] = 1.0;
    }

  // Set color
  (out.*setColor)(component[0], component[1], component[2], component[3]);
  return true;
}

//-----------------------------------------------------------------------------
#define vvXmlUtil_Implement_ReadAttribute(_t, _c) \
  bool readAttribute(_t& out, const QDomElement& elem, const QString& tagName, \
                     const QString& name, QString* error) \
  { \
    test_or_die(elem.hasAttribute(tagName), \
                QString("Error reading %1: no such attribute").arg(name)); \
    bool okay; \
    out = elem.attribute(tagName).to##_c(&okay); \
    test_or_die(okay, QString("Error reading %1: bad value").arg(name)); \
    return true; \
  }

vvXmlUtil_Implement_ReadAttribute(int, Int)
vvXmlUtil_Implement_ReadAttribute(uint, UInt)
vvXmlUtil_Implement_ReadAttribute(long long, LongLong)
vvXmlUtil_Implement_ReadAttribute(float, Float)
vvXmlUtil_Implement_ReadAttribute(double, Double)

//-----------------------------------------------------------------------------
bool readAttribute(
  QString& out, const QDomElement& elem, const QString& tagName,
  const QString& name, QString* error)
{
  test_or_die(elem.hasAttribute(tagName),
              QString("Error reading %1: no such attribute").arg(name));
  out = elem.attribute(tagName);
  return true;
}

//-----------------------------------------------------------------------------
bool readAttribute(
  std::string& out, const QDomElement& elem, const QString& tagName,
  const QString& name, QString* error)
{
  test_or_die(elem.hasAttribute(tagName),
              QString("Error reading %1: no such attribute").arg(name));
  out = stdString(elem.attribute(tagName));
  return true;
}

//-----------------------------------------------------------------------------
bool readAttribute(
  QColor& out, const QDomElement& elem, const QString& tagName,
  const QString& name, QString* error)
{

  test_or_die(elem.hasAttribute(tagName),
              QString("Error reading %1: no such attribute").arg(name));

  bool result = false;
  const QString value = elem.attribute(tagName);
  if (value.startsWith("rgb("))
    {
    result = readColor(out, value, false, &QColor::setRgbF);
    }
  else if (value.startsWith("rgba("))
    {
    result = readColor(out, value, true, &QColor::setRgbF);
    }
  else if (value.startsWith("hsl("))
    {
    result = readColor(out, value, false, &QColor::setHslF);
    }
  else if (value.startsWith("hsla("))
    {
    result = readColor(out, value, true, &QColor::setHslF);
    }
  else
    {
    result = QColor::isValidColor(value);
    out.setNamedColor(value);
    }

  static const char* badColorMsg =
    "Error reading %1: '%2' is not a valid color specification";
  test_or_die(result, QString(badColorMsg).arg(name, value));

  return true;
}

//-----------------------------------------------------------------------------
bool readAttribute(
  vvDatabaseQuery::IntersectionType& out, const QDomElement& elem,
  const QString& tagName, const QString& name, QString* error)
{
  test_or_die(elem.hasAttribute(tagName),
              QString("Error reading %1 filter mode: no such attribute").arg(name));

  const QString value = elem.attribute(tagName);
  test_or_die(!value.isEmpty(),
              QString("Error reading %1 filter mode: missing value").arg(name));

  BEGIN_MAP_FROM_STRING(vvDatabaseQuery::IntersectionType, value, out);
  MAP_FROM_STRING("contains_wholly",     vvDatabaseQuery::ContainsWholly);
  MAP_FROM_STRING("contains_any",        vvDatabaseQuery::ContainsAny);
  MAP_FROM_STRING("intersects",          vvDatabaseQuery::Intersects);
  MAP_FROM_STRING("intersects_inbound",  vvDatabaseQuery::IntersectsInbound);
  MAP_FROM_STRING("intersects_outbound", vvDatabaseQuery::IntersectsOutbound);
  MAP_FROM_STRING("does_not_contain",    vvDatabaseQuery::DoesNotContain);
  MAP_FROM_STRING("ignore",              vvDatabaseQuery::Ignore);
  END_MAP_FROM_STRING(
    QString("Error reading %1 filter mode:"
            " '%2' is not a valid filter mode").arg(name, value));

  return true;
}

//-----------------------------------------------------------------------------
bool readTime(vgTimeStamp& ts, const QDomElement& elem,
              const QString& name, QString* error)
{
  read_attr(ts.Time, elem, "time", name + " time");
  read_attr(ts.FrameNumber, elem, "frame_number", name + " frame number");
  return true;
}

//-----------------------------------------------------------------------------
bool readCoord(vgGeoRawCoordinate& coord, const QDomElement& elem,
               const QString& name, QString* error)
{
  if (elem.hasAttribute("latitude") && elem.hasAttribute("longitude"))
    {
    read_attr(coord.Latitude, elem, "latitude",
              name + " coordinate latitude");
    read_attr(coord.Longitude, elem, "longitude",
              name + " coordinate longitude");
    }
  else if (elem.hasAttribute("easting") && elem.hasAttribute("northing"))
    {
    read_attr(coord.Easting, elem, "easting",
              name + " coordinate easting");
    read_attr(coord.Northing, elem, "northing",
              name + " coordinate northing");
    }
  else
    {
    const QString msg =
      "Error reading %1: location specification missing or invalid";
    die(msg.arg(name));
    }

  return true;
}

//-----------------------------------------------------------------------------
bool readNode(const QDomElement& elem, vvTrackId& id, QString* error)
{
  read_attr(id.Source, elem, "source", "track source");
  read_attr(id.SerialNumber, elem, "serial_number", "serial number");
  return true;
}

//-----------------------------------------------------------------------------
bool readNode(
  const QDomElement& elem, vvDescriptorRegionEntry& re, QString* error)
{
  test_or_fail(readTime(re.TimeStamp, elem,
                        "descriptor region entry", error));
  read_attr(re.ImageRegion.TopLeft.Y, elem, "top",
            "descriptor region entry image region top");
  read_attr(re.ImageRegion.TopLeft.X, elem, "left",
            "descriptor region entry image region left");
  read_attr(re.ImageRegion.BottomRight.Y, elem, "bottom",
            "descriptor region entry image region bottom");
  read_attr(re.ImageRegion.BottomRight.X, elem, "right",
            "descriptor region entry image region right");
  return true;
}

//-----------------------------------------------------------------------------
bool readNode(
  const QDomElement& elem, vgGeocodedPoly& poly,
  vvDatabaseQuery::IntersectionType* filterMode,
  const QString& name, QString* error)
{
  poly = vgGeocodedPoly();
  filterMode && (*filterMode = vvDatabaseQuery::Ignore);

  // Read GCS
  read_attr(poly.GCS, elem, "gcs", name + " GCS");

  // Read coordinates
  foreach_child_element (pce, elem, "coordinate")
    {
    vgGeoRawCoordinate coord;
    test_or_fail(readCoord(coord, pce, name + " coordinate", error));
    poly.Coordinate.push_back(coord);
    }

  // Read filter mode, if requested
  if (filterMode)
    {
    *filterMode = vvDatabaseQuery::ContainsAny;
    read_attr_opt(*filterMode, elem, "mode", name);
    }

  // Done
  return true;
}

//-----------------------------------------------------------------------------
bool readNode(
  const QDomElement& elem, vvDatabaseQuery& query, QString* error)
{
  // Read query ID
  read_attr(query.QueryId, elem, "id", "query ID");

  // Read stream ID limit, if present
  const QDomElement silElem = elem.firstChildElement("stream_limit");
  if (!silElem.isNull())
    {
    read_attr(query.StreamIdLimit, silElem, "value",
              "query stream ID limit");
    }

  // Read temporal limit, if present
  const QDomElement tlElem = elem.firstChildElement("temporal_limit");
  if (!tlElem.isNull())
    {
    read_attr(query.TemporalLowerLimit, tlElem, "lower",
              "query temporal lower limit");
    read_attr(query.TemporalUpperLimit, tlElem, "upper",
              "query temporal upper limit");

    query.TemporalFilter = vvDatabaseQuery::ContainsAny;
    read_attr_opt(query.TemporalFilter, tlElem, "mode", "query temporal");
    }
  else
    {
    query.TemporalFilter = vvDatabaseQuery::Ignore;
    }

  // Read spatial limit, if present
  const QDomElement slElem = elem.firstChildElement("spatial_limit");
  if (!slElem.isNull())
    {
    test_or_fail(readNode(slElem, query.SpatialLimit, &query.SpatialFilter,
                          "query geospatial limit", error));
    }
  else
    {
    query.SpatialFilter = vvDatabaseQuery::Ignore;
    }


  // Done
  return true;
}

//-----------------------------------------------------------------------------
bool readNode(
  const QDomElement& elem, vvRetrievalQuery& query, QString* error)
{
  vvDatabaseQuery& abstractQuery = query;
  test_or_fail(readNode(elem, abstractQuery, error));

  // Read requested entities
  const QString etype = elem.attribute("requested_entities");
  test_or_die(!etype.isEmpty(),
              "Error reading retrieval query entity type: missing value");

  BEGIN_MAP_FROM_STRING(vvRetrievalQuery::EntityType, etype,
                        query.RequestedEntities);
  MAP_FROM_STRING("tracks",             vvRetrievalQuery::Tracks);
  MAP_FROM_STRING("descriptors",        vvRetrievalQuery::Descriptors);
  MAP_FROM_STRING("all",                vvRetrievalQuery::TracksAndDescriptors);
  MAP_FROM_STRING("tracks,descriptors", vvRetrievalQuery::TracksAndDescriptors);
  MAP_FROM_STRING("descriptors,tracks", vvRetrievalQuery::TracksAndDescriptors);
  END_MAP_FROM_STRING(
    QString("Error reading retrieval query entity type:"
            " '%1' is not a valid retrieval type").arg(etype));

  // Done
  return true;
}

//-----------------------------------------------------------------------------
bool readNode(
  const QDomElement& elem, vvSimilarityQuery& query, QString* error)
{
  vvDatabaseQuery& abstractQuery = query;
  test_or_fail(readNode(elem, abstractQuery, error));

  // Read threshold
  query.SimilarityThreshold = 0.0;
  read_attr_opt(query.SimilarityThreshold, elem, "threshold",
                "query similarity threshold");

  // Read tracks and descriptors
  foreach_child_element (qte, elem, "track")
    {
    vvTrack track;
    test_or_fail(vvXmlUtil::readNode(qte, track, error));
    query.Tracks.push_back(track);
    }
  foreach_child_element (qde, elem, "descriptor")
    {
    vvDescriptor descriptor;
    test_or_fail(vvXmlUtil::readNode(qde, descriptor, error));
    query.Descriptors.push_back(descriptor);
    }

  // Read IQR model, if present
  const QDomElement iqrElem = elem.firstChildElement("iqr_model");
  if (!iqrElem.isNull())
    {
    QString encodedModel;
    read_attr(encodedModel, iqrElem, "data", "query IQR model");
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
    }

  // Done
  return true;
}

}

//END reader private helper functions

///////////////////////////////////////////////////////////////////////////////

//BEGIN writer private helper functions

namespace // anonymous
{

//-----------------------------------------------------------------------------
long long getTime(const vgTimeStamp& ts)
{
  return static_cast<long long>(ts.Time);
}

//-----------------------------------------------------------------------------
void testFlag(vvUserData::Flags flags, vvUserData::Flag flag,
              QStringList& list, const QString& name)
{
  if (flags.testFlag(flag))
    {
    list.append(name);
    }
}

//-----------------------------------------------------------------------------
QDomElement makeNode(QDomDocument& doc, const vvTrackId& trackId)
{
  // Create track node with ID
  QDomElement elem = doc.createElement("track");
  elem.setAttribute("source", trackId.Source);
  elem.setAttribute("serial_number", trackId.SerialNumber);

  return elem;
}

//-----------------------------------------------------------------------------
QDomElement makeNode(
  QDomDocument& doc, const vgGeocodedPoly& poly, const QString& tagName)
{
  // Create node with GCS
  QDomElement gpElem = doc.createElement(tagName);
  gpElem.setAttribute("gcs", poly.GCS);

  // Add coordinate list
  for (size_t n = 0, k = poly.Coordinate.size(); n < k; ++n)
    {
    const vgGeoRawCoordinate& coord = poly.Coordinate[n];
    QDomElement ptElem = doc.createElement("coordinate");
    ptElem.setAttribute("easting", coord.Easting);
    ptElem.setAttribute("northing", coord.Northing);
    gpElem.appendChild(ptElem);
    }

  // Done
  return gpElem;
}

//-----------------------------------------------------------------------------
void setFilterAttribute(
  QDomElement& elem, vvDatabaseQuery::IntersectionType value)
{
  switch (value)
    {
    case vvDatabaseQuery::ContainsWholly:
      elem.setAttribute("mode", "contains_wholly");
      break;
    case vvDatabaseQuery::ContainsAny:
      elem.setAttribute("mode", "contains_any");
      break;
    case vvDatabaseQuery::Intersects:
      elem.setAttribute("mode", "intersects");
      break;
    case vvDatabaseQuery::IntersectsInbound:
      elem.setAttribute("mode", "intersects_inbound");
      break;
    case vvDatabaseQuery::IntersectsOutbound:
      elem.setAttribute("mode", "intersects_outbound");
      break;
    case vvDatabaseQuery::DoesNotContain:
      elem.setAttribute("mode", "does_not_contain");
      break;
    default:
      elem.setAttribute("mode", "ignore");
      break;
    }
}

//-----------------------------------------------------------------------------
QDomElement makeNode(QDomDocument& doc, const vvDatabaseQuery& query)
{
  // Create query node with ID and type
  QDomElement qpElem = doc.createElement("query");
  qpElem.setAttribute("type", "abstract");
  qpElem.setAttribute("id", qtString(query.QueryId));

  // Add stream ID limit, if set
  if (!query.StreamIdLimit.empty())
    {
    QDomElement siLimit = doc.createElement("stream_limit");
    siLimit.setAttribute("value", qtString(query.StreamIdLimit));
    qpElem.appendChild(siLimit);
    }

  // Add temporal limit, if set
  if (query.TemporalLowerLimit != -1 || query.TemporalUpperLimit != -1)
    {
    QDomElement tLimit = doc.createElement("temporal_limit");
    setFilterAttribute(tLimit, query.TemporalFilter);
    tLimit.setAttribute("lower", query.TemporalLowerLimit);
    tLimit.setAttribute("upper", query.TemporalUpperLimit);
    qpElem.appendChild(tLimit);
    }

  // Add spatial limit, if present
  if (!query.SpatialLimit.Coordinate.empty())
    {
    QDomElement sLimit = makeNode(doc, query.SpatialLimit, "spatial_limit");
    setFilterAttribute(sLimit, query.SpatialFilter);
    qpElem.appendChild(sLimit);
    }

  // Done
  return qpElem;
}

//-----------------------------------------------------------------------------
void setScore(QDomElement& elem, const QString& name, long long score)
{ if (score >= 0) elem.setAttribute(name, score); }
void setScore(QDomElement& elem, const QString& name, double score)
{ if (score >= 0.0) elem.setAttribute(name, score); }

//-----------------------------------------------------------------------------
void setColor(QDomElement& elem, const QString& name, const QColor& color)
{
  if (color.alphaF() != 1.0)
    {
    // If color has alpha component, must use 'rgba(...)' notation
    const QString value =
      QString("rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green())
                                  .arg(color.blue()).arg(color.alphaF());
    elem.setAttribute(name, value);
    }
  else
    {
    // If color has no alpha component, use hex notation
    elem.setAttribute(name, color.name());
    }
}

}

//END writer private helper functions

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvXmlUtil public reader functions

//-----------------------------------------------------------------------------
bool vvXmlUtil::readNode(
  const QDomNode& node, vvTrack& track, QString* error)
{
  init_elem(elem, node, "track", "Node is not a track");

  track = vvTrack();

  // Read track ID
  test_or_fail(::readNode(elem, track.Id, error));

  // Read track classification
  foreach_child_element (tce, elem, "classification")
    {
    QString type;
    double value;
    read_attr(type, tce, "type", "track classification entry type");
    read_attr(value, tce, "value", "track classification entry probability");
    track.Classification.insert(std::make_pair(stdString(type), value));
    }

  // Read track states
  foreach_child_element (tte, elem, "trajectory_state")
    {
    vvTrackState ts;

    // Read time stamp and image point
    test_or_fail(::readTime(ts.TimeStamp, tte, "track trajectory", error));
    read_attr(ts.ImagePoint.X, tte, "x", "track trajectory image point X");
    read_attr(ts.ImagePoint.Y, tte, "y", "track trajectory image point Y");

    // Read image box
    read_attr(ts.ImageBox.TopLeft.Y, tte, "bbox_top",
              "track trajectory image box top");
    read_attr(ts.ImageBox.TopLeft.X, tte, "bbox_left",
              "track trajectory image box left");
    read_attr(ts.ImageBox.BottomRight.Y, tte, "bbox_bottom",
              "track trajectory image box bottom");
    read_attr(ts.ImageBox.BottomRight.X, tte, "bbox_right",
              "track trajectory image box right");

    // Read world location, if set
    QDomElement wle = tte.firstChildElement("world_location");
    if (!wle.isNull())
      {
      read_attr(ts.WorldLocation.GCS, wle, "gcs",
                "track trajectory world location GCS");
      test_or_fail(readCoord(ts.WorldLocation, wle,
                             "track trajectory world location", error));
      }

    // Read image object
    foreach_child_element (ioe, tte, "image_object_point")
      {
      vvImagePointF ip;
      read_attr(ip.X, ioe, "x", "track trajectory image object point X");
      read_attr(ip.Y, ioe, "y", "track trajectory image object point Y");
      ts.ImageObject.push_back(ip);
      }

    // Add state to track
    track.Trajectory.insert(ts);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlUtil::readNode(
  const QDomNode& node, vvDescriptor& descriptor, QString* error)
{
  init_elem(elem, node, "descriptor", "Node is not a descriptor");

  descriptor = vvDescriptor();

  // Read basic descriptor attributes
  read_attr(descriptor.DescriptorName, elem, "name",
            "descriptor name");
  read_attr(descriptor.ModuleName, elem, "module",
            "descriptor module name");
  read_attr(descriptor.InstanceId, elem, "instance_id",
            "descriptor instance ID");
  read_attr(descriptor.Confidence, elem, "confidence",
            "descriptor confidence");

  // Read values
  foreach_child_element (dve, elem, "value_vector")
    {
    std::vector<float> values;
    foreach_child_element (vve, dve, "value")
      {
      float value;
      read_attr(value, vve, "data", "descriptor values");
      values.push_back(value);
      }
    descriptor.Values.push_back(values);
    }

  // Read regions
  foreach_child_element (dre, elem, "region")
    {
    vvDescriptorRegionEntry region;
    test_or_fail(::readNode(dre, region, error));
    descriptor.Region.insert(region);
    }

  // Read track references
  foreach_child_element (dte, elem, "track")
    {
    vvTrackId tid;
    test_or_fail(::readNode(dte, tid, error));
    descriptor.TrackIds.push_back(tid);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlUtil::readNode(
  const QDomNode& node, vvQueryInstance& query, QString* error)
{
  init_elem(elem, node, "query", "Node is not a query plan");

  // Get query type
  const QString type = elem.attribute("type");
  if (type == "abstract")
    {
    // Read abstract query
    query = vvQueryInstance(vvDatabaseQuery::Abstract);
    test_or_fail(::readNode(elem, *query.abstractQuery(), error));
    }
  else if (type == "retrieval")
    {
    // Read retrieval query
    query = vvQueryInstance(vvDatabaseQuery::Retrieval);
    test_or_fail(::readNode(elem, *query.retrievalQuery(), error));
    }
  else if (type == "similarity")
    {
    // Read similarity query
    query = vvQueryInstance(vvDatabaseQuery::Similarity);
    test_or_fail(::readNode(elem, *query.similarityQuery(), error));
    }
  else if (type.isEmpty())
    {
    die("Error reading query plan: query type not specified");
    }
  else
    {
    const QString msg =
      "Error reading query plan: '%1' is not a recognized query type";
    die(msg.arg(type));
    }

  // Done
  return true;
}

//-----------------------------------------------------------------------------
#define accept_flag(_str, _val) \
  if (flag == _str) { result.UserData.Flags |= _val; continue; }
bool vvXmlUtil::readNode(
  const QDomNode& node, vvQueryResult& result, QString* error)
{
  init_elem(elem, node, "query_result", "Node is not a query result");

  result = vvQueryResult();

  // Read basic result attributes
  read_attr(result.QueryId, elem, "query", "result query ID");
  read_attr(result.StreamId, elem, "stream", "result stream ID");
  read_attr(result.MissionId, elem, "mission", "result mission ID");
  read_attr(result.InstanceId, elem, "instance_id", "result instance ID");

  // Read temporal location, if present
  QDomElement tlElem = elem.firstChildElement("temporal_location");
  if (!tlElem.isNull())
    {
    read_attr(result.StartTime, tlElem, "start", "result start time");
    read_attr(result.EndTime, tlElem, "end", "result end time");
    }

  // Read spatial location, if present
  QDomElement slElem = elem.firstChildElement("spatial_location");
  if (!slElem.isNull())
    {
    read_attr(result.Location.GCS, slElem, "gcs", "result location GCS");
    test_or_fail(readCoord(result.Location, slElem,
                           "result location", error));
    }

  // Get score element
  QDomElement rsElem = elem.firstChildElement("score");
  if (!rsElem.isNull())
    {
    // Read basic score attributes
    read_attr_opt(result.Rank, rsElem, "rank", "result rank");
    read_attr_opt(result.RelevancyScore, rsElem, "relevancy",
                  "result relevancy");

    // Read user classification
    const QString rating = rsElem.attribute("rating");
    if (!rating.isEmpty())
      {
      BEGIN_MAP_FROM_STRING(vvIqr::Classification, rating, result.UserScore);
      MAP_FROM_STRING("positive",     vvIqr::PositiveExample);
      MAP_FROM_STRING("negative",     vvIqr::NegativeExample);
      MAP_FROM_STRING("unclassified", vvIqr::UnclassifiedExample);
      END_MAP_FROM_STRING(
        QString("Error reading result user classification:"
                " '%1' is not a valid classification").arg(rating));
      }

    // Read user flags
    const QStringList flags =
      rsElem.attribute("flags").split(',', QString::SkipEmptyParts);
    foreach (const QString& flag, flags)
      {
      // Test for known flags
      accept_flag("starred", vvUserData::Starred)
      // If we get here, the flag is not recognized
      const QString msg =
        "Error reading result user flags: unknown flag '%1'";
      die(msg.arg(flag));
      }
    }

  // Read user notes
  read_attr_opt(result.UserData.Notes, elem, "notes", "result user notes");

  // Read tracks and descriptors
  foreach_child_element (rte, elem, "track")
    {
    vvTrack track;
    test_or_fail(readNode(rte, track, error));
    result.Tracks.push_back(track);
    }
  foreach_child_element (rde, elem, "descriptor")
    {
    vvDescriptor descriptor;
    test_or_fail(readNode(rde, descriptor, error));
    result.Descriptors.push_back(descriptor);
    }

  // Done
  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlUtil::readNode(
  const QDomNode& node, vvEventSetInfo& info, QString* error)
{
  init_elem(elem, node, "event_meta", "Node is not event set information");

  info = vvEventSetInfo();

  // Read attributes
  read_attr(info.Name, elem, "name",
            "event set name");
  read_attr(info.DisplayThreshold, elem, "display_threshold",
            "event set display threshold");
  read_attr(info.PenColor, elem, "pen_color",
            "event set pen color");
  read_attr(info.BackgroundColor, elem, "background_color",
            "event set background color");
  read_attr(info.ForegroundColor, elem, "foreground_color",
            "event set foreground color");

  // Done
  return true;
}

//-----------------------------------------------------------------------------
bool vvXmlUtil::readNode(
  const QDomNode& node, vgGeocodedPoly& poly,
  vvDatabaseQuery::IntersectionType* filterMode, QString* error)
{
  init_elem(elem, node, "geo_poly", "Node is not a geocoded polygon");
  return ::readNode(elem, poly, filterMode, "geocoded polygon", error);
}

//END vvXmlUtil public reader functions

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvXmlUtil public writer functions

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(QDomDocument& doc, const vvTrack& track)
{
  // Create node from ID
  QDomElement tElem = ::makeNode(doc, track.Id);

  // Add track classification
  foreach_iter (vvTrackObjectClassification::const_iterator,
                tcIter, track.Classification)
    {
    QDomElement tcElem = doc.createElement("classification");
    tcElem.setAttribute("type", qtString(tcIter->first));
    tcElem.setAttribute("value", tcIter->second);
    tElem.appendChild(tcElem);
    }

  // Add track states
  foreach_iter (vvTrackTrajectory::const_iterator, ttIter, track.Trajectory)
    {
    QDomElement ttElem = doc.createElement("trajectory_state");
    ttElem.setAttribute("time", getTime(ttIter->TimeStamp));
    ttElem.setAttribute("frame_number", ttIter->TimeStamp.FrameNumber);
    ttElem.setAttribute("x", ttIter->ImagePoint.X);
    ttElem.setAttribute("y", ttIter->ImagePoint.Y);
    ttElem.setAttribute("bbox_top", ttIter->ImageBox.TopLeft.Y);
    ttElem.setAttribute("bbox_left", ttIter->ImageBox.TopLeft.X);
    ttElem.setAttribute("bbox_bottom", ttIter->ImageBox.BottomRight.Y);
    ttElem.setAttribute("bbox_right", ttIter->ImageBox.BottomRight.X);

    // Add world location, if set
    if (ttIter->WorldLocation.GCS != -1)
      {
      QDomElement wlElem = doc.createElement("world_location");
      wlElem.setAttribute("gcs", ttIter->WorldLocation.GCS);
      wlElem.setAttribute("easting", ttIter->WorldLocation.Easting);
      wlElem.setAttribute("northing", ttIter->WorldLocation.Northing);
      ttElem.appendChild(wlElem);
      }

    // Add object polygon
    foreach_iter (vvImagePolygonF::const_iterator, ioIter, ttIter->ImageObject)
      {
      QDomElement ioElem = doc.createElement("image_object_point");
      ioElem.setAttribute("x", ioIter->X);
      ioElem.setAttribute("y", ioIter->Y);
      ttElem.appendChild(ioElem);
      }

    tElem.appendChild(ttElem);
    }

  // Done
  return tElem;
}

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(
  QDomDocument& doc, const vvDescriptor& descriptor)
{
  // Create descriptor node and add basic information
  QDomElement dElem = doc.createElement("descriptor");
  dElem.setAttribute("name", qtString(descriptor.DescriptorName));
  dElem.setAttribute("module", qtString(descriptor.ModuleName));
  dElem.setAttribute("instance_id", descriptor.InstanceId);
  dElem.setAttribute("confidence", descriptor.Confidence);

  // Add values
  size_t k, i;
  for (i = 0, k = descriptor.Values.size(); i < k; ++i)
    {
    QDomElement vvElem = doc.createElement("value_vector");
    for (size_t j = 0, s = descriptor.Values[i].size(); j < s; ++j)
      {
      QDomElement vdElem = doc.createElement("value");
      vdElem.setAttribute("data", descriptor.Values[i][j]);
      vvElem.appendChild(vdElem);
      }
    dElem.appendChild(vvElem);
    }

  // Add regions
  foreach_iter (vvDescriptorRegionMap::const_iterator,
                drIter, descriptor.Region)
    {
    QDomElement drElem = doc.createElement("region");
    drElem.setAttribute("time", getTime(drIter->TimeStamp));
    drElem.setAttribute("frame_number", drIter->TimeStamp.FrameNumber);
    drElem.setAttribute("top", drIter->ImageRegion.TopLeft.Y);
    drElem.setAttribute("left", drIter->ImageRegion.TopLeft.X);
    drElem.setAttribute("bottom", drIter->ImageRegion.BottomRight.Y);
    drElem.setAttribute("right", drIter->ImageRegion.BottomRight.X);
    dElem.appendChild(drElem);
    }

  // Add track references
  for (i = 0, k = descriptor.TrackIds.size(); i < k; ++i)
    {
    dElem.appendChild(::makeNode(doc, descriptor.TrackIds[i]));
    }

  // Done
  return dElem;
}

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(QDomDocument& doc, const vvDatabaseQuery& query)
{
  return ::makeNode(doc, query);
}

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(
  QDomDocument& doc, const vvRetrievalQuery& query)
{
  // Create abstract query node and change type
  QDomElement qpElem = ::makeNode(doc, query);
  qpElem.setAttribute("type", "retrieval");

  // Add requested entity information
  switch (query.RequestedEntities)
    {
    case vvRetrievalQuery::Tracks:
      qpElem.setAttribute("requested_entities", "tracks");
      break;
    case vvRetrievalQuery::Descriptors:
      qpElem.setAttribute("requested_entities", "descriptors");
      break;
    default:
      qpElem.setAttribute("requested_entities", "tracks,descriptors");
      break;
    }

  // Done
  return qpElem;
}

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(
  QDomDocument& doc, const vvSimilarityQuery& query)
{
  // Create abstract query node, change type, and add threshold
  QDomElement qpElem = ::makeNode(doc, query);
  qpElem.setAttribute("type", "similarity");
  qpElem.setAttribute("threshold", query.SimilarityThreshold);

  // Add tracks and descriptors
  size_t k, n;
  for (n = 0, k = query.Tracks.size(); n < k; ++n)
    {
    qpElem.appendChild(makeNode(doc, query.Tracks[n]));
    }
  for (n = 0, k = query.Descriptors.size(); n < k; ++n)
    {
    qpElem.appendChild(makeNode(doc, query.Descriptors[n]));
    }

  // Add IQR model, if present
  const int iqrModelSize = static_cast<int>(query.IqrModel.size());
  if (iqrModelSize)
    {
    QDomElement iqrElem = doc.createElement("iqr_model");

    const char* rawModel =
      reinterpret_cast<const char*>(&query.IqrModel[0]);
    const QByteArray encodedModel =
      QByteArray::fromRawData(rawModel, iqrModelSize).toBase64();

    iqrElem.setAttribute("data", QString::fromLatin1(encodedModel));
    qpElem.appendChild(iqrElem);
    }

  // Done
  return qpElem;
}

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(QDomDocument& doc, const vvQueryResult& result)
{
  // Create query result node with ID and type
  QDomElement qrElem = doc.createElement("query_result");
  qrElem.setAttribute("query", qtString(result.QueryId));
  qrElem.setAttribute("stream", qtString(result.StreamId));
  qrElem.setAttribute("mission", qtString(result.MissionId));
  qrElem.setAttribute("instance_id", result.InstanceId);

  // Add temporal location, if set
  if (result.StartTime != -1 && result.EndTime != -1)
    {
    QDomElement tlElem = doc.createElement("temporal_location");
    tlElem.setAttribute("start", result.StartTime);
    tlElem.setAttribute("end", result.EndTime);
    qrElem.appendChild(tlElem);
    }

  // Add spatial location, if set
  if (result.Location.GCS != -1)
    {
    QDomElement slElem = doc.createElement("spatial_location");
    slElem.setAttribute("gcs", result.Location.GCS);
    slElem.setAttribute("easting", result.Location.Easting);
    slElem.setAttribute("northing", result.Location.Northing);
    qrElem.appendChild(slElem);
    }

  // Add score
  QDomElement rsElem = doc.createElement("score");
  setScore(rsElem, "rank", result.Rank);
  setScore(rsElem, "relevancy", result.RelevancyScore);
  switch (result.UserScore)
    {
    case vvIqr::PositiveExample:
      rsElem.setAttribute("rating", "positive");
      break;
    case vvIqr::NegativeExample:
      rsElem.setAttribute("rating", "negative");
      break;
    default:
      break;
    }

  // Add user flags
  QStringList flags;
  testFlag(result.UserData.Flags, vvUserData::Starred, flags, "starred");
  if (!flags.isEmpty())
    {
    rsElem.setAttribute("flags", flags.join(","));
    }

  // Add score node to parent
  if (rsElem.hasAttributes())
    {
    qrElem.appendChild(rsElem);
    }

  // Add user notes
  if (!result.UserData.Notes.empty())
    {
    qrElem.setAttribute("notes", qtString(result.UserData.Notes));
    }

  // Add tracks and descriptors
  size_t k, n;
  for (n = 0, k = result.Tracks.size(); n < k; ++n)
    {
    qrElem.appendChild(makeNode(doc, result.Tracks[n]));
    }
  for (n = 0, k = result.Descriptors.size(); n < k; ++n)
    {
    qrElem.appendChild(makeNode(doc, result.Descriptors[n]));
    }

  // Done
  return qrElem;
}

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(QDomDocument& doc, const vgGeocodedPoly& poly)
{
  return ::makeNode(doc, poly, "geo_poly");
}

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(QDomDocument& doc, const vvSpatialFilter& filter)
{
  QDomElement sfElem = ::makeNode(doc, filter.poly, "spatial_limit");
  if (filter.filter)
    {
    setFilterAttribute(sfElem, *filter.filter);
    }
  return sfElem;
}

//-----------------------------------------------------------------------------
QDomNode vvXmlUtil::makeNode(QDomDocument& doc, const vvEventSetInfo& info)
{
  QDomElement esiElem = doc.createElement("event_meta");

  esiElem.setAttribute("name", info.Name);
  esiElem.setAttribute("display_threshold", info.DisplayThreshold);
  setColor(esiElem, "pen_color", info.PenColor);
  setColor(esiElem, "background_color", info.BackgroundColor);
  setColor(esiElem, "foreground_color", info.ForegroundColor);

  return esiElem;
}

//END vvXmlUtil public writer functions
