/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTrackOracleTrackArchiveSource.h"

#include "visgui_track_type.h"

#include <vdfTrackSource.h>

#include <vgGeodesy.h>

#include <qtStlUtil.h>

#include <QDebug>
#include <QSet>
#include <QUuid>

#if QT_VERSION < 0x040800
#include <QtEndian>
#endif

#ifdef KWIVER_TRACK_ORACLE
#include <track_oracle/file_formats/file_format_base.h>
#else
#include <track_oracle/file_format_base.h>
#endif

#include <boost/lexical_cast.hpp>

#ifndef KWIVER_TRACK_ORACLE
namespace track_oracle
{
  using track_oracle_core = ::vidtk::track_oracle;
}

namespace // anonymous
{

//-----------------------------------------------------------------------------
QUuid qtUuid(const boost::uuids::uuid& in)
{
#if QT_VERSION >= 0x040800
  const QByteArray bytes(reinterpret_cast<const char*>(in.data), 16);
  return QUuid::fromRfc4122(bytes);
#else
  const uchar* const data = in.data;
  const uint l = qFromBigEndian<uint>(data + 0);
  const ushort w1 = qFromBigEndian<ushort>(data + 4);
  const ushort w2 = qFromBigEndian<ushort>(data + 6);

  return QUuid(l, w1, w2, data[8], data[9], data[10], data[11],
               data[12], data[13], data[14], data[15]);
#endif
}

} // namespace <anonymous>
#endif

//-----------------------------------------------------------------------------
class vdfTrackOracleTrackDataSourcePrivate
{
public:
  typedef track_oracle::file_format_base FileFormat;

public:
  vdfTrackOracleTrackDataSourcePrivate(
    vdfTrackOracleTrackDataSource* q, FileFormat* format);

  FileFormat* const Format;
  vdfTrackSource* const TrackSourceInterface;
};

QTE_IMPLEMENT_D_FUNC(vdfTrackOracleTrackDataSource)

//-----------------------------------------------------------------------------
vdfTrackOracleTrackDataSourcePrivate::vdfTrackOracleTrackDataSourcePrivate(
  vdfTrackOracleTrackDataSource* q,
  vdfTrackOracleTrackDataSourcePrivate::FileFormat* format) :
  Format(format),
  TrackSourceInterface(q->addInterface<vdfTrackSource>())
{
}

//-----------------------------------------------------------------------------
vdfTrackOracleTrackDataSource::vdfTrackOracleTrackDataSource(
  const QUrl& uri, track_oracle::file_format_base* format, QObject* parent) :
  vdfThreadedArchiveSource(uri, parent),
  d_ptr(new vdfTrackOracleTrackDataSourcePrivate(this, format))
{
}

//-----------------------------------------------------------------------------
vdfTrackOracleTrackDataSource::~vdfTrackOracleTrackDataSource()
{
}

