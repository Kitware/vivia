/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgfItemModel.h"

#include "vgfMetaTypes.h"
#include "vgfNamespace.h"
#include "vgfItemReference.h"

#include <vgUnixTime.h>

#include <qtUtil.h>

#include <vgCheckArg.h>

#include <QIcon>
#include <QPalette>

//-----------------------------------------------------------------------------
class vgfItemModelPrivate
{
public:
  vgfItemModelPrivate() : SortNeeded(false), ShowHidden(false) {}

  QIcon starIcon(bool) const;
  QIcon itemTypeIcon(int) const;

  QHash<int, int> ColumnDataMap;

  bool SortNeeded;
  bool ShowHidden;

  mutable QHash<bool, QIcon> StarIcons;
  mutable QHash<vgf::ItemType, QIcon> ItemTypeIcons;
};

QTE_IMPLEMENT_D_FUNC(vgfItemModel)

//-----------------------------------------------------------------------------
QIcon vgfItemModelPrivate::starIcon(bool state) const
{
  if (!this->StarIcons.contains(state))
    {
    const QString name = (state ? "star" : "star-off");
    this->StarIcons.insert(state, qtUtil::standardIcon(name, 16));
    }

  return this->StarIcons[state];
}

//-----------------------------------------------------------------------------
QIcon vgfItemModelPrivate::itemTypeIcon(int type) const
{
  vgf::ItemType typeEnum = static_cast<vgf::ItemType>(type);

  if (!this->ItemTypeIcons.contains(typeEnum))
    {
    this->ItemTypeIcons.insert(typeEnum, vgf::itemTypeIcon(typeEnum, 16));
    }

  return this->ItemTypeIcons[typeEnum];
}

//-----------------------------------------------------------------------------
vgfItemModel::vgfItemModel(QObject* parent) :
  QSortFilterProxyModel(parent), d_ptr(new vgfItemModelPrivate)
{
  // Our filtering (and likely that of our subclasses) is dependent on the
  // logical data model's data; therefore, we need to re-filter and/or re-sort
  // when the underlying data changes, and so we enable doing so by default
  this->setDynamicSortFilter(true);

  // Work around Qt bug 35180 - QSortFilterProxyModel does not sort items when
  // they become visible after initially all items are filtered - by testing
  // for this condition (see vgfItemModel::sort) and forcing a sort on data
  // changes if needed
  //
  // See also https://bugreports.qt-project.org/browse/QTBUG-35180
  connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
          this, SLOT(updateSort()), Qt::QueuedConnection);
  connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SLOT(updateSort()), Qt::QueuedConnection);
  connect(this, SIGNAL(columnsInserted(QModelIndex, int, int)),
          this, SLOT(updateSort()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
vgfItemModel::~vgfItemModel()
{
}

//-----------------------------------------------------------------------------
int vgfItemModel::roleForColumn(int column) const
{
  QTE_D_CONST(vgfItemModel);
  return d->ColumnDataMap.value(column, -1);
}

//-----------------------------------------------------------------------------
void vgfItemModel::setRoleForColumn(int column, int role)
{
  QTE_D(vgfItemModel);

  if (role < 0)
    {
    d->ColumnDataMap.remove(column);
    return;
    }

  d->ColumnDataMap.insert(column, role);
}

//-----------------------------------------------------------------------------
bool vgfItemModel::areHiddenItemsShown() const
{
  QTE_D_CONST(vgfItemModel);
  return d->ShowHidden;
}

//-----------------------------------------------------------------------------
void vgfItemModel::setHiddenItemsShown(bool state)
{
  QTE_D(vgfItemModel);
  if (d->ShowHidden != state)
    {
    d->ShowHidden = state;
    this->invalidateFilter();
    }
}

//-----------------------------------------------------------------------------
QVariant vgfItemModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid() && this->sourceModel())
    {
    switch (role)
      {
      case Qt::DisplayRole:
      case Qt::DecorationRole:
      case Qt::TextAlignmentRole:
      case Qt::ToolTipRole:
        {
        QTE_D_CONST(vgfItemModel);
        const int c = index.column();
        if (d->ColumnDataMap.contains(c))
          {
          return this->data(this->mapToSource(index), role,
                            d->ColumnDataMap[c]);
          }
        break;
        }

      case Qt::ForegroundRole:
        if (!this->sourceModel()->data(this->mapToSource(index),
                                       vgf::VisibilityRole).toBool())
          {
          // Gray out hidden items
          return QPalette().brush(QPalette::Disabled, QPalette::WindowText);
          }
        break;

      case vgf::InternalReferenceRole:
        {
        const QModelIndex& si = this->mapToSource(index);
        const QAbstractItemModel* sm = this->sourceModel();

        const vgf::ItemType type =
          sm->data(si, vgf::ItemTypeRole).value<vgf::ItemType>();
        const qint64 iid = sm->data(si, vgf::IdentityRole).toLongLong();

        return QVariant::fromValue(vgfItemReference(type, iid));
        }

      default:
        if (role >= vgf::ItemTypeRole && role < vgf::UserRole)
          {
          // Requests for logical data roles are passed through
          return this->sourceModel()->data(this->mapToSource(index), role);
          }
        break;
      }
    }

  return QSortFilterProxyModel::data(index, role);
}

