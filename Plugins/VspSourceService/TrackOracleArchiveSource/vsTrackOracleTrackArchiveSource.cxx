// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsTrackOracleTrackArchiveSource.h"

#include <QDebug>
#include <QSet>

#include <qtThread.h>
#include <qtStlUtil.h>

#include <vgGeodesy.h>

#ifdef KWIVER_TRACK_ORACLE
#include <track_oracle/file_formats/file_format_base.h>
#else
#include <track_oracle/file_format_base.h>
#endif

#include <vsTrackData.h>

#include <vsAdapt.h>
#include <vsArchiveSourcePrivate.h>

#include "visgui_track_type.h"

#ifndef KWIVER_TRACK_ORACLE
namespace track_oracle
{
  using track_oracle_core = ::vidtk::track_oracle;
}
#endif

namespace
{

//-----------------------------------------------------------------------------
QString fieldName(const track_oracle::track_field_base& field)
{
  return qtString(field.get_field_name());
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vsTrackOracleTrackArchiveSourcePrivate : public vsArchiveSourcePrivate
{
public:
  typedef track_oracle::file_format_base FileFormat;

  vsTrackOracleTrackArchiveSourcePrivate(
    vsTrackOracleTrackArchiveSource* q, const QUrl& uri, FileFormat* format);

protected:
  QTE_DECLARE_PUBLIC(vsTrackOracleTrackArchiveSource)

  virtual bool processArchive(const QUrl& uri) QTE_OVERRIDE;

  FileFormat* Format;
};

QTE_IMPLEMENT_D_FUNC(vsTrackOracleTrackArchiveSource)

//-----------------------------------------------------------------------------
vsTrackOracleTrackArchiveSourcePrivate::vsTrackOracleTrackArchiveSourcePrivate(
  vsTrackOracleTrackArchiveSource* q, const QUrl& uri, FileFormat* format)
  : vsArchiveSourcePrivate(q, "tracks", uri), Format(format)
{
}

//-----------------------------------------------------------------------------
bool vsTrackOracleTrackArchiveSourcePrivate::processArchive(const QUrl& uri)
{
  QTE_Q(vsTrackOracleTrackArchiveSource);

  const std::string fileName = stdString(uri.toLocalFile());

  // Read tracks
  const visgui_track_type schema;
  track_oracle::track_handle_list_type tracks;
  std::vector<track_oracle::element_descriptor> missingFieldDescriptors;

  if (!this->Format->read(fileName, tracks, schema, missingFieldDescriptors))
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

    // Check if missing field(s) is/are mandatory; we are okay without frame
    // number
    // TODO Allow missing time value once core supports time mapping
    if (missingFields.count() > 1 ||
        *missingFields.begin() != fieldName(schema.frame_number))
      {
      qWarning() << "unable to load tracks from" << uri
                 << "due to missing fields" << missingFields;
      return false;
      }
    }

  // Process the tracks
  visgui_track_type oracle;
  for (size_t n = 0, k = tracks.size(); n < k; ++n)
    {
    const track_oracle::track_handle_type& trackHandle = tracks[n];
    if (trackHandle.is_valid())
      {
      // Set oracle object to track instance referenced by handle
      oracle(trackHandle);

      // Get track ID
      vsTrackId id = vsAdaptTrackId(oracle.external_id());

      // Convert track states
      QList<vvTrackState> states;
      vsTrackData data;
      auto frameHandles =
        track_oracle::track_oracle_core::get_frames(trackHandle);
      for (size_t n = 0, k = frameHandles.size(); n < k; ++n)
        {
        auto const& frameHandle = frameHandles[n];
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
            continue;
            }

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

          // Extract image location
          const vgl_point_2d<double>& vglPoint = oracle.obj_location();
          state.ImagePoint.X = vglPoint.x();
          state.ImagePoint.Y = vglPoint.y();

          // Add state
          states.append(state);
          }
        }

      // Emit track (but skip empty tracks)
      if (!states.isEmpty())
        {
        emit q->trackUpdated(id, states);
        emit q->trackDataUpdated(id, data);
        emit q->trackClosed(id);
        }
      }
    }

  // Done
  return true;
}

//-----------------------------------------------------------------------------
vsTrackOracleTrackArchiveSource::vsTrackOracleTrackArchiveSource(
  const QUrl& uri, track_oracle::file_format_base* format) :
  super(new vsTrackOracleTrackArchiveSourcePrivate(this, uri, format))
{
}

//-----------------------------------------------------------------------------
vsTrackOracleTrackArchiveSource::~vsTrackOracleTrackArchiveSource()
{
}
