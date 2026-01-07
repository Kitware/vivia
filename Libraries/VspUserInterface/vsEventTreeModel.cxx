// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsEventTreeModel.h"

#include <QDebug>
#include <QMimeData>
#include <QPalette>

#include <qtUtil.h>

#include <vgEventType.h>
#include <vgSwatchCache.h>
#include <vgUnixTime.h>

#include <vtkVgEvent.h>
#include <vtkVgEventFilter.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgTypeDefs.h>

#include <algorithm>
#include <vector>

#include <vsDisplayInfo.h>

#include "vsEventStatus.h"
#include "vsScene.h"

namespace // anonymous
{

bool CompareClassifiers(const vtkVgEventFilter::ScoredClassifier& a,
                        const vtkVgEventFilter::ScoredClassifier& b)
{
  return a.second > b.second;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vsEventTreeModel::vsEventTreeModel(
  vsScene* scene, vtkVgEventFilter* eventFilter,
  vtkVgEventTypeRegistry* eventTypeRegistry, const vgSwatchCache& swatchCache,
  QObject* parent)
  : QAbstractItemModel(parent), Scene(scene), eventFilter(eventFilter),
    eventTypeRegistry(eventTypeRegistry), swatchCache(swatchCache),
    isSorted(true)
{
  this->yesIcon = qtUtil::standardIcon("okay", 16);
  this->noIcon = qtUtil::standardIcon("cancel", 16);

  connect(scene, SIGNAL(eventSceneUpdated()), this, SLOT(update()));
}

//-----------------------------------------------------------------------------
vsEventTreeModel::~vsEventTreeModel()
{
}

//-----------------------------------------------------------------------------
int vsEventTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return NumColumns;
}

//-----------------------------------------------------------------------------
Qt::ItemFlags vsEventTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemIsDropEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled |
         Qt::ItemIsDropEnabled;
}

