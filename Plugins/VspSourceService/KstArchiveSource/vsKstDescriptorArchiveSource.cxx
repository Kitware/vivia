/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsKstDescriptorArchiveSource.h"

#include <QDebug>

#include <qtKstReader.h>

#include <vvQueryResult.h>

#include <vvEventSetInfo.h>
#include <vvHeader.h>
#include <vvKstReader.h>

#include <vsTrackClassifier.h>

#include <vsAdapt.h>
#include <vsArchiveSourcePrivate.h>

#define die(_msg) return this->abort(uri, _msg);
#define test_or_die(_cond, _msg) if (!(_cond)) die(_msg)

//-----------------------------------------------------------------------------
class vsKstDescriptorArchiveSourcePrivate : public vsArchiveSourcePrivate
{
public:
  vsKstDescriptorArchiveSourcePrivate(vsKstDescriptorArchiveSource* q,
                                      const QUrl& uri);

protected:
  QTE_DECLARE_PUBLIC(vsKstDescriptorArchiveSource)

  virtual bool processArchive(const QUrl& uri) QTE_OVERRIDE;

  void processResults(const vvEventSetInfo& info,
                      const QList<vvQueryResult>& results);

  bool abort(const QUrl& uri, const QString& message);
  bool abort(const QUrl& uri, const char* message);
};

QTE_IMPLEMENT_D_FUNC(vsKstDescriptorArchiveSource)

//-----------------------------------------------------------------------------
vsKstDescriptorArchiveSourcePrivate::vsKstDescriptorArchiveSourcePrivate(
  vsKstDescriptorArchiveSource* q, const QUrl& uri)
  : vsArchiveSourcePrivate(q, "descriptors", uri)
{
}

//-----------------------------------------------------------------------------
bool vsKstDescriptorArchiveSourcePrivate::processArchive(const QUrl& uri)
{
  // Read file and header
  qtKstReader kst(uri);
  vvHeader header;
  vvKstReader reader;
  test_or_die(kst.isValid(), "the archive is not a valid KST");
  test_or_die(reader.readHeader(kst, header), "unable to read header");

  // What type of data are we dealing with?
  if (header.type == vvHeader::EventSetInfo)
    {
    vvEventSetInfo info;
    test_or_die(reader.readEventSetInfo(kst, info, header.version),
                "unable to read result set meta-information");
    test_or_die(reader.readHeader(kst, header), "unable to read header");

    // Check that we have results (fall through to 'unsupported data type'
    // otherwise)
    if (header.type == vvHeader::QueryResults)
      {
      QList<vvQueryResult> results;
      test_or_die(reader.readQueryResults(kst, results, header.version),
                  "unable to read result set results");
      test_or_die(!results.isEmpty(), "result set is empty");

      // Hand off to helper
      this->processResults(info, results);

      // Done
      return true;
      }
    }
  // TODO: support raw descriptors

  // If we get here, the archive contains something that we don't support
  die("the archive does not contain a supported data type");
}

//-----------------------------------------------------------------------------
void vsKstDescriptorArchiveSourcePrivate::processResults(
  const vvEventSetInfo& info, const QList<vvQueryResult>& results)
{
  QTE_Q(vsKstDescriptorArchiveSource);

  // Generate and emit event type
  const int eventType = vsEventInfo::UserType;
  q->emitEventType(vsEventInfo::fromEventSetInfo(info, eventType),
                   info.DisplayThreshold);

  // Emit events
  vtkIdType id = 0;
  foreach (const vvQueryResult& result, results)
    {
    vsEvent event = vsAdapt(result);
    event->SetId(++id);
    event->AddClassifier(eventType, result.RelevancyScore);
    q->emitEvent(event);
    }
}

//-----------------------------------------------------------------------------
bool vsKstDescriptorArchiveSourcePrivate::abort(
  const QUrl& uri, const QString& message)
{
  return this->abort(uri, qPrintable(message));
}

//-----------------------------------------------------------------------------
bool vsKstDescriptorArchiveSourcePrivate::abort(
  const QUrl& uri, const char* message)
{
  const QString errorPrefix("Error reading archive \"%1\":");
  qWarning() << qPrintable(errorPrefix.arg(uri.toString())) << message;
  return false;
}

//-----------------------------------------------------------------------------
vsKstDescriptorArchiveSource::vsKstDescriptorArchiveSource(const QUrl& uri) :
  super(new vsKstDescriptorArchiveSourcePrivate(this, uri))
{
}

//-----------------------------------------------------------------------------
vsKstDescriptorArchiveSource::~vsKstDescriptorArchiveSource()
{
}
