// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsTrackTreeModel.h"

#include <QDebug>
#include <QIcon>
#include <QPalette>

#include <qtUtil.h>

#include <vgSwatchCache.h>
#include <vgUnixTime.h>

#include <vtkVgTrack.h>
#include <vtkVgTrackFilter.h>
#include <vtkVgTypeDefs.h>

#include <algorithm>
#include <vector>

#include <vsDisplayInfo.h>

#include "vsCore.h"
#include "vsScene.h"

//-----------------------------------------------------------------------------
vsTrackTreeModel::vsTrackTreeModel(
  vsCore* core, vsScene* scene, vtkVgTrackFilter* trackFilter,
  QObject* parent)
  : QAbstractItemModel(parent), Core(core), Scene(scene),
    trackFilter(trackFilter), swatchCache(core->swatchCache()),
    isSorted(true)
{
  connect(scene, SIGNAL(trackSceneUpdated()), this, SLOT(update()));
}

//-----------------------------------------------------------------------------
vsTrackTreeModel::~vsTrackTreeModel()
{
}

//-----------------------------------------------------------------------------
int vsTrackTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return NumColumns;
}

//-----------------------------------------------------------------------------
Qt::ItemFlags vsTrackTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return 0;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

//-----------------------------------------------------------------------------
QVariant vsTrackTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  vtkVgTrack* track = this->tracks[index.row()];
  switch (role)
    {
    case Qt::DisplayRole:
      switch (index.column())
        {
        case IdColumn:
          return track->GetId();

        case NameColumn:
          return track->GetName();

        case TrackTypeColumn:
          {
          int type = this->trackFilter->GetBestClassifier(track);
          switch (type)
            {
            case vtkVgTrack::Person:       return "Person";
            case vtkVgTrack::Vehicle:      return "Vehicle";
            case vtkVgTrack::Other:        return "Other";
            case vtkVgTrack::Unclassified: return "Unclassified";
            }
          return "(none)";
          }

        case ProbabilityColumn:
          {
          int type = this->trackFilter->GetBestClassifier(track);

          double probability;
          if (type == -1 || type == vtkVgTrack::Unclassified)
            {
            probability = 0.0;
            }
          else
            {
            probability = track->GetPVO()[type];
            }

          return QString::number(probability, 'f', 4);
          }

        case StartTimeColumn:
          return track->IsStarted()
            ? vgUnixTime(track->GetStartFrame().GetTime()).timeString()
            : QString("unknown");

        case EndTimeColumn:
          return track->IsStarted()
              ? vgUnixTime(track->GetEndFrame().GetTime()).timeString()
              : QString("unknown");

        case NoteColumn:
          return QString::fromLocal8Bit(track->GetNote());
        }
      break;

    case Qt::DecorationRole:
      if (index.column() == TrackTypeColumn)
        {
        const vsDisplayInfo& di = this->Scene->trackDisplayInfo(track->GetId());
        if (di.Color.isValid())
          {
          QColor color = di.Color.toQColor();
          color.setAlphaF(di.Visible ? 1.0 : 0.5);
          return this->swatchCache.swatch(color);
          }
        return this->swatchCache.swatch(Qt::transparent);
        }
      else if (index.column() == StarColumn)
        {
        const QString name = (track->IsStarred() ? "star" : "star-off");
        return qtUtil::standardIcon(name, 16);
        }
      break;

    case Qt::ForegroundRole:
      // Gray out filtered and hidden tracks
      if (this->isIndexHidden(index))
        return QPalette().brush(QPalette::Disabled, QPalette::WindowText);
      break;

    case Qt::TextAlignmentRole:
      switch (index.column())
        {
        case StarColumn:
          return Qt::AlignCenter;
        case ProbabilityColumn:
          return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
      break;

    case Qt::ToolTipRole:
      switch (index.column())
        {
        case TrackTypeColumn:
        case ProbabilityColumn:
          {
          double PVO[3];
          track->GetPVO(PVO);

          QString tooltip = "P:%1, V:%2, O:%3";

          return tooltip.arg(PVO[0], 0, 'f', 2)
                        .arg(PVO[1], 0, 'f', 2)
                        .arg(PVO[2], 0, 'f', 2);
          }

        case NoteColumn:
          {
          const char* note = track->GetNote();
          if (note)
            {
            return QString::fromLocal8Bit(note);
            }
          break;
          }
        }
      break;

    case LogicalIdRole:
      return QVariant::fromValue(
               this->Core->logicalTrackId(track->GetId()));

    case DisplayStateRole:
      return this->Scene->trackDisplayState(track->GetId());

    case StarRole:
      return track->IsStarred();
    }

  return QVariant();
}