//-----------------------------------------------------------------------------
QVariant vsEventTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  vsEventUserInfo vsEventUserInfo = this->events[index.row()];
  vtkVgEvent* event = vsEventUserInfo.Event;
  switch (role)
    {
    case Qt::DisplayRole:
      switch (index.column())
        {
        case IdColumn:
          return event->GetId();

        case NameColumn:
          // If event has a name...
          if (event->GetName())
            {
            // ...then use that
            return QString::fromLocal8Bit(event->GetName());
            }
          else
            {
            // ...else show the ID in parentheses (temporary?)
            return QString("(%1)").arg(event->GetId());
            }

        case EventTypeColumn:
          {
          int type = this->eventFilter->GetBestClassifier(event);
          if (type == -1)
            {
            return QString("(none)");
            }
          return this->eventTypeRegistry->GetTypeById(type).GetName();
          }

        case ProbabilityColumn:
          {
          int type = this->eventFilter->GetBestClassifier(event);
          double probability =
            (type == -1 ? 0.0 : event->GetProbability(type));
          return QString::number(probability, 'f', 4);
          }

        case StartTimeColumn:
          return vgUnixTime(event->GetStartFrame().GetTime()).timeString();

        case EndTimeColumn:
          return vgUnixTime(event->GetEndFrame().GetTime()).timeString();

        case NoteColumn:
          return QString::fromLocal8Bit(event->GetNote());
        }
      break;

    case Qt::DecorationRole:
      if (index.column() == RatingColumn)
        {
        switch (event->GetStatus())
          {
          case vgObjectStatus::Adjudicated: return this->yesIcon;
          case vgObjectStatus::Excluded:    return this->noIcon;
          }
        }
      else if (index.column() == StarColumn)
        {
        const QString name = (event->IsStarred() ? "star" : "star-off");
        return qtUtil::standardIcon(name, 16);
        }
      else if (index.column() == EventTypeColumn)
        {
        const vsDisplayInfo& di = this->Scene->eventDisplayInfo(event->GetId());
        if (di.Color.isValid())
          {
          QColor color = di.Color.toQColor();
          color.setAlphaF(di.Visible ? 1.0 : 0.5);
          return this->swatchCache.swatch(color);
          }
        return this->swatchCache.swatch(Qt::transparent);
        }
      break;

    case Qt::ForegroundRole:
      // Gray out filtered and hidden events
      if (this->isIndexHidden(index))
        return QPalette().brush(QPalette::Disabled, QPalette::WindowText);
      break;

    case Qt::TextAlignmentRole:
      switch (index.column())
        {
        case StarColumn:
          return Qt::AlignCenter;
        case ProbabilityColumn:
        case NameColumn: // temporary?
          return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
      break;

    case Qt::ToolTipRole:
      switch (index.column())
        {
        case RatingColumn:
          switch (event->GetStatus())
            {
            case vgObjectStatus::None:        return "Unrated";
            case vgObjectStatus::Adjudicated: return "Relevant";
            case vgObjectStatus::Excluded:    return "Not Relevant";
            }
          break;

        case EventTypeColumn:
        case ProbabilityColumn:
          {
          std::vector<vtkVgEventFilter::ScoredClassifier> classifiers =
            this->eventFilter->GetActiveClassifiers(event);

          if (classifiers.size() == 0)
            return QString("(none)");

          // sort by probability
          std::sort(classifiers.begin(), classifiers.end(),
                    CompareClassifiers);

          // build the tool tip string with more likely classifiers near the top
          QString tooltip = "<html><table>";
          for (size_t i = 0, end = classifiers.size(); i < end; ++i)
            {
            if (i != 0)
              tooltip.append('\n');

            const int type = classifiers[i].first;
            const double p = event->GetProbability(type);

            const QString str("<tr><td>%1</td><td align=\"right\">(%2)</td>");
            const char* name =
              this->eventTypeRegistry->GetTypeById(type).GetName();
            tooltip += str.arg(name).arg(p, 0, 'f', 4);
            }
          return tooltip + "</table></html>";
          }

        case NoteColumn:
          {
          const char* note = event->GetNote();
          if (note)
            {
            return QString::fromLocal8Bit(note);
            }
          break;
          }
        default:
          break;
        }
      break;

    case DisplayStateRole:
      return this->Scene->eventDisplayState(event->GetId());

    case RatingRole:
      return event->GetStatus();

    case StatusRole:
      return vsEventUserInfo.Status;

    case StarRole:
      return event->IsStarred();

    case NameSortRole:
      if (event->GetName())
        {
        return QString::fromLocal8Bit(event->GetName()).toInt();
        }
      return event->GetId();

    case ModifiableRole:
      return event->IsModifiable();
    }

  return QVariant();
}

//-----------------------------------------------------------------------------
QVariant vsEventTreeModel::headerData(
  int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal)
    {
    if (role == Qt::DisplayRole)
      {
      switch (section)
        {
        case RatingColumn:      return "";
        case NameColumn:        return "Id";
        case EventTypeColumn:   return "Event Type";
        case ProbabilityColumn: return "Probability";
        case StartTimeColumn:   return "Start Time";
        case EndTimeColumn:     return "End Time";
        case NoteColumn:        return "Note";
        default: break;
        }
      }
    else if (role == Qt::DecorationRole && section == StarColumn)
      {
      if (section == StarColumn)
        {
        return qtUtil::standardIcon("star", 16);
        }
      }
    else if (role == Qt::ToolTipRole)
      {
      if (section == StarColumn)
        {
        return "Starred";
        }
      }
    }

  return QVariant();
}

