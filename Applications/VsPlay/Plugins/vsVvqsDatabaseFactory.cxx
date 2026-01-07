// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsVvqsDatabaseFactory.h"

#include "vsVvqsDatabaseQueryDialog.h"
#include "vsVvqsDatabaseSource.h"

#include <vsTrackSource.h>

#include <vvQueryService.h>

#include <QMessageBox>
#include <QUrlQuery>

QTE_IMPLEMENT_D_FUNC(vsVvqsDatabaseFactory)

//-----------------------------------------------------------------------------
class vsVvqsDatabaseFactoryPrivate
{
public:
  static vvQuerySession* createSession(QUrl server);

  vsTrackSourcePtr TrackSource;
  vsDescriptorSourcePtr DescriptorSource;
};

//-----------------------------------------------------------------------------
vvQuerySession* vsVvqsDatabaseFactoryPrivate::createSession(QUrl server)
{
  // Remove request parameters from URI
  auto q = QUrlQuery{server};
  q.removeAllQueryItems("Stream");
  q.removeAllQueryItems("TemporalLower");
  q.removeAllQueryItems("TemporalUpper");
  q.removeAllQueryItems("ExtractClassifiers");
  server.setQuery(q);

  // Create session
  return vvQueryService::createSession(server);
}

//-----------------------------------------------------------------------------
vsVvqsDatabaseFactory::vsVvqsDatabaseFactory() :
  d_ptr(new vsVvqsDatabaseFactoryPrivate)
{
}

//-----------------------------------------------------------------------------
vsVvqsDatabaseFactory::~vsVvqsDatabaseFactory()
{
}

//-----------------------------------------------------------------------------
bool vsVvqsDatabaseFactory::initialize(QWidget* dialogParent)
{
  vsVvqsDatabaseQueryDialog dlg(dialogParent);
  if (dlg.exec() != QDialog::Accepted)
    {
    return false;
    }

  return this->initialize(dlg.uri());
}

//-----------------------------------------------------------------------------
bool vsVvqsDatabaseFactory::initialize(const QUrl& uri)
{
  return this->initialize(uri, 0);
}

//-----------------------------------------------------------------------------
bool vsVvqsDatabaseFactory::initialize(const QUrl& uri, QWidget* dialogParent)
{
  // Create query session
  vvQuerySession* querySession =
    vsVvqsDatabaseFactoryPrivate::createSession(uri);
  if (!querySession)
    {
    const QString message =
      "Error processing database request \"" + uri.toString() +
      "\": the scheme " + uri.scheme() + " is not supported.";
    this->warn(dialogParent, "Not supported", message);
    return false;
    }

  auto muri = uri;
  auto mquery = QUrlQuery{muri};
  muri.setQuery(QUrlQuery{});
  if (mquery.queryItemValue("ExtractClassifiers") == "ask")
    {
    // Build message
    QString msg = "Do you wish to request classifier extraction in this query?"
                  "\n\nServer: " + muri.toString();
    mquery.removeAllQueryItems("ExtractClassifiers");
    foreach (const auto& qi, mquery.queryItems())
      {
      msg += QString("\n%1: %2").arg(qi.first, qi.second);
      }

    // Ask user if they want classifiers
    int result = QMessageBox::question(0, "Extract Classifiers?", msg,
                                       QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes)
      {
      mquery.addQueryItem("ExtractClassifiers", "yes");
      }
    }
  muri.setQuery(mquery);

  QTE_D(vsVvqsDatabaseFactory);

  // Create source
  vsVvqsDatabaseSource* source = new vsVvqsDatabaseSource(querySession, muri);
  d->DescriptorSource = vsDescriptorSourcePtr(source);
  d->TrackSource = source->trackSource();

  // Done
  return true;
}

//-----------------------------------------------------------------------------
QList<vsTrackSourcePtr> vsVvqsDatabaseFactory::trackSources() const
{
  QTE_D_CONST(vsVvqsDatabaseFactory);

  if (!d->TrackSource)
    {
    return vsSourceFactory::trackSources();
    }

  QList<vsTrackSourcePtr> list;
  list.append(d->TrackSource);
  return list;
}

//-----------------------------------------------------------------------------
QList<vsDescriptorSourcePtr> vsVvqsDatabaseFactory::descriptorSources() const
{
  QTE_D_CONST(vsVvqsDatabaseFactory);

  if (!d->DescriptorSource)
    {
    return vsSourceFactory::descriptorSources();
    }

  QList<vsDescriptorSourcePtr> list;
  list.append(d->DescriptorSource);
  return list;
}
