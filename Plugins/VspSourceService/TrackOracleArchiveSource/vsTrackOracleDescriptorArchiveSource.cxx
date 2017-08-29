/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackOracleDescriptorArchiveSource.h"

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

#include <vsTrackId.h>
#include <vtkVsTrackInfo.h>

#include <vsAdapt.h>
#include <vsArchiveSourcePrivate.h>

#include "visgui_event_type.h"

#ifndef KWIVER_TRACK_ORACLE
namespace track_oracle
{
  using track_oracle_core = track_oracle;
}
#endif

//-----------------------------------------------------------------------------
class vsTrackOracleDescriptorArchiveSourcePrivate :
  public vsArchiveSourcePrivate
{
public:
  typedef track_oracle::file_format_base FileFormat;

  vsTrackOracleDescriptorArchiveSourcePrivate(
    vsTrackOracleDescriptorArchiveSource* q, const QUrl& uri,
    FileFormat* format);

  bool extractPvo(const track_oracle::track_handle_type&);
  bool extractEvent(const track_oracle::track_handle_type&);

protected:
  QTE_DECLARE_PUBLIC(vsTrackOracleDescriptorArchiveSource)

  virtual bool processArchive(const QUrl& uri) QTE_OVERRIDE;

  FileFormat* Format;
  vtkIdType LastEventId;
  bool UsingInternalIds;
};

QTE_IMPLEMENT_D_FUNC(vsTrackOracleDescriptorArchiveSource)

//-----------------------------------------------------------------------------
vsTrackOracleDescriptorArchiveSourcePrivate::
vsTrackOracleDescriptorArchiveSourcePrivate(
  vsTrackOracleDescriptorArchiveSource* q,
  const QUrl& uri, FileFormat* format) :
  vsArchiveSourcePrivate(q, "descriptors", uri), Format(format),
  LastEventId(0), UsingInternalIds(false)
{
}

