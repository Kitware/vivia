// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vdfThreadedArchiveSource.h"

#include <QtConcurrentRun>
#include <QUrl>

//-----------------------------------------------------------------------------
class vdfThreadedArchiveSourcePrivate
{
protected:
  QTE_DECLARE_PUBLIC_PTR(vdfThreadedArchiveSource)

private:
  QTE_DECLARE_PUBLIC(vdfThreadedArchiveSource)

public:
  vdfThreadedArchiveSourcePrivate(
    vdfThreadedArchiveSource* q, const QUrl& uri) :
    q_ptr(q),
    ArchiveUri(uri),
    Started(false)
    {}

  const QUrl ArchiveUri;
  QFuture<void> Future;
  bool Started;
};

QTE_IMPLEMENT_D_FUNC(vdfThreadedArchiveSource)

//-----------------------------------------------------------------------------
vdfThreadedArchiveSource::vdfThreadedArchiveSource(
  const QUrl& uri, QObject* parent) :
  vdfDataSource(parent), d_ptr(new vdfThreadedArchiveSourcePrivate(this, uri))
{
}

//-----------------------------------------------------------------------------
vdfThreadedArchiveSource::~vdfThreadedArchiveSource()
{
  QTE_D(vdfThreadedArchiveSource);
  if (d->Future.isRunning())
    {
    d->Future.waitForFinished();
    }
}

//-----------------------------------------------------------------------------
QString vdfThreadedArchiveSource::type() const
{
  QTE_THREADSAFE_STATIC_CONST(QString) myType("Archive Data Source");
  return myType;
}

//-----------------------------------------------------------------------------
vdfDataSource::Mechanism vdfThreadedArchiveSource::mechanism() const
{
  return vdfDataSource::Archive;
}

//-----------------------------------------------------------------------------
void vdfThreadedArchiveSource::start()
{
  QTE_D(vdfThreadedArchiveSource);

  if (!d->Started)
    {
    // Archive has not been started; do so now by executing
    // vdfThreadedArchiveSource::run in a separate thread
    d->Future = QtConcurrent::run(this, &vdfThreadedArchiveSource::run);
    d->Started = true;
    }
}

//-----------------------------------------------------------------------------
void vdfThreadedArchiveSource::run()
{
  QMetaObject::invokeMethod(
    this, "setStatus", Q_ARG(vdfDataSource::Status, vdfDataSource::Active));

  if (this->processArchive(this->uri()))
    {
    // Archive was read successfully; indicate that we are done
    QMetaObject::invokeMethod(
      this, "setStatus", Q_ARG(vdfDataSource::Status, vdfDataSource::Stopped));
    }
  else
    {
    // Archive read failed; indicate that the source is invalid
    QMetaObject::invokeMethod(
      this, "setStatus", Q_ARG(vdfDataSource::Status, vdfDataSource::Invalid));
    }
}

//-----------------------------------------------------------------------------
QUrl vdfThreadedArchiveSource::uri() const
{
  QTE_D_CONST(vdfThreadedArchiveSource);
  return d->ArchiveUri;
}
