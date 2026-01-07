// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsArchiveSourcePrivate.h"

QTE_IMPLEMENT_ALIASED_D_FUNC(vsArchiveSourceHelper, vsArchiveSourcePrivate)

//-----------------------------------------------------------------------------
vsArchiveSourcePrivate::~vsArchiveSourcePrivate()
{
}

//-----------------------------------------------------------------------------
void vsArchiveSourcePrivate::run()
{
  QTE_Q(vsArchiveSourceHelper);

  q->emitStatusChanged(vsDataSource::ArchivedActive);

  if (!this->processArchive(this->ArchiveUri))
    {
    // If the read failed, commit suicide (the core will clean us up)
    q->suicide();
    return;
    }

  // Done
  this->Active = false;
  q->emitStatusChanged(vsDataSource::ArchivedIdle);
}

//-----------------------------------------------------------------------------
bool vsArchiveSourcePrivate::isRunning() const
{
  return this->Active && qtThread::isRunning();
}

//-----------------------------------------------------------------------------
vsArchiveSourceHelper::vsArchiveSourceHelper(vsArchiveSourcePrivate* d) :
  d_ptr(d)
{
}

//-----------------------------------------------------------------------------
vsArchiveSourceHelper::~vsArchiveSourceHelper()
{
  QTE_D(vsArchiveSource);
  delete d;
}

//-----------------------------------------------------------------------------
void vsArchiveSourceHelper::finishConstruction()
{
  this->emitStatusChanged(vsDataSource::ArchivedIdle);
}

//-----------------------------------------------------------------------------
void vsArchiveSourceHelper::wait()
{
  QTE_D(vsArchiveSource);
  d->wait();
}

//-----------------------------------------------------------------------------
void vsArchiveSourceHelper::start()
{
  QTE_D(vsArchiveSource);
  d->start();
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsArchiveSourceHelper::status() const
{
  QTE_D_CONST(vsArchiveSource);
  return (d->isRunning() ? vsDataSource::ArchivedActive
                         : vsDataSource::ArchivedIdle);
}

//-----------------------------------------------------------------------------
QString vsArchiveSourceHelper::text() const
{
  QTE_D_CONST(vsArchiveSource);
  return (d->isRunning() ? "Loading" : "Archived");
}

//-----------------------------------------------------------------------------
QString vsArchiveSourceHelper::toolTip() const
{
  QTE_D_CONST(vsArchiveSource);

  const QString ttFormat = (d->isRunning() ? "Reading archived %1 from %2"
                                           : "Using archived %1 from %2");
  return ttFormat.arg(d->Type, d->ArchiveUri.toString());
}
