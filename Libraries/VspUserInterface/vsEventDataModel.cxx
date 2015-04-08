/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsEventDataModel.h"

#include "vsScene.h"

#include <vgfNamespace.h>

#include <vgCheckArg.h>

#include <vtkVgEvent.h>

#include <QDebug>

namespace // anonymous
{

//-----------------------------------------------------------------------------
struct EventData
{
  vsEventId Id;
  vtkVgEvent* Event;
};

//-----------------------------------------------------------------------------
struct CompareEventIds
{
  bool operator()(const EventData& a, vtkIdType b)
    { return a.Id.GlobalId < b; }
  bool operator()(vtkIdType a, const EventData& b)
    { return a < b.Id.GlobalId; }
};

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vsEventDataModelPrivate
{
private:
  QTE_DECLARE_PUBLIC_PTR(vsEventDataModel)
  QTE_DECLARE_PUBLIC(vsEventDataModel)

public:
  vsEventDataModelPrivate(vsEventDataModel* q, vsScene* scene) :
    q_ptr(q), Scene(scene), UpdatePosted(false) {}

  int indexOfEvent(vtkIdType eventId) const;

  void postUpdate();

public:
  vsScene* const Scene;

  QList<EventData> Data;

  bool UpdatePosted;
  QMap<vtkIdType, EventData> PendingAdditions;
  QList<vtkVgEvent*> PendingUpdates;
};

QTE_IMPLEMENT_D_FUNC(vsEventDataModel)

//-----------------------------------------------------------------------------
int vsEventDataModelPrivate::indexOfEvent(vtkIdType eventId) const
{
  const QList<EventData>::const_iterator iter =
    qBinaryFind(this->Data.constBegin(), this->Data.constEnd(),
                eventId, CompareEventIds());
  if (iter != this->Data.constEnd())
    {
    return iter - this->Data.constBegin();
    }

  qDebug() << "Event" << eventId << "not found in model";
  return -1;
}

//-----------------------------------------------------------------------------
void vsEventDataModelPrivate::postUpdate()
{
  if (!this->UpdatePosted)
    {
    QTE_Q(vsEventDataModel);
    this->UpdatePosted = true;
    QMetaObject::invokeMethod(q, "processUpdates", Qt::QueuedConnection);
    }
}

//-----------------------------------------------------------------------------
vsEventDataModel::vsEventDataModel(vsScene* scene, QObject* parent) :
  QAbstractItemModel(parent), d_ptr(new vsEventDataModelPrivate(this, scene))
{
  connect(scene, SIGNAL(eventSceneUpdated()), this, SLOT(invalidate()));
}

//-----------------------------------------------------------------------------
vsEventDataModel::~vsEventDataModel()
{
}

//-----------------------------------------------------------------------------
QVariant vsEventDataModel::data(const QModelIndex& index, int role) const
{
  QTE_D_CONST(vsEventDataModel);

  CHECK_ARG(index.isValid(), QVariant());
  CHECK_ARG(index.row() >= 0 && index.row() < d->Data.count(), QVariant());

  const EventData& data = d->Data[index.row()];
  switch (role)
    {
    case vgf::ItemTypeRole:
      return QVariant::fromValue(vgf::EventItem);

    case vgf::NameRole:
      {
      // If event has a name...
      const char* const name = data.Event->GetName();
      if (name)
        {
        // ...then use that...
        return QString::fromLocal8Bit(name);
        }
      // ...else show the (internal) ID in parentheses (temporary?)
      return QString("(%1)").arg(data.Id.GlobalId);
      }

    case vgf::IdentityRole:
      return data.Id.GlobalId;

    case vgf::LogicalIdentityRole:
      return QVariant::fromValue(data.Id);

    case vgf::UniqueIdentityRole:
      return QVariant::fromValue(data.Id.UniqueId);

    case vgf::VisibilityRole:
      return d->Scene->eventVisibility(data.Id.GlobalId);

    case vgf::UserVisibilityRole:
      return d->Scene->eventDisplayState(data.Id.GlobalId);

    case vgf::StartTimeRole:
      return QVariant::fromValue(data.Event->GetStartFrame().GetRawTimeStamp());

    case vgf::EndTimeRole:
      return QVariant::fromValue(data.Event->GetEndFrame().GetRawTimeStamp());

    case vgf::RatingRole:
      return data.Event->GetStatus();

    case vgf::StarRole:
      return data.Event->IsStarred();

    case vgf::NoteRole:
      {
      const char* note = data.Event->GetNote();
      if (note && *note)
        {
        return QString::fromLocal8Bit(note);
        }
      return QVariant();
      }

    case vgf::ClassificationRole:
      return QVariant(); // TODO

    case vgf::ConfidenceRole:
      return QVariant(); // TODO

    default:
      return QVariant();
    }
}

//-----------------------------------------------------------------------------
bool vsEventDataModel::setData(
  const QModelIndex& index, const QVariant& value, int role)
{
  // TODO
  Q_UNUSED(index)
  Q_UNUSED(value)
  Q_UNUSED(role)
  return false;
}

//-----------------------------------------------------------------------------
QModelIndex vsEventDataModel::index(
  int row, int column, const QModelIndex& parent) const
{
  if (!this->hasIndex(row, column, parent))
    {
    return QModelIndex();
    }

  return this->createIndex(row, column);
}

//-----------------------------------------------------------------------------
QModelIndex vsEventDataModel::parent(const QModelIndex& /*index*/) const
{
  return QModelIndex();
}

//-----------------------------------------------------------------------------
int vsEventDataModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    {
    QTE_D_CONST(vsEventDataModel);
    return d->Data.count();
    }

