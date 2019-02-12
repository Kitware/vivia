/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVvqsDatabaseSourcePrivate.h"

#include <vsAdapt.h>

#include <vsTrackSource.h>

#include <vvMakeId.h>
#include <vvQuerySession.h>

#include <vgStringLiteral.h>

#include <qtScopedValueChange.h>
#include <qtStlUtil.h>

#include <QDebug>
#include <QRegExp>
#include <QSettings>

#include <limits>

QTE_IMPLEMENT_D_FUNC(vsVvqsDatabaseSource)

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsVvqsDatabaseTrackSource

//-----------------------------------------------------------------------------
class vsVvqsDatabaseTrackSource : public vsTrackSource
{
public:
  virtual ~vsVvqsDatabaseTrackSource() {}

  virtual Status status() const;
  virtual QString text() const;
  virtual QString toolTip() const;

protected:
  QTE_DECLARE_PUBLIC_PTR(vsVvqsDatabaseSourcePrivate)
  friend class vsVvqsDatabaseSource;

  vsVvqsDatabaseTrackSource(vsVvqsDatabaseSourcePrivate* q) : q_ptr(q) {}

private:
  QTE_DECLARE_PUBLIC(vsVvqsDatabaseSourcePrivate)
};

//-----------------------------------------------------------------------------
vsDataSource::Status vsVvqsDatabaseTrackSource::status() const
{
  QTE_Q_CONST(vsVvqsDatabaseSourcePrivate);
  return (q->PublicTrackStatus == vsDataSource::InProcessActive
          ? vsDataSource::ArchivedActive : q->PublicTrackStatus);
}

//-----------------------------------------------------------------------------
QString vsVvqsDatabaseTrackSource::text() const
{
  QTE_Q_CONST(vsVvqsDatabaseSourcePrivate);
  return q->text(q->PublicTrackStatus);
}

//-----------------------------------------------------------------------------
QString vsVvqsDatabaseTrackSource::toolTip() const
{
  QTE_Q_CONST(vsVvqsDatabaseSourcePrivate);
  return q->toolTip(q->PublicTrackStatus, "track", "tracks");
}

//END vsVvqsDatabaseTrackSource

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsVvqsDatabaseSourcePrivate

//-----------------------------------------------------------------------------
vsVvqsDatabaseSourcePrivate::vsVvqsDatabaseSourcePrivate(
  vsVvqsDatabaseSource* q, vvQuerySession* qs, const QUrl& request)
  : q_ptr(q), TrackSource(new vsVvqsDatabaseTrackSource(this)),
    QuerySession(qs), RequestUri(request), NextEventId(0)
{
  this->TrackSource->moveToThread(this->thread());
}

