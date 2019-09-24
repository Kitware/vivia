/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTrackOracleEventDataSource.h"

#include "visgui_event_type.h"

#include <vdfEventSource.h>

#include <qtStlUtil.h>

#include <QDebug>

#include <track_oracle/file_formats/file_format_base.h>

namespace // anonymous
{

//-----------------------------------------------------------------------------
QString fieldName(const track_oracle::track_field_base& field)
{
  return qtString(field.get_field_name());
}

//-----------------------------------------------------------------------------
vgTimeStamp adapt(const kwiver::vital::timestamp& ts)
{
  vgTimeStamp out;
  if (ts.has_valid_frame())
  {
    out.FrameNumber = static_cast<unsigned>(ts.get_frame());
  }
  if (ts.has_valid_time())
  {
    out.Time = ts.get_time_seconds();
  }

  return out;
}

//-----------------------------------------------------------------------------
vgTimeStamp startTime(const vvEvent& event)
{
  vgTimeStamp out;

  for (const auto& i : event.TrackIntervals)
  {
    if (!out.IsValid() || i.Start < out)
    {
      out = i.Start;
    }
  }

  return out;
}

//-----------------------------------------------------------------------------
vgTimeStamp stopTime(const vvEvent& event)
{
  vgTimeStamp out;

  for (const auto& i : event.TrackIntervals)
  {
    if (!out.IsValid() || i.Stop > out)
    {
      out = i.Stop;
    }
  }

  return out;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vdfTrackOracleEventDataSourcePrivate
{
public:
  vdfTrackOracleEventDataSourcePrivate(
    vdfTrackOracleEventDataSource* q,
    track_oracle::file_format_base* format);

  track_oracle::file_format_base* const Format;
  vdfEventSource* const EventSourceInterface;
};

QTE_IMPLEMENT_D_FUNC(vdfTrackOracleEventDataSource)

//-----------------------------------------------------------------------------
vdfTrackOracleEventDataSourcePrivate::vdfTrackOracleEventDataSourcePrivate(
  vdfTrackOracleEventDataSource* q, track_oracle::file_format_base* format)
  : Format{format}, EventSourceInterface{q->addInterface<vdfEventSource>()}
{
}

//-----------------------------------------------------------------------------
vdfTrackOracleEventDataSource::vdfTrackOracleEventDataSource(
  const QUrl& uri, track_oracle::file_format_base* format, QObject* parent)
  : vdfThreadedArchiveSource{uri, parent},
    d_ptr{new vdfTrackOracleEventDataSourcePrivate{this, format}}
{
}

//-----------------------------------------------------------------------------
vdfTrackOracleEventDataSource::~vdfTrackOracleEventDataSource()
{
}

//-----------------------------------------------------------------------------
bool vdfTrackOracleEventDataSource::processArchive(const QUrl& uri)
{
  QTE_D();

  const auto fileName = stdString(uri.toLocalFile());

  // Read events
  const visgui_event_type schema;
  track_oracle::track_handle_list_type events;
  std::vector<track_oracle::element_descriptor> missingFieldDescriptors;

  if (!d->Format->read(fileName, events, schema, missingFieldDescriptors))
  {
    qWarning() << "error reading events from" << uri;
    return false;
  }

  // Are we missing any fields?
  if (missingFieldDescriptors.size() > 0)
  {
    // Build set of missing fields
    QSet<QString> missingFields;
    for (const auto& f : missingFieldDescriptors)
    {
      missingFields.insert(qtString(f.name));
    }

    // Remove fields that are allowed to be missing
    // NOTE: It is okay for these fields to be missing; we can synthesize their
    //       values from the track intervals
    missingFields.remove(fieldName(schema.event_start));
    missingFields.remove(fieldName(schema.event_stop));

    // Check if any mandatory fields are missing
    if (!missingFields.empty())
    {
      qWarning() << "unable to read events from" << uri
                 << "due to missing fields" << missingFields;
      return false;
    }
  }

  // Process the events
  visgui_event_type oracle;
  bool good = false, error = false;
  for (const auto& eventHandle : events)
  {
    if (eventHandle.is_valid())
    {
      // Set oracle object to track instance referenced by handle
      oracle(eventHandle);

      // Create temporary event
      vvEvent out;
      out.Id = oracle.event_id();
      out.Classification = oracle.event_labels();
      if (out.Classification.empty())
      {
        error = true;
        qWarning() << "ignoring event" << out.Id << "with no labels";
        continue;
      }

      // Extract actor/track intervals
      for (const auto& ti : oracle.actor_intervals())
      {
        const auto tl = adapt(ti.start);
        const auto tu = adapt(ti.stop);
        if (!tl.IsValid() && !tu.IsValid())
        {
          error = true;
          qWarning() << "ignoring track" << ti.track << "interval for event"
                     << out.Id << "with bad temporal extents";
        }
        else
        {
          out.TrackIntervals.push_back({vvTrackId{-1, ti.track}, tl, tu});
        }
      }

      // Extract start time
      if (oracle.event_start.exists())
      {
        out.Start = adapt(oracle.event_start());
      }
      else
      {
        out.Start = startTime(out);
      }

      // Extract stop time
      if (oracle.event_start.exists())
      {
        out.Stop = adapt(oracle.event_stop());
      }
      else
      {
        out.Stop = stopTime(out);
      }

      if (out.TrackIntervals.empty() &&
          !out.Start.IsValid() && !out.Stop.IsValid())
      {
        error = true;
        qWarning()
          << "ignoring event " << out.Id
          << "with no temporal locality";
        continue;
      }

      // Emit event
      good = true;
      emit d->EventSourceInterface->eventsAvailable({{out}});
    }
  }

  // Done; test if we got any usable data and (unless the file simply had no
  // data) spit out a warning otherwise
  if (error && !good)
  {
    qWarning().nospace()
      << "error reading events from " << uri
      << ": none of the oracle tracks contained usable data";
    return false;
  }
  return true;
}
