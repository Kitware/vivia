/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTrackOracleTrackDataSource.h"

#include <visgui_track_type.h>

#include <vdfTrackSource.h>

#include <vgGeodesy.h>

#include <qtEnumerate.h>
#include <qtStlUtil.h>

#include <QDebug>
#include <QSet>
#include <QUuid>

#ifdef KWIVER_TRACK_ORACLE
  #include <track_oracle/file_formats/file_format_base.h>
#else
  #include <track_oracle/file_format_base.h>
#endif

#ifndef KWIVER_TRACK_ORACLE
namespace track_oracle
{

using track_oracle_core = ::vidtk::track_oracle;

} // namespace track_oracle
#endif

namespace // anonymous
{

#ifndef KWIVER_TRACK_ORACLE

//-----------------------------------------------------------------------------
QUuid qtUuid(const boost::uuids::uuid& in)
{
  const QByteArray bytes{reinterpret_cast<const char*>(in.data), 16};
  return QUuid::fromRfc4122(bytes);
}

#else

//-----------------------------------------------------------------------------
QUuid qtUuid(const kwiver::vital::uid& in)
{
  const auto& bytes = qtBytes(in.value());
  const auto result = QUuid{bytes};
  if (!result.isNull())
  {
    return result;
  }

  // TODO build UUID from hash of input?
  return result;
}

#endif

//-----------------------------------------------------------------------------
QString fieldName(const track_oracle::track_field_base& field)
{
  return qtString(field.get_field_name());
}

} // namespace <anonymous>

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
  vdfTrackOracleTrackDataSourcePrivate::FileFormat* format)
  : Format{format},
    TrackSourceInterface{q->addInterface<vdfTrackSource>()}
{
}

//-----------------------------------------------------------------------------
vdfTrackOracleTrackDataSource::vdfTrackOracleTrackDataSource(
  const QUrl& uri, track_oracle::file_format_base* format, QObject* parent)
  : vdfThreadedArchiveSource{uri, parent},
    d_ptr{new vdfTrackOracleTrackDataSourcePrivate{this, format}}
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
    missingFields.remove(fieldName(schema.track_uuid));
    missingFields.remove(fieldName(schema.timestamp_usecs));
    missingFields.remove(fieldName(schema.frame_number));
    missingFields.remove(fieldName(schema.bounding_box));
    missingFields.remove(fieldName(schema.obj_location));
    missingFields.remove(fieldName(schema.world_location));
    missingFields.remove(fieldName(schema.world_gcs));
    missingFields.remove(fieldName(schema.state_flags));

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
      const QUuid uuid =
        (oracle.track_uuid.exists() ? qtUuid(oracle.track_uuid()) : QUuid());
      vdfTrackId id(this, oracle.external_id(), uuid);

      // Convert track states
      QList<vvTrackState> states;
      vgTimeMap<vdfTrackAttributes> attrs;
      vdfTrackScalarDataCollection data;

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
            state.ImageObject.emplace_back(
              box.TopLeft.X, box.TopLeft.Y);
            state.ImageObject.emplace_back(
              box.BottomRight.X, box.TopLeft.Y);
            state.ImageObject.emplace_back(
              box.BottomRight.X, box.BottomRight.Y);
            state.ImageObject.emplace_back(
              box.TopLeft.X, box.BottomRight.Y);
          }

          if (oracle.obj_location.exists())
          {
            // Extract image location
            const auto& vglPoint = oracle.obj_location();
            state.ImagePoint.X = vglPoint.x();
            state.ImagePoint.Y = vglPoint.y();
          }

          if (oracle.world_location.exists())
          {
            // Extract world location
            const auto& p = oracle.world_location();
            state.WorldLocation =
              vgGeocodedCoordinate{p.y(), p.x(), vgGeodesy::LatLon_Wgs84};
            if (oracle.world_gcs.exists() && oracle.world_gcs() > 0)
            {
              state.WorldLocation.GCS = oracle.world_gcs();
            }
          }

          if (oracle.state_flags.exists())
          {
            auto ai = oracle.state_flags().get_flags();
            if (!ai.empty())
            {
              vdfTrackAttributes& ao =
                *attrs.insert(state.TimeStamp, vdfTrackAttributes());

              for (auto iter : qtEnumerate(ai))
              {
                // Distinguish between old-style attributes and new-style
                // key/value pairs: if the value is empty, it's an old-style
                // attribute
                if (iter->second.empty())
                {
                  ao.insert(qtString(iter->first));
                }
              }
            }
          }

          // Add state
          states.append(state);
        }
      }

      // Emit track (but skip empty tracks)
      if (!states.isEmpty())
      {
        good = true;
        emit d->TrackSourceInterface->trackUpdated(id, states, attrs, data);
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