//-----------------------------------------------------------------------------
vsVvqsDatabaseSourcePrivate::~vsVvqsDatabaseSourcePrivate()
{
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePrivate::run()
{
  this->updateStatus(vsDataSource::ArchivedActive,
                     vsDataSource::ArchivedActive);

  // Connect to query session signals
  connect(this->QuerySession.data(),
          SIGNAL(resultAvailable(vvQueryResult, bool)),
          this, SLOT(processResult(vvQueryResult)));
  connect(this->QuerySession.data(), SIGNAL(resultSetComplete(bool)),
          this, SLOT(finalize()));
  connect(this->QuerySession.data(),
          SIGNAL(error(qtStatusSource, QString)),
          this, SLOT(die(qtStatusSource, QString)));

  // Get retrieval options
  QSettings settings;
  settings.beginGroup("Database");
  this->RetrieveDescriptors =
    settings.value("RetrieveDescriptors", true).toBool();
  this->DescriptorBatchSize =
    settings.value("DescriptorBatchSize", 400).toInt();

  // Create retrieval query
  this->Query.QueryId = vvMakeId("VSPLAY-DB-SOURCE");
  this->Query.StreamIdLimit =
    stdString(this->RequestUri.queryItemValue("Stream"));

  // Query initially for just tracks, as retrieving descriptors can be slow,
  // and we want the tracks available as soon as possible
  this->Query.RequestedEntities = vvRetrievalQuery::Tracks;

  const QString tl = this->RequestUri.queryItemValue("TemporalLower");
  const QString tu = this->RequestUri.queryItemValue("TemporalUpper");
  if (!tl.isEmpty())
    {
    this->Query.TemporalLowerLimit = tl.toLongLong();
    }
  if (!tu.isEmpty())
    {
    this->Query.TemporalUpperLimit = tu.toLongLong();
    }

  const QString ec =
    this->RequestUri.queryItemValue("ExtractClassifiers").toLower();
  this->ExtractClassifiers = (ec == "yes" || ec == "true");

  // Issue the query
  this->QuerySession->processQuery(
    this->Query, std::numeric_limits<int>::max());

  // Hand off to the event loop
  qtThread::run();
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePrivate::processResult(vvQueryResult result)
{
  vsDataSource::Status trackStatus =
    (result.Tracks.empty() ? this->PrivateTrackStatus
                           : vsDataSource::InProcessActive);
  vsDataSource::Status descriptorStatus =
    (result.Descriptors.empty() ? this->PrivateDescriptorStatus
                                : vsDataSource::InProcessActive);

  this->updateStatus(trackStatus, descriptorStatus);

  size_t n, k;

  // Emit tracks
  for (n = 0, k = result.Tracks.size(); n < k; ++n)
    {
    const vvTrack& track = result.Tracks[n];

    QList<vvTrackState> states;
    foreach_iter (vvTrackTrajectory::const_iterator, tsIter, track.Trajectory)
      {
      states.append(*tsIter);
      }

    // Emit track
    emit this->TrackSource->trackUpdated(track.Id, states);
    emit this->TrackSource->trackClosed(track.Id);

    // Extract and emit track classification
    vsTrackObjectClassifier toc;
    QMap<std::string, double> classifier(track.Classification);
    toc.probabilityPerson = classifier.value("TTPerson", 0.0);
    toc.probabilityVehicle = classifier.value("TTVehicle", 0.0);
    toc.probabilityOther =
      1.0 - (toc.probabilityPerson + toc.probabilityVehicle);
    emit this->tocAvailable(track.Id, toc);
    }

  // Emit descriptors and extract classifiers
  QList<vsEvent> classifierEvents;
  for (n = 0, k = result.Descriptors.size(); n < k; ++n)
    {
    const vvDescriptor& descriptor = result.Descriptors[n];

    // Add descriptor to current batch
    this->PendingDescriptors.append(new vvDescriptor(descriptor));

    // Check if current batch is at size limit
    if (this->PendingDescriptors.count() >= this->DescriptorBatchSize)
      {
      // If so, send the current batch (which will transfer the descriptors to
      // a new instance, making the current batch empty again)
      emit this->descriptorsAvailable(this->PendingDescriptors);
      }

    if (this->ExtractClassifiers)
      {
      vsExtractClassifier(descriptor, classifierEvents);
      }
    }

  // Emit classifier events
  foreach (vsEvent event, classifierEvents)
    {
    event->SetId(++this->NextEventId);
    emit this->eventAvailable(event);
    }
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePrivate::finalize()
{
  // Did we get descriptors, or just tracks?
  if (this->Query.RequestedEntities == vvRetrievalQuery::Tracks)
    {
    // If just tracks, do we want to re-query for descriptors?
    if (this->RetrieveDescriptors)
      {
      this->updateStatus(vsDataSource::ArchivedIdle,
                         vsDataSource::ArchivedActive);
      this->Query.RequestedEntities = vvRetrievalQuery::Descriptors;
      this->QuerySession->endQuery();
      this->QuerySession->processQuery(
        this->Query, std::numeric_limits<int>::max());

      // Wait for second query to finish
      return;
      }
    }

  // Do we have any descriptors left that have not yet been sent?
  if (!this->PendingDescriptors.isEmpty())
    {
    // If so, send them now
    emit this->descriptorsAvailable(this->PendingDescriptors);
    }

  // We're done; change status to Idle and release query session
  this->updateStatus(vsDataSource::ArchivedIdle,
                     vsDataSource::ArchivedIdle);
  this->QuerySession.reset();
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePrivate::die(qtStatusSource, QString error)
{
  if (error.isEmpty())
    {
    error = "An unknown error occurred";
    }
  qWarning() << "Database retrieval failed:" << error;
  this->suicide();
}

//-----------------------------------------------------------------------------
QString vsVvqsDatabaseSourcePrivate::text(vsDataSource::Status status) const
{
  // Get appropriate format
  switch (status)
    {
    case vsDataSource::InProcessActive:
    case vsDataSource::ArchivedActive:
      return "A Archived";
    case vsDataSource::ArchivedIdle:
      return "(I Archived)";
    default:
      return "(none)";
    }
}

//-----------------------------------------------------------------------------
QString vsVvqsDatabaseSourcePrivate::toolTip(
  vsDataSource::Status status,
  const QString& sourceTypeSingular,
  const QString& sourceTypePlural) const
{
  // Get appropriate format
  QString format;
  switch (status)
    {
    case vsDataSource::InProcessActive:
      format = "Receiving %1 from %2";
      break;
    case vsDataSource::ArchivedActive:
      format = "Requesting %1 from %2";
      break;
    case vsDataSource::ArchivedIdle:
      format = "Using archived %1 from %2";
      break;
    default:
      return QString("(no %1 source)").arg(sourceTypeSingular);
    }

  // Return formatted text
  return format.arg(sourceTypePlural).arg(this->displayableRequestUri());
}

//-----------------------------------------------------------------------------
QString vsVvqsDatabaseSourcePrivate::displayableRequestUri() const
{
  // Separate query items with zero-width space for better line breaking
  QString uri = this->RequestUri.toString();
  static const auto replacement = QStringLiteral("\u200b\\1");
  return uri.replace(QRegExp(QStringLiteral("([?&])")), replacement);
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePrivate::updateStatus(
  vsDataSource::Status trackStatus, vsDataSource::Status descriptorStatus)
{
  bool needUpdate = false;

  if (trackStatus != this->PrivateTrackStatus)
    {
    this->PrivateTrackStatus = trackStatus;
    needUpdate = true;
    }
  if (descriptorStatus != this->PrivateDescriptorStatus)
    {
    this->PrivateDescriptorStatus = descriptorStatus;
    needUpdate = true;
    }

  if (needUpdate)
    {
    // Report status to parent
    QTE_Q(vsVvqsDatabaseSource);
    QMetaObject::invokeMethod(
      q, "updateStatus",
      Q_ARG(vsDataSource::Status, trackStatus),
      Q_ARG(vsDataSource::Status, descriptorStatus));
    }
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSourcePrivate::suicide()
{
  QTE_Q(vsVvqsDatabaseSource);

  // Stop our thread
  this->quit();

  // Notify core we are done
  this->TrackSource->suicide();
  q->suicide(); // CAUTION: we may be deleted when this returns!
}

//END vsVvqsDatabaseSourcePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsVvqsDatabaseSource

//-----------------------------------------------------------------------------
vsVvqsDatabaseSource::vsVvqsDatabaseSource(
  vvQuerySession* querySession, const QUrl& request)
  : d_ptr(new vsVvqsDatabaseSourcePrivate(this, querySession, request))
{
  QTE_D(vsVvqsDatabaseSource);

  connect(d, SIGNAL(descriptorsAvailable(vsDescriptorList)),
          this, SLOT(emitDescriptors(vsDescriptorList)));
  connect(d, SIGNAL(eventAvailable(vsEvent)),
          this, SLOT(emitEvent(vsEvent)));
  connect(d, SIGNAL(tocAvailable(vsTrackId, vsTrackObjectClassifier)),
          this, SIGNAL(tocAvailable(vsTrackId, vsTrackObjectClassifier)));

  d->PublicTrackStatus = vsDataSource::ArchivedIdle;
  d->PrivateTrackStatus = vsDataSource::ArchivedIdle;
  d->PublicDescriptorStatus = vsDataSource::ArchivedIdle;
  d->PrivateDescriptorStatus = vsDataSource::ArchivedIdle;
}

//-----------------------------------------------------------------------------
vsVvqsDatabaseSource::~vsVvqsDatabaseSource()
{
  QTE_D(vsVvqsDatabaseSource);

  // Don't emit our own destroyed() when we are going away anyway
  qtScopedBlockSignals bs(this);

  // Shut down thread and track source
  d->suicide();
  d->wait();
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSource::start()
{
  QTE_D(vsVvqsDatabaseSource);

  d->start();
  vsDataSource::start();
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseSource::updateStatus(
  vsDataSource::Status trackStatus, vsDataSource::Status descriptorStatus)
{
  QTE_D(vsVvqsDatabaseSource);

  if (trackStatus != d->PublicTrackStatus)
    {
    d->PublicTrackStatus = trackStatus;
    emit d->TrackSource->statusChanged(trackStatus);
    }
  if (descriptorStatus != d->PublicDescriptorStatus)
    {
    d->PublicDescriptorStatus = descriptorStatus;
    emit this->statusChanged(descriptorStatus);
    }
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsVvqsDatabaseSource::status() const
{
  QTE_D_CONST(vsVvqsDatabaseSource);
  return (d->PublicDescriptorStatus == vsDataSource::InProcessActive
          ? vsDataSource::ArchivedActive : d->PublicDescriptorStatus);
}

//-----------------------------------------------------------------------------
QString vsVvqsDatabaseSource::text() const
{
  QTE_D_CONST(vsVvqsDatabaseSource);
  return d->text(d->PublicDescriptorStatus);
}

//-----------------------------------------------------------------------------
QString vsVvqsDatabaseSource::toolTip() const
{
  QTE_D_CONST(vsVvqsDatabaseSource);
  return d->toolTip(d->PublicDescriptorStatus, "descriptor", "descriptors");
}

//-----------------------------------------------------------------------------
vsTrackSourcePtr vsVvqsDatabaseSource::trackSource()
{
  QTE_D(vsVvqsDatabaseSource);
  return d->TrackSource;
}

//END vsVvqsDatabaseSource
