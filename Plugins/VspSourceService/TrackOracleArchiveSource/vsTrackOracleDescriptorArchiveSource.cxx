/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackOracleDescriptorArchiveSource.h"

#include <QDebug>
#include <QSet>

#include <qtThread.h>
#include <qtStlUtil.h>

#include <vgGeodesy.h>

#include <track_oracle/file_format_base.h>

#include <vtkVsTrackInfo.h>

#include <vsAdapt.h>
#include <vsArchiveSourcePrivate.h>

#include "visgui_descriptor_type.h"

//-----------------------------------------------------------------------------
class vsTrackOracleDescriptorArchiveSourcePrivate :
  public vsArchiveSourcePrivate
{
public:
  typedef vidtk::file_format_base FileFormat;

  vsTrackOracleDescriptorArchiveSourcePrivate(
    vsTrackOracleDescriptorArchiveSource* q, const QUrl& uri,
    FileFormat* format);

  bool extractPvo(const vidtk::track_handle_type&);
  bool extractClassifier(const vidtk::track_handle_type&);

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
  const vidtk::track_handle_type& trackHandle)
{
  // Attempt to extract P/V/O track object classifier
  visgui_pvo_descriptor_type pvo;
  pvo(trackHandle);
  if (pvo.source_track_ids.exists() &&
      pvo.descriptor_pvo_raw_scores.exists() &&
      pvo.descriptor_pvo_raw_scores().size() >= 3)
    {
    QTE_Q(vsTrackOracleDescriptorArchiveSource);

    const vcl_vector<unsigned int>& tracks = pvo.source_track_ids();
    for (size_t n = 0, k = tracks.size(); n < k; ++n)
      {
      vsTrackObjectClassifier toc;
      toc.probabilityPerson  = pvo.descriptor_pvo_raw_scores()[0];
      toc.probabilityVehicle = pvo.descriptor_pvo_raw_scores()[1];
      toc.probabilityOther   = pvo.descriptor_pvo_raw_scores()[2];

      emit q->tocAvailable(vsAdaptTrackId(tracks[n]), toc);
      return true;
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vsTrackOracleDescriptorArchiveSourcePrivate::extractClassifier(
  const vidtk::track_handle_type& trackHandle)
{
  // Attempt to extract classifier event
  visgui_classifier_descriptor_type classifier;
  classifier(trackHandle);
  if (classifier.source_track_ids.exists() &&
      classifier.descriptor_classifier.exists())
    {
    vsEvent event = vsEvent(QUuid() /* FIXME: get from vidtk object */);
    const vcl_vector<unsigned int> tracks = classifier.source_track_ids();

    // Determine event ID (not guaranteed to provide one)
    if (this->UsingInternalIds)
      {
      const vtkIdType id = ++this->LastEventId;
      if(classifier.external_id.exists())
        {
        qDebug() << "assigning generated ID" << id
                 << "to descriptor with external ID"
                 << classifier.external_id();
        }
      event->SetId(id);
      }
    else if(classifier.external_id.exists())
      {
      const vtkIdType id = static_cast<vtkIdType>(classifier.external_id());
      this->LastEventId = qMax(id, this->LastEventId);
      event->SetId(id);
      }
    else
      {
      if (this->LastEventId)
        {
        // Oh, dear... some descriptors have ID's, and some don't...
        qWarning() << "descriptor ID missing after previous descriptors"
                   << "using external ID";
        qWarning() << "forcing remaining descriptor(s)"
                   << "to use generated ID's";
        }
      this->UsingInternalIds = true;
      event->SetId(++this->LastEventId);
      }

    // Extract classifiers
    const vcl_vector<double>& classifierValues =
      classifier.descriptor_classifier();
    for (size_t n = 0, k = classifierValues.size(); n < k; ++n)
      {
      const double p = classifierValues[n];
      if (p > 0.0)
        {
        event->AddClassifier(static_cast<int>(n), p, 0.0);
        }
      }

    // Extract descriptor regions
    bool eventIsValid = false;
    vtkVgTimeStamp eventStart;
    vtkVgTimeStamp eventEnd;
    vidtk::frame_handle_list_type frameHandles =
      vidtk::track_oracle::get_frames(trackHandle);
    for (size_t n = 0, k = frameHandles.size(); n < k; ++n)
      {
      const vidtk::frame_handle_type& frameHandle = frameHandles[n];
      if (frameHandle.is_valid())
        {
        // Set oracle object to frame instance referenced by handle
        classifier[frameHandle];

        // Extract timestamp
        vtkVgTimeStamp ts;
        if (classifier.timestamp_usecs.exists())
          {
          ts.SetTime(classifier.timestamp_usecs());
          }
        if (classifier.frame_number.exists())
          {
          ts.SetFrameNumber(classifier.frame_number());
          }

        // Don't add region if timestamp is not valid, or box is missing
        if (!ts.IsValid() || !classifier.bounding_box.exists())
          {
          continue;
          }

        // Update event temporal bounds
        eventStart = (eventIsValid && ts >= eventStart ? eventStart : ts);
        eventEnd   = (eventIsValid && ts <= eventEnd   ? eventEnd   : ts);

        // Extract bounding box
        const vgl_box_2d<double>& box = classifier.bounding_box();

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
      qWarning() << "ignoring classifier descriptor with empty region list";
      return false;
      }

    // Add track(s)
    classifier(trackHandle);
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

  return false;
}

//-----------------------------------------------------------------------------
bool vsTrackOracleDescriptorArchiveSourcePrivate::processArchive(
  const QUrl& uri)
{
  const std::string fileName = stdString(uri.toLocalFile());

  // Read descriptors
  vidtk::track_handle_list_type descriptors;
  if (!this->Format->read(fileName, descriptors))
    {
    qWarning() << "error reading descriptors from" << uri;
    return false;
    }

  // Process the descriptors
  bool dataFound = false;
  for (size_t n = 0, k = descriptors.size(); n < k; ++n)
    {
    const vidtk::track_handle_type& trackHandle = descriptors[n];
    if (trackHandle.is_valid())
      {
      dataFound = this->extractPvo(trackHandle) || dataFound;
      dataFound = this->extractClassifier(trackHandle) || dataFound;
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
  const QUrl& uri, vidtk::file_format_base* format) :
  super(new vsTrackOracleDescriptorArchiveSourcePrivate(this, uri, format))
{
}

//-----------------------------------------------------------------------------
vsTrackOracleDescriptorArchiveSource::~vsTrackOracleDescriptorArchiveSource()
{
}
