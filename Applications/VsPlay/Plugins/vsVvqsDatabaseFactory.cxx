/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVvqsDatabaseFactory.h"

#include <QMessageBox>

#include <vvQueryService.h>

#include <vsTrackSource.h>

#include "vsVvqsDatabaseQueryDialog.h"
#include "vsVvqsDatabaseSource.h"

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
  server.removeAllQueryItems("Stream");
  server.removeAllQueryItems("TemporalLower");
  server.removeAllQueryItems("TemporalUpper");
  server.removeAllQueryItems("ExtractClassifiers");

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

  QUrl muri = uri;
  if (uri.queryItemValue("ExtractClassifiers") == "ask")
    {
    // Build message
    muri.setEncodedQuery(QByteArray());
    QString msg = "Do you wish to request classifier extraction in this query?"
                  "\n\nServer: " + muri.toString();
    muri = uri;
    muri.removeAllQueryItems("ExtractClassifiers");
    typedef QPair<QString, QString> QueryItem;
    foreach (const QueryItem& qi, muri.queryItems())
      {
      msg += QString("\n%1: %2").arg(qi.first, qi.second);
      }

    // Ask user if they want classifiers
    int result = QMessageBox::question(0, "Extract Classifiers?", msg,
                                       QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes)
      {
      muri.addQueryItem("ExtractClassifiers", "yes");
      }
    }

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
