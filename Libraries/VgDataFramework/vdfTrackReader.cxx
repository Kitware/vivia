/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfTrackReader.h"
#include "vdfTrackReaderPrivate.h"

#include "vdfDataSource.h"
#include "vdfTrackSource.h"

#include <vgCheckArg.h>

QTE_IMPLEMENT_D_FUNC(vdfTrackReader)

//-----------------------------------------------------------------------------
void vdfTrackReaderPrivate::setTrackName(
  const vdfTrackId& trackId, const QString& name)
{
  this->Tracks[trackId].Name = name;
}

//-----------------------------------------------------------------------------
void vdfTrackReaderPrivate::setTrackClassification(
  const vdfTrackId& trackId, const vvTrackObjectClassification& toc)
{
  this->Tracks[trackId].Classification = toc;
}

//-----------------------------------------------------------------------------
void vdfTrackReaderPrivate::setTrackState(
  const vdfTrackId& trackId, vvTrackState state)
{
  this->Tracks[trackId].Trajectory.insert(state.TimeStamp, state);
}

//-----------------------------------------------------------------------------
void vdfTrackReaderPrivate::setTrackStates(
  const vdfTrackId& trackId, const QList<vvTrackState>& states)
{
  vdfTrackReader::Track& track = this->Tracks[trackId];
  foreach (const vvTrackState& state, states)
    {
    track.Trajectory.insert(state.TimeStamp, state);
    }
}

//-----------------------------------------------------------------------------
vdfTrackReader::vdfTrackReader(vdfDataSource* source, QObject* parent) :
  vdfDataReader(parent), d_ptr(new vdfTrackReaderPrivate)
{
  this->setSource(source);
}

//-----------------------------------------------------------------------------
vdfTrackReader::~vdfTrackReader()
{
}

//-----------------------------------------------------------------------------
bool vdfTrackReader::hasData() const
{
  QTE_D_CONST(vdfTrackReader);
  return !d->Tracks.isEmpty();
}

//-----------------------------------------------------------------------------
QHash<vdfTrackId, vdfTrackReader::Track> vdfTrackReader::tracks() const
{
  QTE_D_CONST(vdfTrackReader);
  return d->Tracks;
}

//-----------------------------------------------------------------------------
bool vdfTrackReader::connectSource(vdfDataSource* source)
{
  CHECK_ARG(source, false);

  vdfTrackSourceInterface* ti = source->interface<vdfTrackSourceInterface>();
  if (ti)
    {
    QTE_D(vdfTrackReader);

#define CONNECT(signal, slot, signature) \
  connect(ti, SIGNAL(signal signature), d, SLOT(slot signature))

    CONNECT(trackNameAvailable, setTrackName, (vdfTrackId, QString));
    CONNECT(trackClassificationAvailable, setTrackClassification,
            (vdfTrackId, vvTrackObjectClassification));

    CONNECT(trackUpdated, setTrackState, (vdfTrackId, vvTrackState));
    CONNECT(trackUpdated, setTrackStates, (vdfTrackId, QList<vvTrackState>));
#undef CONNECT

    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
const QObject* vdfTrackReader::dataReceiver() const
{
  QTE_D_CONST(vdfTrackReader);
  return d;
}