//-----------------------------------------------------------------------------
bool vsEventTreeModel::setData(
  const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid())
    return false;

  switch (role)
    {
    case Qt::DisplayRole:
      if (index.column() == NoteColumn)
        {
        vtkVgEvent* event = this->events[index.row()].Event;
        emit this->eventNoteChanged(event->GetId(), value.toString());
        break;
        }

    default:
      return false;

    case DisplayStateRole:
      this->setEventDisplayState(index, value.toBool());
      break;

    case RatingRole:
      {
      vtkVgEvent* event = this->events[index.row()].Event;
      emit this->eventRatingChanged(event->GetId(), value.toInt());
      break;
      }

    case StatusRole:
      {
      vsEventUserInfo& eventInfo = this->events[index.row()];

      const int oldStatus = eventInfo.Status;
      const int newStatus = value.toInt();

      if (newStatus != oldStatus)
        {
        if (newStatus == vs::RejectedEvent &&
            oldStatus != vs::RejectedEvent)
          {
          // Auto-hide rejected events
          this->setData(index, false, DisplayStateRole);
          }
        else if (oldStatus == vs::RejectedEvent &&
                 newStatus != vs::RejectedEvent)
          {
          // Likewise re-show if moving out of rejected list
          this->setData(index, true, DisplayStateRole);
          }

        eventInfo.Status = newStatus;
        emit this->eventStatusChanged(eventInfo.Event->GetId(), newStatus);
        }
      break;
      }

    case StarRole:
      {
      vtkVgEvent* event = this->events[index.row()].Event;
      value.toBool() ? event->SetFlags(vtkVgEventBase::EF_Starred)
                     : event->ClearFlags(vtkVgEventBase::EF_Starred);
      break;
      }
    }

  emit this->dataChanged(index, index);
  return true;
}

//-----------------------------------------------------------------------------
QModelIndex vsEventTreeModel::index(
  int row, int column, const QModelIndex& parent) const
{
  if (!this->hasIndex(row, column, parent))
    return QModelIndex();

  return this->createIndex(row, column);
}

//-----------------------------------------------------------------------------
QModelIndex vsEventTreeModel::parent(const QModelIndex& /*index*/) const
{
  return QModelIndex();
}

//-----------------------------------------------------------------------------
int vsEventTreeModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    return this->events.size();

  return 0;
}

//-----------------------------------------------------------------------------
void vsEventTreeModel::addEvent(vtkVgEvent* event)
{
  if (this->addedEvents.isEmpty() && this->updatedEvents.isEmpty())
    {
    QMetaObject::invokeMethod(this, "deferredUpdateEvents",
                              Qt::QueuedConnection);
    }
  this->addedEvents.insert(event->GetId(), event);
}

//-----------------------------------------------------------------------------
void vsEventTreeModel::updateEvent(vtkVgEvent* event)
{
  if (this->addedEvents.isEmpty() && this->updatedEvents.isEmpty())
    {
    QMetaObject::invokeMethod(this, "deferredUpdateEvents",
                              Qt::QueuedConnection);
    }
  this->updatedEvents.append(event);
}

