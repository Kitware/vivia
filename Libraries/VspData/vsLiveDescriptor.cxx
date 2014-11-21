/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsLiveDescriptor.h"
#include "vsLiveDescriptorPrivate.h"

#include <QTimer>

QTE_IMPLEMENT_D_FUNC(vsLiveDescriptor)

//-----------------------------------------------------------------------------
vsLiveDescriptorPrivate::vsLiveDescriptorPrivate(vsLiveDescriptor* q) :
  q_ptr(q)
{
}

//-----------------------------------------------------------------------------
vsLiveDescriptorPrivate::~vsLiveDescriptorPrivate()
{
}

//-----------------------------------------------------------------------------
void vsLiveDescriptorPrivate::run()
{
  QTE_Q(vsLiveDescriptor);

  q->updateStatus(vsDataSource::InProcessActive);
  qtThread::run();
}

//-----------------------------------------------------------------------------
void vsLiveDescriptorPrivate::setSuicideTimer(int timeout, bool changeStatus)
{
  if (changeStatus)
    {
    QTE_Q(vsLiveDescriptor);
    q->updateStatus(vsDataSource::InProcessIdle);
    }

  this->suicideTimer.reset(new QTimer);
  this->suicideTimer->setInterval(timeout);
  this->suicideTimer->setSingleShot(true);
  this->suicideTimer->start();
  connect(this->suicideTimer.data(), SIGNAL(timeout()),
          this, SLOT(suicide()));
}

//-----------------------------------------------------------------------------
void vsLiveDescriptorPrivate::cancelSuicideTimer(bool changeStatus)
{
  if (this->suicideTimer)
    {
    if (changeStatus)
      {
      QTE_Q(vsLiveDescriptor);
      q->updateStatus(vsDataSource::InProcessActive);
      }
    this->suicideTimer.reset();
    }
}

//-----------------------------------------------------------------------------
void vsLiveDescriptorPrivate::suicide()
{
  QTE_Q(vsLiveDescriptor);
  q->suicide();
}

//-----------------------------------------------------------------------------
vsLiveDescriptor::vsLiveDescriptor(vsLiveDescriptorPrivate* d) : d_ptr(d)
{
  connect(d, SIGNAL(eventAvailable(vsEvent)),
          this, SLOT(emitEvent(vsEvent)));
  connect(d, SIGNAL(eventRevoked(vtkIdType)),
          this, SLOT(revokeEvent(vtkIdType)));
  d->status = vsDataSource::InProcessIdle;
}

//-----------------------------------------------------------------------------
vsLiveDescriptor::~vsLiveDescriptor()
{
  QTE_D(vsLiveDescriptor);
  d->quit();
  d->wait();
  delete d;
}

//-----------------------------------------------------------------------------
void vsLiveDescriptor::start()
{
  QTE_D(vsLiveDescriptor);
  vsDataSource::start();
  d->start();
}

//-----------------------------------------------------------------------------
void vsLiveDescriptor::updateStatus(vsDataSource::Status status)
{
  if (QThread::currentThread() != this->thread())
    {
    QMetaObject::invokeMethod(this, "updateStatus",
                              Q_ARG(vsDataSource::Status, status));
    return;
    }

  QTE_D(vsLiveDescriptor);
  d->status = status;
  emit this->statusChanged(status);
}

//-----------------------------------------------------------------------------
vsDataSource::Status vsLiveDescriptor::status() const
{
  QTE_D_CONST(vsLiveDescriptor);
  return d->status;
}

//-----------------------------------------------------------------------------
QString vsLiveDescriptor::text() const
{
  return "In-Process";
}

//-----------------------------------------------------------------------------
QString vsLiveDescriptor::toolTip() const
{
  QTE_D_CONST(vsLiveDescriptor);

  QString format("Live in-process %1 descriptor");
  if (d->status == vsDataSource::InProcessIdle)
    format += " (idle)";

  return format.arg(this->name());
}

//-----------------------------------------------------------------------------
void vsLiveDescriptor::injectInput(qint64 id, vsDescriptorInputPtr input)
{
  QTE_D(vsLiveDescriptor);
  QMetaObject::invokeMethod(d, "injectInput", Qt::QueuedConnection,
                            Q_ARG(qint64, id),
                            Q_ARG(vsDescriptorInputPtr, input));
  vsDescriptorSource::injectInput(id, input);
}

//-----------------------------------------------------------------------------
void vsLiveDescriptor::revokeInput(qint64 id, bool revokeEvents)
{
  QTE_D(vsLiveDescriptor);
  QMetaObject::invokeMethod(d, "revokeInput", Qt::QueuedConnection,
                            Q_ARG(qint64, id),
                            Q_ARG(bool, revokeEvents));
  vsDescriptorSource::revokeInput(id, revokeEvents);
}

//-----------------------------------------------------------------------------
void vsLiveDescriptor::revokeAllInput(bool revokeEvents)
{
  QTE_D(vsLiveDescriptor);
  QMetaObject::invokeMethod(d, "revokeAllInput", Qt::QueuedConnection,
                            Q_ARG(bool, revokeEvents));
  vsDescriptorSource::revokeAllInput(revokeEvents);
}
