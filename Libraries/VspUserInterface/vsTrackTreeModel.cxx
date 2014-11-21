/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsTrackTreeModel.h"

#include <QDebug>
#include <QPalette>

#include <vgSwatchCache.h>
#include <vgUnixTime.h>

#include <vtkVgTrack.h>
#include <vtkVgTrackFilter.h>
#include <vtkVgTypeDefs.h>

#include <algorithm>
#include <vector>

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
          return vgUnixTime(track->GetStartFrame().GetTime()).timeString();

        case EndTimeColumn:
          return vgUnixTime(track->GetEndFrame().GetTime()).timeString();
        }
      break;

    case Qt::DecorationRole:
      if (index.column() == TrackTypeColumn)
        {
        int type = this->trackFilter->GetBestClassifier(track);
        if (type == -1)
          {
          // If the track is filtered out, don't show a color swatch
          return QVariant();
          }
        QColor color;
        if (this->colorHelper)
          {
          color = this->colorHelper->color(track);
          }
        if (!color.isValid())
          {
          color = this->colors[type];
          }
        color.setAlphaF(this->isIndexHidden(index) ? 0.5 : 1.0);
        return this->swatchCache.swatch(color);
        }
      break;

    case Qt::ForegroundRole:
      // Gray out filtered and hidden tracks
      if (this->isIndexHidden(index))
        return QPalette().brush(QPalette::Disabled, QPalette::WindowText);
      break;

    case Qt::TextAlignmentRole:
      if (index.column() == ProbabilityColumn)
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
      break;

    case Qt::ToolTipRole:
      if (index.column() == TrackTypeColumn ||
          index.column() == ProbabilityColumn)
        {
        double PVO[3];
        track->GetPVO(PVO);

        QString tooltip = "P:%1, V:%2, O:%3";

        return tooltip.arg(PVO[0], 0, 'f', 2)
                      .arg(PVO[1], 0, 'f', 2)
                      .arg(PVO[2], 0, 'f', 2);
        }
      break;

    case LogicalIdRole:
      return QVariant::fromValue(
               this->Core->logicalTrackId(track->GetId()));

    case DisplayStateRole:
      return this->Scene->trackDisplayState(track->GetId());
    }

  return QVariant();
}

//-----------------------------------------------------------------------------
QVariant vsTrackTreeModel::headerData(
  int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
    switch (static_cast<ModelColumn>(section))
      {
      case NameColumn:        return "Id";
      case TrackTypeColumn:   return "Track Type";
      case ProbabilityColumn: return "Probability";
      case StartTimeColumn:   return "Start Time";
      case EndTimeColumn:     return "End Time";
      default: break;
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

    case DisplayStateRole:
      this->setTrackDisplayState(index, value.toBool());
      break;
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
  if (this->addedTracks.isEmpty())
    {
    QMetaObject::invokeMethod(this, "deferredAddTracks",
                              Qt::QueuedConnection);
    }
  this->addedTracks.insert(track->GetId(), track);
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::deferredAddTracks()
{
  if (this->addedTracks.isEmpty())
    {
    return;
    }

  int count = this->addedTracks.count();
  int row = this->rowCount();
  this->beginInsertRows(QModelIndex(), row, row + count - 1);

  typedef QMap<vtkIdType, vtkVgTrack*>::const_iterator Iterator;
  foreach_iter (Iterator, iter, this->addedTracks)
    {
    vtkVgTrack* prev = this->tracks.isEmpty() ? 0 : this->tracks.back();
    this->tracks.append(iter.value());

    // Check if ordering has been violated. We assume the id of a track will
    // not change once it has been added, so this check should be sufficient.
    if (prev && this->isSorted && iter.key() < prev->GetId())
      {
      this->isSorted = false;
      qWarning() << "Track tree ids are not in sorted order, "
                    "falling back to linear search";
      }
    }

  this->addedTracks.clear();
  this->endInsertRows();
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::updateTrack(vtkVgTrack* track)
{
  if (this->updatedTracks.isEmpty())
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

//-----------------------------------------------------------------------------
void vsTrackTreeModel::setColor(int type, const QColor& color)
{
  this->colors[type] = color;
}

//-----------------------------------------------------------------------------
void vsTrackTreeModel::setColorHelper(vsTrackTreeColorHelper* helper)
{
  this->colorHelper = helper;
}