//-----------------------------------------------------------------------------
void vsEventTreeModel::deferredUpdateEvents()
{
  bool changed = false;
  int minRow = this->rowCount() - 1;
  int maxRow = 0;

  // Flush any new additions first, as we might also have updates for the new
  // events
  if (!this->addedEvents.isEmpty())
    {
    int count = this->addedEvents.count();
    int row = this->rowCount();
    this->beginInsertRows(QModelIndex(), row, row + count - 1);

    typedef QMap<vtkIdType, vtkVgEvent*>::const_iterator Iterator;
    foreach_iter (Iterator, iter, this->addedEvents)
      {
      vsEventUserInfo ei;
      ei.Event = iter.value();
      ei.Status = vgObjectStatus::None;

      vtkVgEvent* prev = this->events.isEmpty() ? 0 : this->events.back().Event;
      this->events.append(ei);

      // Check if ordering has been violated... we assume that the ID of an
      // event will not change once it has been added, so this check should be
      // sufficient
      if (prev && this->isSorted && iter.key() < prev->GetId())
        {
        this->isSorted = false;
        qWarning() << "Event tree ids are not in sorted order, "
                      "falling back to linear search";
        }
      }

    this->addedEvents.clear();
    this->endInsertRows();
    }

  // Update event pointers and compute the model rows covered by the events
  foreach (vtkVgEvent* event, this->updatedEvents)
    {
    QModelIndex index = this->indexOfEvent(event->GetId());
    if (!index.isValid())
      {
      continue;
      }

    changed = true;
    const int row = index.row();
    this->events[row].Event = event;

    minRow = qMin(row, minRow);
    maxRow = qMax(row, maxRow);
    }

  this->updatedEvents.clear();

  if (changed)
    {
    emit this->dataChanged(this->index(minRow, 0),
                           this->index(maxRow, NumColumns - 1));
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeModel::removeEvent(vtkIdType eventId)
{
  // Check if the event is still waiting to be added
  if (this->addedEvents.remove(eventId))
    {
    // Yes; nothing to do besides removing it from pending additions
    return;
    }

  QModelIndex index = this->indexOfEvent(eventId);
  if (index.isValid())
    {
    int row = index.row();
    this->beginRemoveRows(QModelIndex(), row, row);
    this->events.removeAt(row);
    this->endRemoveRows();
    }
}

//-----------------------------------------------------------------------------
void vsEventTreeModel::update()
{
  // If the event model has been updated, conservatively assume that every
  // event has been modified
  emit this->dataChanged(this->index(0, 0),
                         this->index(this->rowCount() - 1, NumColumns - 1));
}

//-----------------------------------------------------------------------------
void vsEventTreeModel::setEventDisplayState(
  const QModelIndex& index, bool show)
{
  if (index.column() != 0)
    return;

  const vtkIdType eventId = this->events[index.row()].Event->GetId();
  this->Scene->setEventDisplayState(eventId, show);
}

//-----------------------------------------------------------------------------
bool vsEventTreeModel::isIndexHidden(const QModelIndex& index) const
{
  vtkVgEvent* event = this->events[index.row()].Event;
  return !this->Scene->eventVisibility(event->GetId());
}

//-----------------------------------------------------------------------------
QMimeData* vsEventTreeModel::mimeData(const QModelIndexList& indexes) const
{
  QMimeData* mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  // Put the event id in the mime encoding since the event may not be in
  // the same row when it is dropped.
  foreach (QModelIndex index, indexes)
    {
    if (index.isValid() && index.column() == 0)
      stream << this->events[index.row()].Event->GetId();
    }

  mimeData->setData("application/vnd.kitware.event", encodedData);
  return mimeData;
}

//-----------------------------------------------------------------------------
QStringList vsEventTreeModel::mimeTypes() const
{
  QStringList types;
  types << "application/vnd.kitware.event";
  return types;
}

//-----------------------------------------------------------------------------
struct compareEventIds
{
  bool operator()(const vsEventUserInfo& a, vtkIdType b)
    { return a.Event->GetId() < b; }
  bool operator()(vtkIdType a, const vsEventUserInfo& b)
    { return a < b.Event->GetId(); }
};

//-----------------------------------------------------------------------------
QModelIndex vsEventTreeModel::indexOfEvent(vtkIdType eventId) const
{
  if (this->isSorted)
    {
    // We can do binary search
    const QList<vsEventUserInfo>::const_iterator iter =
      qBinaryFind(this->events.constBegin(), this->events.constEnd(),
                  eventId, compareEventIds());
    if (iter != this->events.constEnd())
      {
      return this->index(iter - this->events.constBegin(), 0);
      }
    }
  else
    {
    // Not sorted by id, fall back to linear search
    for (int i = 0, k = this->events.count(); i < k; ++i)
      {
      if (this->events[i].Event->GetId() == eventId)
        return this->index(i, 0);
      }
    }

  qDebug() << "Event" << eventId << "not found in tree";
  return QModelIndex();
}
