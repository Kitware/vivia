// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsRandomAlertDescriptor.h"

#include <QHash>
#include <QList>

#include <qtRand.h>

#include <vsTrackId.h>
#include <vsTrackState.h>
#include <vtkVsTrackInfo.h>

#include <vsLiveDescriptorPrivate.h>

//BEGIN vsRandomAlertDescriptorPrivate

//-----------------------------------------------------------------------------
class vsRandomAlertDescriptorPrivate : public vsLiveDescriptorPrivate
{
public:
  vsRandomAlertDescriptorPrivate(vsRandomAlertDescriptor*);
  virtual ~vsRandomAlertDescriptorPrivate();

protected:
  virtual void run() QTE_OVERRIDE;

  virtual void injectInput(qint64 id, vsDescriptorInputPtr input) QTE_OVERRIDE;
  virtual void revokeInput(qint64 id, bool revokeEvents) QTE_OVERRIDE;
  virtual void revokeAllInput(bool revokeEvents) QTE_OVERRIDE;

  typedef QHash<vsTrackId, vtkVgTimeStamp> PendingAlertList;
  typedef QHash<qint64, PendingAlertList> PendingAlertMap;

  QHash<qint64, int> Queries;
  QHash<qint64, QList<vtkIdType> > Events;
  PendingAlertMap PendingAlerts;
  vtkIdType NextEventId;

private:
  QTE_DECLARE_PUBLIC(vsRandomAlertDescriptor)
};

//-----------------------------------------------------------------------------
vsRandomAlertDescriptorPrivate::vsRandomAlertDescriptorPrivate(
  vsRandomAlertDescriptor* q)
  : vsLiveDescriptorPrivate(q), NextEventId(0)
{
}

//-----------------------------------------------------------------------------
vsRandomAlertDescriptorPrivate::~vsRandomAlertDescriptorPrivate()
{
}

//-----------------------------------------------------------------------------
void vsRandomAlertDescriptorPrivate::run()
{
  QTE_Q(vsRandomAlertDescriptor);
  emit q->readyForInput(q);
  vsLiveDescriptorPrivate::run();
}

//-----------------------------------------------------------------------------
void vsRandomAlertDescriptorPrivate::injectInput(
  qint64 id, vsDescriptorInputPtr input)
{
  // New query?
  if (input->type() == vsDescriptorInput::Query)
    {
    // If the input is valid...
    const int* qi = input->queryId();
    if (qi)
      {
      // ...then add the query to our map and abort pending suicide (if set)
      this->Queries.insert(id, *qi);
      this->cancelSuicideTimer();
      }
    }
  // If not, check first if we have any queries, since it is pointless to do
  // anything otherwise
  else if (this->Queries.count())
    {
    // Track update? Random threshold exceeded?
    if (input->type() == vsDescriptorInput::TrackUpdate && qtRandD() < 0.05)
      {
      // Check if input is valid
      const vsTrackId* trackId = input->trackId();
      const vsTrackState* state = input->trackState();
      if (trackId && state)
        {
        // Pick a query at random
        int queryCount = this->Queries.keys().count();
        int queryIdIndex = static_cast<int>(qtRandD() * queryCount);
        qint64 queryId = this->Queries.keys().at(queryIdIndex);
        // Do we have a pending alert?
        if (this->PendingAlerts.contains(queryId) &&
            this->PendingAlerts[queryId].contains(*trackId))
          {
          // Yes; generate an event
          int classifier = this->Queries.value(queryId);
          QList<vtkIdType>& queryEvents = this->Events[queryId];
          PendingAlertList& pal = this->PendingAlerts[queryId];
          vsEvent eventBase = vsEvent(QUuid::createUuid());
          vtkIdType eventId = this->NextEventId++;
          eventBase->SetId(eventId);
          eventBase->AddClassifier(classifier, qtRandD(), 0.0);
          vtkVsTrackInfo* ti =
            new vtkVsTrackInfo(*trackId, pal.value(*trackId), state->time);
          eventBase->AddTrack(ti);
          queryEvents.append(eventId);
          emit this->eventAvailable(eventBase);
          }
        else
          {
          // No; add a pending alert
          PendingAlertList& pal = this->PendingAlerts[queryId];
          pal.insert(*trackId, state->time);
          }
        }
      }
    // Track closure?
    else if (input->type() == vsDescriptorInput::TrackClosure)
      {
      // Check if input is valid
      const vsTrackId* trackId = input->trackId();
      const vsTrackState* state = input->trackState();
      if (trackId && state)
        {
        // Cancel any pending alerts for the track
        foreach_iter (PendingAlertMap::iterator, iter, this->PendingAlerts)
          {
          iter->remove(*trackId);
          }
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vsRandomAlertDescriptorPrivate::revokeInput(qint64 id, bool revokeEvents)
{
  this->Queries.remove(id);
  this->PendingAlerts.remove(id);

  if (revokeEvents && this->Events.contains(id))
    {
    QList<vtkIdType> queryEvents = this->Events.take(id);
    foreach (vtkIdType eventId, queryEvents)
      {
      emit this->eventRevoked(eventId);
      }
    }

  if (this->Queries.isEmpty() && this->Events.isEmpty())
    {
    // If we have no registered queries left, set a timer to suicide rather
    // than hang around inactive forever
    this->setSuicideTimer();
    }
}

//-----------------------------------------------------------------------------
void vsRandomAlertDescriptorPrivate::revokeAllInput(bool revokeEvents)
{
  if (revokeEvents)
    {
    foreach (const QList<vtkIdType>& queryEvents, this->Events)
      {
      foreach (vtkIdType eventId, queryEvents)
        {
        emit this->eventRevoked(eventId);
        }
      }
    this->Events.clear();
    this->setSuicideTimer();
    }
  this->Queries.clear();
  this->PendingAlerts.clear();
}

//END vsRandomAlertDescriptorPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsRandomAlertDescriptor

//-----------------------------------------------------------------------------
vsRandomAlertDescriptor::vsRandomAlertDescriptor() :
  vsLiveDescriptor(new vsRandomAlertDescriptorPrivate(this))
{
}

//-----------------------------------------------------------------------------
vsRandomAlertDescriptor::~vsRandomAlertDescriptor()
{
}

//-----------------------------------------------------------------------------
vsDescriptorInput::Types vsRandomAlertDescriptor::inputAccepted() const
{
  return vsDescriptorInput::TrackUpdate | vsDescriptorInput::TrackClosure |
         vsDescriptorInput::Query |
         vsLiveDescriptor::inputAccepted();
}

//-----------------------------------------------------------------------------
QString vsRandomAlertDescriptor::name() const
{
  return "random alert";
}

//END vsRandomAlertDescriptor