  return 0;
}

//-----------------------------------------------------------------------------
int vsEventDataModel::columnCount(const QModelIndex& /*parent*/) const
{
  // NOTE: This needs to be small enough that QSortFilterProxyModel can reserve
  //       a QVector<int> of this size, but will effectively truncate the
  //       columns of any display model, so should be at least as large as the
  //       most columns any display model might have
  return 64;
}

//-----------------------------------------------------------------------------
void vsEventDataModel::addEvent(vtkVgEvent* event, const vsEventId& eventId)
{
  QTE_D(vsEventDataModel);

  EventData data = { eventId, event };
  d->PendingAdditions.insert(eventId.GlobalId, data);
  d->postUpdate();
}

//-----------------------------------------------------------------------------
void vsEventDataModel::updateEvent(vtkVgEvent* event)
{
  QTE_D(vsEventDataModel);

  d->PendingUpdates.append(event);
  d->postUpdate();
}

//-----------------------------------------------------------------------------
void vsEventDataModel::removeEvent(vtkIdType eventId)
{
  QTE_D(vsEventDataModel);

  // Check if the event is still waiting to be added
  if (d->PendingAdditions.remove(eventId))
    {
    // Yes; nothing to do besides removing it from pending additions
    return;
    }

  const int row = d->indexOfEvent(eventId);
  if (row >= 0)
    {
    this->beginRemoveRows(QModelIndex(), row, row);
    d->Data.removeAt(row);
    this->endRemoveRows();
    }
}

//-----------------------------------------------------------------------------
void vsEventDataModel::processUpdates()
{
  QTE_D(vsEventDataModel);

  // First add any events that need to be inserted into the middle of the data
  // array
  if (!d->Data.isEmpty())
    {
    const int lastId = d->Data.back().Id.GlobalId;
    QMap<vtkIdType, EventData>::iterator iter;
    for (iter = d->PendingAdditions.begin();
         !d->PendingAdditions.isEmpty() && iter.key() < lastId;
         iter = d->PendingAdditions.erase(iter))
      {
      // Find the index where this event should be inserted to maintain sorting
      const QList<EventData>::iterator insertPosition =
        qLowerBound(d->Data.begin(), d->Data.end(),
                    iter.key(), CompareEventIds());
      const int insertRow = insertPosition - d->Data.begin();

      // Insert the new row; we expect non-tail insertions to be unusual, and
      // consecutive non-tail insertions even more so, and so don't try to
      // combine such insertions into batch operations
      this->beginInsertRows(QModelIndex(), insertRow, insertRow);
      d->Data.insert(insertPosition, iter.value());
      this->endInsertRows();
      }
    }

  // Insert remaining new events
  if (!d->PendingAdditions.isEmpty())
    {
    int row = d->Data.count();
    const int newEventCount = d->PendingAdditions.count();
    this->beginInsertRows(QModelIndex(), row, row + newEventCount - 1);

    foreach (const EventData& newEvent, d->PendingAdditions)
      {
      d->Data.append(newEvent);
      }
    d->PendingAdditions.clear();
    this->endInsertRows();
    }

  // Update modified events
  if (!d->PendingUpdates.isEmpty())
    {
    QList<int> modifiedRows;

    // Update event pointers and collect set of affected model rows
    foreach (vtkVgEvent* const event, d->PendingUpdates)
      {
      const int row = d->indexOfEvent(event->GetId());
      if (row >= 0)
        {
        modifiedRows.append(row);
        d->Data[row].Event = event;
        }
      }

    d->PendingUpdates.clear();

    // Determine what blocks of rows have changed and emit notification thereof
    if (!modifiedRows.isEmpty())
      {
      qSort(modifiedRows.begin(), modifiedRows.end());

      int first = modifiedRows.takeFirst();
      int last = first;

      foreach (int row, modifiedRows)
        {
        if (row != last + 1)
          {
          emit this->dataChanged(this->index(first, 0),
                                 this->index(last, this->columnCount() - 1));
          first = row;
          }
        last = row;
        }
      emit this->dataChanged(this->index(first, 0),
                             this->index(last, this->columnCount() - 1));
      }
    }

  // Done processing updates
  d->UpdatePosted = false;
}

//-----------------------------------------------------------------------------
void vsEventDataModel::invalidate()
{
  // If the event model has been updated, conservatively assume that every
  // event has been modified
  emit this->dataChanged(this->index(0, 0),
                         this->index(this->rowCount() - 1,
                                     this->columnCount() - 1));
}
