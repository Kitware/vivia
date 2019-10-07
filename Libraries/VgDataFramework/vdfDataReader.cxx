/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfDataReader.h"

#include "vdfDataSource.h"

#include <vgCheckArg.h>

#include <QEventLoop>

//-----------------------------------------------------------------------------
class vdfDataReaderPrivate
{
public:
  vdfDataReaderPrivate() : Source(0) {}

  vdfDataSource* Source;
};

QTE_IMPLEMENT_D_FUNC(vdfDataReader)

//-----------------------------------------------------------------------------
vdfDataReader::vdfDataReader(QObject* parent) :
  QObject(parent), d_ptr(new vdfDataReaderPrivate)
{
}

//-----------------------------------------------------------------------------
vdfDataReader::~vdfDataReader()
{
}

//-----------------------------------------------------------------------------
vdfDataSource* vdfDataReader::source() const
{
  QTE_D_CONST(vdfDataReader);
  return d->Source;
}

//-----------------------------------------------------------------------------
bool vdfDataReader::setSource(vdfDataSource* source)
{
  QTE_D(vdfDataReader);

  if (d->Source != source && (!source || this->connectSource(source)))
    {
    this->disconnectSource(d->Source);
    d->Source = source;

    connect(source, SIGNAL(destroyed(QObject*)),
            this, SLOT(releaseSource(QObject*)));

    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
const QObject* vdfDataReader::dataReceiver() const
{
  return this;
}

//-----------------------------------------------------------------------------
void vdfDataReader::disconnectSource(vdfDataSource* source)
{
  CHECK_ARG(source);

  const QObject* const receiver = this->dataReceiver();
  foreach (QObject* const interface, source->interfaces())
    {
    // Disconnect all signals/slots from the interface to the data receiver
    disconnect(interface, 0, receiver, 0);
    }

  // Disconnect all signals/slots from the source to ourself
  disconnect(source, 0, this, 0);
}

//-----------------------------------------------------------------------------
void vdfDataReader::releaseSource(QObject* object)
{
  QTE_D(vdfDataReader);
  if (object && d->Source == object)
    {
    // Reset the source to 'none' as the old source is no longer valid
    // WARNING: Do not call disconnectSource here! Besides being superfluous,
    //          it will crash when we try to enumerate the source's interfaces,
    //          because the source's virtual methods are no longer valid
    d->Source = 0;
    }
}

//-----------------------------------------------------------------------------
bool vdfDataReader::hasData() const
{
  return false;
}

//-----------------------------------------------------------------------------
bool vdfDataReader::failed() const
{
  auto* const s = this->source();
  return (s && (s->status() == vdfDataSource::Invalid));
}

//-----------------------------------------------------------------------------
bool vdfDataReader::exec()
{
  vdfDataSource* const s = this->source();
  CHECK_ARG(s, false);

  // Start the source if it has not been started yet
  if (s->status() == vdfDataSource::Unstarted)
    {
    s->start();
    }

  // Set up an event loop that will interrupt when the source's status changes
  QEventLoop eventLoop;
  connect(s, SIGNAL(statusChanged(vdfDataSource::Status)),
          &eventLoop, SLOT(quit()));

  // Wait for the source to become stopped or invalid (loop won't execute if
  // the source ends up this way synchronously or if exec() was called on some
  // other reader connected to the same source; in the latter case the other
  // reader's event loop will also let us receive data)
  while (s->status() != vdfDataSource::Stopped &&
         s->status() != vdfDataSource::Invalid)
    {
    // Process events (i.e. let the source do its thing, and our subclasses
    // receive and process the data from the source) until the source's status
    // changes (at which point we'll stop and check the status again)
    eventLoop.exec();
    }

  // We are successful if data was received
  return !this->failed() && this->hasData();
}