//-----------------------------------------------------------------------------
bool vdfTrackOracleTrackDataSource::processArchive(const QUrl& uri)
{
  QTE_D(vdfTrackOracleTrackDataSource);

  const std::string fileName = stdString(uri.toLocalFile());

  // Read tracks
  const visgui_track_type schema;
  track_oracle::track_handle_list_type tracks;
  std::vector<track_oracle::element_descriptor> missingFieldDescriptors;

  if (!d->Format->read(fileName, tracks, schema, missingFieldDescriptors))
    {
    qWarning() << "error reading tracks from" << uri;
    return false;
    }

  // Are we missing any fields?
  if (missingFieldDescriptors.size() > 0)
    {
    // Build set of missing fields
    QSet<QString> missingFields;
    for (size_t n = 0, k = missingFieldDescriptors.size(); n < k; ++n)
      {
      missingFields.insert(qtString(missingFieldDescriptors[n].name));
      }

    // Remove fields that are allowed to be missing
    // NOTE: It is okay for any one of these fields to be missing, but some
    //       combinations of missing fields will result in an invalid file;
    //       since track oracle doesn't guarantee that the file format having
    //       fields means that every frame will have the same fields, we need
    //       to (and do) test this at the frame level anyway, so it's not
    //       crucial for this to detect such cases
    missingFields.remove("unique_id");
    missingFields.remove("timestamp_usecs");
    missingFields.remove("frame_number");
    missingFields.remove("bounding_box");
    missingFields.remove("obj_location");
    missingFields.remove("world_location");

    // Check if any mandatory fields are missing
    if (!missingFields.empty())
      {
      qWarning() << "unable to read tracks from" << uri
                 << "due to missing fields" << missingFields;
      return false;
      }
    }

  // Process the tracks
  visgui_track_type oracle;
  bool good = false, error = false;
  for (size_t n = 0, k = tracks.size(); n < k; ++n)
    {
    const auto& trackHandle = tracks[n];
    if (trackHandle.is_valid())
      {
      // Set oracle object to track instance referenced by handle
      oracle(trackHandle);

      // Get track ID
#ifdef KWIVER_TRACK_ORACLE
      // TODO: Remove this when UUID's are supported by KWIVER's track_oracle
      const QUuid uuid;
#else
      const QUuid uuid =
        (oracle.unique_id.exists() ? qtUuid(oracle.unique_id()) : QUuid());
#endif
      vdfTrackId id(this, oracle.external_id(), uuid);

      // Convert track states
      QList<vvTrackState> states;
      auto frameHandles =
        track_oracle::track_oracle_core::get_frames(trackHandle);
      for (size_t n = 0, k = frameHandles.size(); n < k; ++n)
        {
        const auto& frameHandle = frameHandles[n];
        if (frameHandle.is_valid())
          {
          vvTrackState state;

          // Set oracle object to frame instance referenced by handle
          oracle[frameHandle];

          // Extract timestamp
          if (oracle.timestamp_usecs.exists())
            {
            state.TimeStamp.Time = oracle.timestamp_usecs();
            }
          if (oracle.frame_number.exists())
            {
            state.TimeStamp.FrameNumber = oracle.frame_number();
            }
          if (!state.TimeStamp.IsValid())
            {
            // Don't add state if timestamp is not valid
            error = true;
            continue;
            }

          if (!oracle.bounding_box.exists() && !oracle.obj_location.exists())
            {
            // Don't add state if it has no image location information
            error = true;
            continue;
            }

          if (oracle.bounding_box.exists())
            {
            // Extract bounding box
            const vgl_box_2d<double>& vglBox = oracle.bounding_box();
            vvImageBoundingBox& box = state.ImageBox;
            box.TopLeft.X = vglBox.min_x();
            box.TopLeft.Y = vglBox.min_y();
            box.BottomRight.X = vglBox.max_x();
            box.BottomRight.Y = vglBox.max_y();

            // Copy bounding box to object
            state.ImageObject.push_back(
              vvImagePointF(box.TopLeft.X, box.TopLeft.Y));
            state.ImageObject.push_back(
              vvImagePointF(box.BottomRight.X, box.TopLeft.Y));
            state.ImageObject.push_back(
              vvImagePointF(box.BottomRight.X, box.BottomRight.Y));
            state.ImageObject.push_back(
              vvImagePointF(box.TopLeft.X, box.BottomRight.Y));
            }

          if (oracle.obj_location.exists())
            {
            // Extract image location
            const vgl_point_2d<double>& vglPoint = oracle.obj_location();
            state.ImagePoint.X = vglPoint.x();
            state.ImagePoint.Y = vglPoint.y();
            }

          if (oracle.world_location.exists())
            {
            // Extract world location
            const vgl_point_3d<double>& p = oracle.world_location();
            state.WorldLocation =
              vgGeocodedCoordinate(p.y(), p.x(), vgGeodesy::LatLon_Wgs84);
            }

          // Add state
          states.append(state);
          }
        }

      // Emit track (but skip empty tracks)
      if (!states.isEmpty())
        {
        good = true;
        emit d->TrackSourceInterface->trackUpdated(id, states);
        emit d->TrackSourceInterface->trackClosed(id);
        }
      }
    }

  // Done; test if we got any usable data and (unless the file simply had no
  // data) spit out a warning otherwise
  if (error && !good)
    {
    qWarning().nospace()
      << "error reading tracks from " << uri
      << ": none of the oracle frames contained usable data";
    return false;
    }
  return true;
}