//-----------------------------------------------------------------------------
QVariant vsTrackTreeModel::headerData(
  int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal)
    {
    if (role == Qt::DisplayRole)
      {
      switch (static_cast<ModelColumn>(section))
        {
        case NameColumn:        return "Id";
        case TrackTypeColumn:   return "Track Type";
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
bool vsTrackTreeModel::setData(
  const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid())
    return false;

  switch (role)
    {
    default:
      return false;

    case Qt::DisplayRole:
      if (index.column() == NoteColumn)
        {
        vtkVgTrack* track = this->tracks[index.row()];
        emit this->trackNoteChanged(track->GetId(), value.toString());
        break;
        }

    case DisplayStateRole:
      this->setTrackDisplayState(index, value.toBool());
      break;

    case StarRole:
      {
      vtkVgTrack* const track = this->tracks[index.row()];
      value.toBool() ? track->SetFlags(vtkVgTrack::TF_Starred)
                     : track->ClearFlags(vtkVgTrack::TF_Starred);
      break;
      }
    }

  emit this->dataChanged(index, index);
  return true;
}

//-----------------------------------------------------------------------------
QModelIndex vsTrackTreeModel::index(
  int row, int column, const QModelIndex& parent) const
{
  if (!this->hasIndex(row, column, parent))
    return QModelIndex();

  return this->createIndex(row, column);
}

//-----------------------------------------------------------------------------
QModelIndex vsTrackTreeModel::parent(const QModelIndex& /*index*/) const
{
  return QModelIndex();
}

//-----------------------------------------------------------------------------
int vsTrackTreeModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    return this->tracks.size();

  return 0;
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::addTrack(vtkVgTrack* track)
{
  if (this->addedTracks.isEmpty() && this->updatedTracks.isEmpty())
    {
    QMetaObject::invokeMethod(this, "deferredUpdateTracks",
                              Qt::QueuedConnection);
    }
  this->addedTracks.insert(track->GetId(), track);
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::updateTrack(vtkVgTrack* track)
{
  if (this->addedTracks.isEmpty() && this->updatedTracks.isEmpty())
    {
    QMetaObject::invokeMethod(this, "deferredUpdateTracks",
                              Qt::QueuedConnection);
    }

  this->updatedTracks.append(track);
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::deferredUpdateTracks()
{
  bool changed = false;
  int minRow = this->rowCount() - 1;
  int maxRow = 0;

  // Flush any new additions first, as we might also have updates for the new
  // tracks
  if (!this->addedTracks.isEmpty())
    {
    int count = this->addedTracks.count();
    int row = this->rowCount();
    this->beginInsertRows(QModelIndex(), row, row + count - 1);

    typedef QMap<vtkIdType, vtkVgTrack*>::const_iterator Iterator;
    foreach_iter (Iterator, iter, this->addedTracks)
      {
      vtkVgTrack* prev = this->tracks.isEmpty() ? 0 : this->tracks.back();
      this->tracks.append(iter.value());

      // Check if ordering has been violated... we assume that the ID of a
      // track will not change once it has been added, so this check should be
      // sufficient
      if (prev && this->isSorted && iter.key() < prev->GetId())
        {
        this->isSorted = false;
        qWarning() << "Track tree IDs are not in sorted order; "
                      "falling back to linear search";
        }
      }

    this->addedTracks.clear();
    this->endInsertRows();
    }

  // Update track pointers and compute the model rows covered by the tracks
  foreach (vtkVgTrack* track, this->updatedTracks)
    {
    QModelIndex index = this->indexOfTrack(track->GetId());
    if (!index.isValid())
      {
      continue;
      }

    changed = true;
    const int row = index.row();
    this->tracks[row] = track;

    minRow = qMin(row, minRow);
    maxRow = qMax(row, maxRow);
    }

  this->updatedTracks.clear();

  if (changed)
    {
    emit this->dataChanged(this->index(minRow, 0),
                           this->index(maxRow, NumColumns - 1));
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::removeTrack(vtkIdType trackId)
{
  // Check if the track is still waiting to be added
  if (this->addedTracks.remove(trackId))
    {
    // Yes; nothing to do besides removing it from pending additions
    return;
    }

  QModelIndex index = this->indexOfTrack(trackId);
  if (index.isValid())
    {
    int row = index.row();
    this->beginRemoveRows(QModelIndex(), row, row);
    this->tracks.removeAt(row);
    this->endRemoveRows();
    }
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::update()
{
  // If the track model has been updated, conservatively assume that every
  // track has been modified
  emit this->dataChanged(this->index(0, 0),
                         this->index(this->rowCount() - 1, NumColumns - 1));
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::setTrackDisplayState(
  const QModelIndex& index, bool show)
{
  if (index.column() != 0)
    return;

  const vtkIdType trackId = this->tracks[index.row()]->GetId();
  this->Scene->setTrackDisplayState(trackId, show);
}

//-----------------------------------------------------------------------------
bool vsTrackTreeModel::isIndexHidden(const QModelIndex& index) const
{
  vtkVgTrack* track = this->tracks[index.row()];
  return !this->Scene->trackVisibility(track->GetId());
}

//-----------------------------------------------------------------------------
struct compareTrackIds
{
  bool operator()(vtkVgTrack* a, vtkIdType b)
    { return a->GetId() < b; }
  bool operator()(vtkIdType a, vtkVgTrack* b)
    { return a < b->GetId(); }
};

//-----------------------------------------------------------------------------
QModelIndex vsTrackTreeModel::indexOfTrack(vtkIdType trackId) const
{
  if (this->isSorted)
    {
    // We can do binary search
    const QList<vtkVgTrack*>::const_iterator iter =
      qBinaryFind(this->tracks.constBegin(), this->tracks.constEnd(),
                  trackId, compareTrackIds());
    if (iter != this->tracks.constEnd())
      {
      return this->index(iter - this->tracks.constBegin(), 0);
      }
    }
  else
    {
    // Not sorted by id, fall back to linear search
    for (int i = 0, k = this->tracks.count(); i < k; ++i)
      {
      if (this->tracks[i]->GetId() == trackId)
        return this->index(i, 0);
      }
    }

  qDebug() << "Track" << trackId << "not found in tree";
  return QModelIndex();
}