//-----------------------------------------------------------------------------
QVariant vgfItemModel::data(
  const QModelIndex& sourceIndex, int presentationRole, int dataRole) const
{
  CHECK_ARG(sourceIndex.isValid(), QVariant());
  CHECK_ARG(this->sourceModel(), QVariant());

  const QAbstractItemModel* const sm = this->sourceModel();

  switch (presentationRole)
    {
    case Qt::ToolTipRole:
      switch (dataRole)
        {
        case vgf::StartTimeRole:
        case vgf::EndTimeRole:
          {
          const vgTimeStamp ts =
            sm->data(sourceIndex, dataRole).value<vgTimeStamp>();
          const vgUnixTime t(qRound64(ts.Time));
          return QString("%1 %2").arg(t.dateString(), t.timeString());
          }

        case vgf::ClassificationRole:
        case vgf::ConfidenceRole:
          break; // TODO

        default:
          break;
        }

      // If tool tip was not handled by above, fall through and use display
      // text as tool tip
    case Qt::DisplayRole:
      switch (dataRole)
        {
        case vgf::ItemTypeRole:
          break; // TODO

        case vgf::NameRole:
          return sm->data(sourceIndex, vgf::NameRole);

        case vgf::UniqueIdentityRole:
          {
          const QUuid uuid = sm->data(sourceIndex, dataRole).value<QUuid>();
          return (uuid.isNull() ? QString("(null)") : uuid.toString());
          }

        case vgf::StartTimeRole:
        case vgf::EndTimeRole:
          {
          const vgTimeStamp ts =
            sm->data(sourceIndex, dataRole).value<vgTimeStamp>();
          return vgUnixTime(qRound64(ts.Time)).timeString();
          }

        case vgf::RatingRole:
          break; // TODO

        case vgf::NoteRole:
          return sm->data(sourceIndex, dataRole).toString();

        case vgf::ClassificationRole:
          break; // TODO

        case vgf::ConfidenceRole:
          break; // TODO

        default:
          break;
        }
      break;

    case Qt::DecorationRole:
      switch (dataRole)
        {
        case vgf::ItemTypeRole:
          {
          QTE_D_CONST(vgfItemModel);
          return d->itemTypeIcon(sm->data(sourceIndex, dataRole).toInt());
          }

        case vgf::ClassificationRole:
          break; // TODO

        case vgf::StarRole:
          {
          QTE_D_CONST(vgfItemModel);
          return d->starIcon(sm->data(sourceIndex, dataRole).toBool());
          }

        default:
          break;
        }
      break;

    case Qt::TextAlignmentRole:
      if (dataRole == vgf::ConfidenceRole)
        {
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
      break;

    default:
      break;
    }

  return QVariant();
}

//-----------------------------------------------------------------------------
QVariant vgfItemModel::headerData(
  int section, Qt::Orientation orientation, int role) const
{
  QTE_D_CONST(vgfItemModel);

  if (orientation == Qt::Horizontal && d->ColumnDataMap.contains(section))
    {
    switch (role)
      {
      case Qt::DisplayRole:
        switch (d->ColumnDataMap[section])
          {
          case vgf::ItemTypeRole:           return "Type";
          case vgf::NameRole:               return "Name";
          case vgf::IdentityRole:           return "ID";
          case vgf::LogicalIdentityRole:    return "ID";
          case vgf::UniqueIdentityRole:     return "UUID";
          case vgf::StartTimeRole:          return "Start Time";
          case vgf::EndTimeRole:            return "End Time";
          case vgf::RatingRole:             return "Rating";
          case vgf::StarRole:               return QVariant(); // icon only
          case vgf::NoteRole:               return "Note";
          case vgf::ClassificationRole:     return "Classification";
          case vgf::ConfidenceRole:         return "Confidence";
          default: return QVariant();
          }
        break; // Should never get here; all paths above return a value

      case Qt::DecorationRole:
        switch (d->ColumnDataMap[section])
          {
          case vgf::StarRole:   return d->starIcon(true);
          default: return QVariant();
          }
        break; // Should never get here; all paths above return a value

      default:
        break;
      }
    }

  return QSortFilterProxyModel::headerData(section, orientation, role);
}

//-----------------------------------------------------------------------------
bool vgfItemModel::lessThan(
  const QModelIndex& left, const QModelIndex& right) const
{
  QTE_D_CONST(vgfItemModel);

  const int c = left.column();
  Q_ASSERT(c == right.column());

  if (d->ColumnDataMap.contains(c))
    {
    const int dataRole = d->ColumnDataMap[c];
    return this->lessThan(left, right, dataRole);
    }

  return QSortFilterProxyModel::lessThan(left, right);
}

//-----------------------------------------------------------------------------
bool vgfItemModel::lessThan(
  const QModelIndex& left, const QModelIndex& right, int role) const
{
  CHECK_ARG(this->sourceModel(), false);

  Q_ASSERT(left.column() == right.column());

  const QAbstractItemModel* const sm = this->sourceModel();
  const QVariant leftData = sm->data(left, role);
  const QVariant rightData = sm->data(right, role);

  switch (role)
    {
    // String comparisons
    case vgf::NameRole:
    case vgf::UniqueIdentityRole:
    case vgf::NoteRole:
      return QString::localeAwareCompare(leftData.toString(),
                                         rightData.toString());

    // Boolean comparisons
    case vgf::VisibilityRole:
    case vgf::UserVisibilityRole:
    case vgf::StarRole:
      return leftData.toBool() < rightData.toBool();

    // Integer comparisons
    case vgf::ItemTypeRole:
    case vgf::RatingRole:
    case vgf::ClassificationRole:
      return leftData.toInt() < rightData.toInt();

    // Floating-point comparisons
    case vgf::ConfidenceRole:
      return leftData.toDouble() < rightData.toDouble();

    // Timestamp comparisons
    case vgf::StartTimeRole:
    case vgf::EndTimeRole:
      return leftData.value<vgTimeStamp>() < rightData.value<vgTimeStamp>();

    default:
      return false;
    }
}

//-----------------------------------------------------------------------------
bool vgfItemModel::filterAcceptsRow(
  int sourceRow, const QModelIndex& sourceParent) const
{
  QTE_D_CONST(vgfItemModel);

  const QAbstractItemModel* const srcModel = this->sourceModel();
  if (!d->ShowHidden && srcModel)
    {
    const QModelIndex srcIndex = srcModel->index(sourceRow, 0, sourceParent);

    if (!srcModel->data(srcIndex, vgf::VisibilityRole).toBool())
      {
      return false;
      }
    }

  return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

//-----------------------------------------------------------------------------
bool vgfItemModel::filterAcceptsColumn(
  int sourceColumn, const QModelIndex& sourceParent) const
{
  const QModelIndex parent = this->mapFromSource(sourceParent);
  if (this->columnCount(this->mapFromSource(sourceParent)) < sourceColumn)
    {
    // Underlying model is expected to only "really" have one column, since
    // data roles are used to get data that is mapped to display columns by the
    // proxy model, but to claim to have a large number of columns because
    // QSortFilterProxyModel will only show a maximum of the source model's
    // columnCount() columns. Therefore, by default we ignore columns whose
    // index is greater than our column count, which should be equal to the
    // number of columns that we actually map.
    return false;
    }
  return QSortFilterProxyModel::filterAcceptsColumn(sourceColumn, sourceParent);
}

//-----------------------------------------------------------------------------
void vgfItemModel::sort(int column, Qt::SortOrder order)
{
  QTE_D(vgfItemModel);

  QSortFilterProxyModel::sort(column, order);

  // Determine, if we are sorting (column >= 0), if QSortFilterProxyModel
  // figured out what column to sort; if not, we may need to force a sort later
  d->SortNeeded =
    column >= 0 &&
    !this->mapToSource(this->index(0, column, QModelIndex())).isValid();
}

//-----------------------------------------------------------------------------
void vgfItemModel::updateSort()
{
  QTE_D(vgfItemModel);

  // Our data has changed; if we previously failed to sort...
  if (d->SortNeeded)
    {
    // ...then attempt to do so now (if dynamic sorting is enabled)
    if (this->dynamicSortFilter())
      {
      // NOTE: Must clear dynamic sort filter first or sort() will decline to
      //       actually do anything; this ends up sorting twice, but there
      //       doesn't seem to be any way around that
      this->setDynamicSortFilter(false);
      this->sort(this->sortColumn(), this->sortOrder());
      this->setDynamicSortFilter(true);
      }
    }
}
