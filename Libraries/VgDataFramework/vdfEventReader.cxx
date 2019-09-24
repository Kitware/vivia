/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfEventReader.h"

#include "vdfDataSource.h"
#include "vdfEventSource.h"

#include <vgCheckArg.h>

#include <QDebug>

QTE_IMPLEMENT_D_FUNC(vdfEventReader)

//-----------------------------------------------------------------------------
class vdfEventReaderPrivate
{
public:
  void addEvents(const QList<vvEvent>& events);

  QSet<long long> EventIds;
  QList<vvEvent> Events;

};

//-----------------------------------------------------------------------------
void vdfEventReaderPrivate::addEvents(
  const QList<vvEvent>& events)
{
  this->Events.reserve(this->Events.count() + events.count());
  for (auto const& event : events)
  {
    if (this->EventIds.contains(event.Id))
    {
      qWarning() << "vdfEventReader: Ignoring event with duplicate ID"
                 << event.Id;
    }
    else
    {
      this->EventIds.insert(event.Id);
      this->Events.append(event);
    }
  }
}

//-----------------------------------------------------------------------------
vdfEventReader::vdfEventReader(vdfDataSource* source, QObject* parent) :
  vdfDataReader(parent), d_ptr(new vdfEventReaderPrivate)
{
  this->setSource(source);
}

//-----------------------------------------------------------------------------
vdfEventReader::~vdfEventReader()
{
}

//-----------------------------------------------------------------------------
bool vdfEventReader::hasData() const
{
  QTE_D();
  return !d->Events.isEmpty();
}

//-----------------------------------------------------------------------------
QList<vvEvent> vdfEventReader::events() const
{
  QTE_D();
  return d->Events;
}

//-----------------------------------------------------------------------------
bool vdfEventReader::connectSource(vdfDataSource* source)
{
  CHECK_ARG(source, false);

  vdfEventSourceInterface* ei = source->interface<vdfEventSourceInterface>();
  if (ei)
  {
    QTE_D();

    connect(ei, &vdfEventSourceInterface::eventsAvailable, this,
            [d](const QList<vvEvent> events){ d->addEvents(events); });

    return true;
  }

  return false;
}
