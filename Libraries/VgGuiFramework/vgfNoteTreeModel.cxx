// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgfNoteTreeModel.h"

#include "vgfMetaTypes.h"
#include "vgfNamespace.h"

#include <vgCheckArg.h>

//-----------------------------------------------------------------------------
class vgfNoteTreeModelPrivate
{
public:
  vgfNoteTreeModelPrivate() : ShowInactive(true) {}

  bool ShowInactive;
  vgTimeStamp CurrentTime;
};

QTE_IMPLEMENT_D_FUNC(vgfNoteTreeModel)

//-----------------------------------------------------------------------------
vgfNoteTreeModel::vgfNoteTreeModel(QObject* parent) :
  vgfItemModel(parent), d_ptr(new vgfNoteTreeModelPrivate)
{
  this->setRoleForColumn(EntityTypeColumn, vgf::ItemTypeRole);
  this->setRoleForColumn(EntityIconColumn, vgf::ClassificationRole);
  this->setRoleForColumn(StartTimeColumn, vgf::StartTimeRole);
  this->setRoleForColumn(EndTimeColumn, vgf::EndTimeRole);
  this->setRoleForColumn(NoteColumn, vgf::NoteRole);
}

//-----------------------------------------------------------------------------
vgfNoteTreeModel::~vgfNoteTreeModel()
{
}

//-----------------------------------------------------------------------------
bool vgfNoteTreeModel::areInactiveItemsShown() const
{
  QTE_D_CONST(vgfNoteTreeModel);
  return d->ShowInactive;
}

//-----------------------------------------------------------------------------
void vgfNoteTreeModel::setInactiveItemsShown(bool state)
{
  QTE_D(vgfNoteTreeModel);
  if (d->ShowInactive != state)
    {
    d->ShowInactive = state;
    this->invalidateFilter();
    }
}

//-----------------------------------------------------------------------------
int vgfNoteTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return ColumnCount;
}

//-----------------------------------------------------------------------------
QVariant vgfNoteTreeModel::headerData(
  int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
    switch (static_cast<vgfNoteTreeModel::Column>(section))
      {
      case EntityTypeColumn:
      case EntityIconColumn:
        // No header text for entity type/icon columns
        return QVariant();

      default:
        break;
      }
    }

  return vgfItemModel::headerData(section, orientation, role);
}

//-----------------------------------------------------------------------------
QVariant vgfNoteTreeModel::data(const QModelIndex& index, int role) const
{
  CHECK_ARG(index.isValid(), QVariant());

  switch (index.column())
    {
    case EntityTypeColumn:
      switch (role)
        {
        case Qt::DisplayRole:
          // Only display icon for entity type column
          return QVariant();

        case Qt::ToolTipRole:
          {
          const QModelIndex srcIndex = this->mapToSource(index);

          const QString itemType =
            this->data(srcIndex, role, vgf::ItemTypeRole).toString();
          const QString itemName =
            this->data(srcIndex, role, vgf::NameRole).toString();

          return QString("<h4>%1</h4>%2").arg(itemType, itemName);
          }

        default:
          break;
        }
      break;

    case EntityIconColumn:
      if (role == Qt::DisplayRole)
        {
        // Only display icon for entity icon column
        return QVariant();
        }
      break;

    default:
      break;
    }

  return vgfItemModel::data(index, role);
}

//-----------------------------------------------------------------------------
bool vgfNoteTreeModel::filterAcceptsRow(
  int sourceRow, const QModelIndex& sourceParent) const
{
  QTE_D_CONST(vgfNoteTreeModel);

  const QAbstractItemModel* const srcModel = this->sourceModel();
  if (srcModel)
    {
    const QModelIndex srcIndex = srcModel->index(sourceRow, 0, sourceParent);
    if (srcModel->data(srcIndex, vgf::NoteRole).toString().isEmpty())
      {
      // Only items with notes are shown in the note list
      return false;
      }

    if (!d->ShowInactive && srcModel)
      {
      const vgTimeStamp st =
        srcModel->data(srcIndex, vgf::StartTimeRole).value<vgTimeStamp>();
      const vgTimeStamp et =
        srcModel->data(srcIndex, vgf::EndTimeRole).value<vgTimeStamp>();

      if (!(st <= d->CurrentTime && d->CurrentTime <= et))
        {
        // Item temporal extent does not include current time; hide item
        return false;
        }
      }
    }

  return vgfItemModel::filterAcceptsRow(sourceRow, sourceParent);
}

//-----------------------------------------------------------------------------
void vgfNoteTreeModel::setCurrentTime(const vgTimeStamp& ts)
{
  QTE_D(vgfNoteTreeModel);

  if (d->CurrentTime != ts)
    {
    d->CurrentTime = ts;
    if (!d->ShowInactive)
      {
      this->invalidateFilter();
      }
    }
}