//-----------------------------------------------------------------------------
bool vsTrackOracleDescriptorArchiveSourcePrivate::extractPvo(
  const track_oracle::track_handle_type& trackHandle)
{
  // Attempt to extract P/V/O track object classifier
  visgui_pvo_descriptor_type pvo;
  pvo(trackHandle);
  if (pvo.source_track_ids.exists() &&
      pvo.descriptor_pvo_raw_scores.exists() &&
      pvo.descriptor_pvo_raw_scores().size() >= 3)
    {
    QTE_Q(vsTrackOracleDescriptorArchiveSource);

    const std::vector<unsigned int>& tracks = pvo.source_track_ids();
    for (size_t n = 0, k = tracks.size(); n < k; ++n)
      {
      vsTrackObjectClassifier toc;
      toc.probabilityFish  = pvo.descriptor_pvo_raw_scores()[0];
      toc.probabilityScallop = pvo.descriptor_pvo_raw_scores()[1];
      toc.probabilityOther   = pvo.descriptor_pvo_raw_scores()[2];

      emit q->tocAvailable(vsAdaptTrackId(tracks[n]), toc);
      return true;
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vsTrackOracleDescriptorArchiveSourcePrivate::extractEvent(
  const track_oracle::track_handle_type& trackHandle)
{
  vsEvent event = vsEvent(QUuid() /* FIXME: get from track_oracle object */);
  bool eventIsValid = false;
  bool isClassifierEvent = false;

  // Attempt to extract event
  visgui_generic_event_type oracle;
  oracle(trackHandle);

  // Determine event ID (not guaranteed to provide one)
  if (this->UsingInternalIds)
    {
    const vtkIdType id = ++this->LastEventId;
    if(oracle.external_id.exists())
      {
      qDebug() << "assigning generated ID" << id
               << "to event with external ID" << oracle.external_id();
      }
    event->SetId(id);
    }
  else if(oracle.external_id.exists())
    {
    const vtkIdType id = static_cast<vtkIdType>(oracle.external_id());
    this->LastEventId = qMax(id, this->LastEventId);
    event->SetId(id);
    }
  else
    {
    if (this->LastEventId)
      {
      // Oh, dear... some events have ID's, and some don't...
      qWarning() << "event ID missing after previous events using external ID";
      qWarning() << "forcing remaining event(s) to use generated ID's";
      }
    this->UsingInternalIds = true;
    event->SetId(++this->LastEventId);
    }

  // Extract classifiers, if present
  if (oracle.descriptor_classifier.exists())
    {
    const std::vector<double>& classifierValues =
      oracle.descriptor_classifier();
    for (size_t n = 0, k = classifierValues.size(); n < k; ++n)
      {
      const double p = classifierValues[n];
      if (p > 0.0)
        {
        event->AddClassifier(static_cast<int>(n), p, 0.0);
        }
      }

    eventIsValid = true;
    isClassifierEvent = true;
    }

  // Extract note, if present
  if (oracle.augmented_annotation.exists())
    {
    event->SetNote(oracle.augmented_annotation().c_str());
    eventIsValid = true;
    }
  else if (oracle.basic_annotation.exists())
    {
    event->SetNote(oracle.basic_annotation().c_str());
    eventIsValid = true;
    }

  // Don't proceed unless event had either a note and/or type classifier
  if (!eventIsValid)
    {
    qWarning() << "ignoring event with neither a classifier descriptor"
               << "nor an annotation";
    return false;
    }

  if (!isClassifierEvent)
    {
    // Event is not a classifier event, but has a note; must be an annotation
    event->AddClassifier(vsEventInfo::Annotation, 1.0, 0.0);
    }

  // Get event frame-independent start and end time
  vtkVgTimeStamp eventStart;
  vtkVgTimeStamp eventEnd;

  if (oracle.start_time_secs.exists())
    {
    eventStart.SetTime(oracle.start_time_secs() * 1e6);
    event->SetStartFrame(eventStart);
    }
  if (oracle.end_time_secs.exists())
    {
    eventEnd.SetTime(oracle.end_time_secs() * 1e6);
    event->SetEndFrame(eventEnd);
    }

  // Check if start and end time are set; if not, event must have valid frames
  // or it will be rejected
  eventIsValid = (eventStart.IsValid() && eventEnd.IsValid());

  // Extract event regions
  auto const& frameHandles =
    track_oracle::track_oracle_core::get_frames(trackHandle);
  for (size_t n = 0, k = frameHandles.size(); n < k; ++n)
    {
    auto const& frameHandle = frameHandles[n];
    if (frameHandle.is_valid())
      {
      // Set oracle object to frame instance referenced by handle
      oracle[frameHandle];

      // Extract timestamp
      vtkVgTimeStamp ts;
      if (oracle.timestamp_usecs.exists())
        {
        ts.SetTime(oracle.timestamp_usecs());
        }
      if (oracle.frame_number.exists())
        {
        ts.SetFrameNumber(oracle.frame_number());
        }

      // Don't add region if timestamp is not valid, or box is missing
      if (!ts.IsValid() || !oracle.bounding_box.exists())
        {
        continue;
        }

      // Update event temporal bounds
      eventStart = (eventIsValid && ts >= eventStart ? eventStart : ts);
      eventEnd   = (eventIsValid && ts <= eventEnd   ? eventEnd   : ts);

      // Extract bounding box
      const vgl_box_2d<double>& box = oracle.bounding_box();

      double points[8] =
        {
        box.min_x(), box.min_y(),
        box.max_x(), box.min_y(),
        box.max_x(), box.max_y(),
        box.min_x(), box.max_y()
        };

      event->AddRegion(ts, 4, points);
      eventIsValid = true;
      }
    }

  if (!eventIsValid)
    {
    qWarning() << "ignoring event with empty region list"
               << "and unspecified start/end time";
    return false;
    }

  // Add track(s)
  oracle(trackHandle);
  const std::vector<unsigned int> tracks = oracle.source_track_ids();
  for (size_t n = 0, k = tracks.size(); n < k; ++n)
    {
    vtkVsTrackInfo* ti = new vtkVsTrackInfo(vsAdaptTrackId(tracks[n]),
                                            eventStart, eventEnd);
    event->AddTrack(ti);
    }

  // Emit the event
  QTE_Q(vsTrackOracleDescriptorArchiveSource);
  q->emitEvent(event);
  return true;
}

//-----------------------------------------------------------------------------
bool vsTrackOracleDescriptorArchiveSourcePrivate::processArchive(
  const QUrl& uri)
{
  const std::string fileName = stdString(uri.toLocalFile());

  // Read descriptors
  track_oracle::track_handle_list_type descriptors;
  if (!this->Format->read(fileName, descriptors))
    {
    qWarning() << "error reading descriptors from" << uri;
    return false;
    }

  // Process the descriptors
  bool dataFound = false;
  for (size_t n = 0, k = descriptors.size(); n < k; ++n)
    {
    auto const& trackHandle = descriptors[n];
    if (trackHandle.is_valid())
      {
      dataFound = this->extractPvo(trackHandle) || dataFound;
      dataFound = this->extractEvent(trackHandle) || dataFound;
      }
    }

  if (!dataFound)
    {
    qWarning() << "no usable descriptors found from" << uri;
    return false;
    }

  // Done
  return true;
}

//-----------------------------------------------------------------------------
vsTrackOracleDescriptorArchiveSource::vsTrackOracleDescriptorArchiveSource(
  const QUrl& uri, track_oracle::file_format_base* format) :
  super(new vsTrackOracleDescriptorArchiveSourcePrivate(this, uri, format))
{
}

//-----------------------------------------------------------------------------
vsTrackOracleDescriptorArchiveSource::~vsTrackOracleDescriptorArchiveSource()
{
}
