// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsRegionList.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QSignalMapper>

#include "vsRegionTypeDelegate.h"

namespace // anonymous
{

enum DataRole
{
  IdRole = Qt::UserRole,
  NameRole,
  VisibilityRole
};

//-----------------------------------------------------------------------------
int itemId(QTreeWidgetItem* item)
{
  return item->data(0, IdRole).toInt();
}

//-----------------------------------------------------------------------------
void setItemId(QTreeWidgetItem* item, int newId)
{
  item->setData(0, IdRole, QVariant(newId));
}

//-----------------------------------------------------------------------------
QString itemName(QTreeWidgetItem* item)
{
  return item->data(0, NameRole).toString();
}

//-----------------------------------------------------------------------------
void setItemName(QTreeWidgetItem* item, const QString& newName)
{
  item->setData(0, NameRole, QVariant(newName));
  item->setText(0, newName);
}

//-----------------------------------------------------------------------------
bool itemVisibility(QTreeWidgetItem* item)
{
  return item->data(0, VisibilityRole).toBool();
}

//-----------------------------------------------------------------------------
void setItemVisibility(QTreeWidgetItem* item, bool newVisibility)
{
  item->setData(0, VisibilityRole, QVariant(newVisibility));
  item->setCheckState(0, newVisibility ? Qt::Checked : Qt::Unchecked);
}

//-----------------------------------------------------------------------------
vsContour::Type itemType(QTreeWidgetItem* item)
{
  return item->data(1, Qt::UserRole).value<vsContour::Type>();
}

//-----------------------------------------------------------------------------
void setItemType(QTreeWidgetItem* item, vsContour::Type newType)
{
  QVariant typeData = QVariant::fromValue<vsContour::Type>(newType);
  item->setData(1, Qt::UserRole, typeData);
  item->setText(1, vsContour::typeString(newType));
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vsRegionListPrivate
{
public:
  vsRegionListPrivate() : ConvertEventMenu(0), ShowHidden(true) {}

  QMenu* ContextMenu;
  QMenu* ConvertEventMenu;
  QSignalMapper* ConvertEventMapper;

  bool ShowHidden;
};

QTE_IMPLEMENT_D_FUNC(vsRegionList)

//-----------------------------------------------------------------------------
vsRegionList::vsRegionList(QWidget* parent)
  : QTreeWidget(parent), d_ptr(new vsRegionListPrivate)
{
  QTE_D(vsRegionList);

  vsRegionTypeDelegate* delegate = new vsRegionTypeDelegate(this);
  this->setItemDelegateForColumn(1, delegate);

  connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(updateRegionFromItem(QTreeWidgetItem*, int)));
  connect(this, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateSelectionStatus()));

  d->ContextMenu = new QMenu(this);
  d->ConvertEventMapper = new QSignalMapper(this);
  connect(d->ConvertEventMapper, SIGNAL(mapped(int)),
          this, SLOT(convertSelectedToEvents(int)));
}

//-----------------------------------------------------------------------------
vsRegionList::~vsRegionList()
{
}

//-----------------------------------------------------------------------------
void vsRegionList::addRegion(vsContour contour)
{
  const int id = contour.id();

  QTreeWidgetItem* item = new QTreeWidgetItem;
  setItemId(item, id);
  setItemName(item, contour.name());
  setItemType(item, contour.type());
  setItemVisibility(item, true);
  item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
  this->addTopLevelItem(item);
}

//-----------------------------------------------------------------------------
void vsRegionList::setRegionName(int id, QString newName)
{
  foreach_child (QTreeWidgetItem* item, this->invisibleRootItem())
    {
    if (itemId(item) == id)
      {
      setItemName(item, newName);
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vsRegionList::setRegionType(int id, vsContour::Type newType)
{
  foreach_child (QTreeWidgetItem* item, this->invisibleRootItem())
    {
    if (itemId(item) == id)
      {
      setItemType(item, newType);
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vsRegionList::removeRegion(int id)
{
  foreach_child (QTreeWidgetItem* item, this->invisibleRootItem())
    {
    if (itemId(item) == id)
      {
      delete item;
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vsRegionList::removeSelected()
{
  int k = 0;
  foreach (QTreeWidgetItem* item, this->selectedItems())
    {
    emit this->regionRemoved(itemId(item));
    ++k;
    }
  this->emitRemovalStatus(k);
  // Item(s) will be removed on signal from vsCore
}

//-----------------------------------------------------------------------------
void vsRegionList::removeAll()
{
  int k = 0;
  foreach (QTreeWidgetItem* item, this->invisibleRootItem()->takeChildren())
    {
    emit this->regionRemoved(itemId(item));
    ++k;
    }
  this->emitRemovalStatus(k);
  // Item(s) will be removed on signal from vsCore
}

//-----------------------------------------------------------------------------
void vsRegionList::emitRemovalStatus(int numRemoved)
{
  return; // \TODO

  QString message;
  if (numRemoved)
    {
    const char* n = (numRemoved > 1 ? "regions" : "region");
    message = QString("Removed %2 %1").arg(n).arg(numRemoved);
    }
  else
    {
    message = "No regions were removed";
    }
  // \TODO emit status message
}

//-----------------------------------------------------------------------------
void vsRegionList::setShowHidden(bool newFlag)
{
  QTE_D(vsRegionList);

  if (newFlag != d->ShowHidden)
    {
    d->ShowHidden = newFlag;
    for (int i = 0, count = this->topLevelItemCount(); i < count; ++i)
      {
      QTreeWidgetItem* item = this->topLevelItem(i);
      bool show = itemVisibility(item) || d->ShowHidden;
      item->setHidden(!show);
      }
    }
}

//-----------------------------------------------------------------------------
void vsRegionList::updateRegionFromItem(QTreeWidgetItem* item, int column)
{
  QTE_D(vsRegionList);

  const int id = itemId(item);

  switch (column)
    {
    case 0:
      {
      QString newName = item->text(0);
      bool newVisibility = (item->checkState(0) == Qt::Checked);

      // Check if visibility changed
      if (newVisibility != itemVisibility(item))
        {
        setItemVisibility(item, newVisibility);
        item->setHidden(!(newVisibility || d->ShowHidden));
        emit this->regionVisibilityChanged(id, newVisibility);
        }

      // Check if name changed
      if (newName != itemName(item))
        emit this->regionNameChanged(id, newName);
      } break;

    case 1:
      emit this->regionTypeChanged(id, itemType(item));
      break;

    default:
      break;
    }
}

//-----------------------------------------------------------------------------
void vsRegionList::updateSelectionStatus()
{
  emit this->selectionStatusChanged(!this->selectedItems().isEmpty());
}

//-----------------------------------------------------------------------------
void vsRegionList::addContextMenuAction(QAction* action)
{
  QTE_D(vsRegionList);
  d->ContextMenu->addAction(action);
}

//-----------------------------------------------------------------------------
void vsRegionList::contextMenuEvent(QContextMenuEvent* event)
{
  if (!this->selectedItems().isEmpty())
    {
    QTE_D(vsRegionList);
    d->ContextMenu->exec(event->globalPos());
    return;
    }

  QWidget::contextMenuEvent(event);
}

//-----------------------------------------------------------------------------
void vsRegionList::setEventTypes(QList<vsEventInfo> types)
{
  QTE_D(vsRegionList);

  if (!d->ConvertEventMenu)
    {
    d->ConvertEventMenu = new QMenu("Convert to &Event", this);
    d->ContextMenu->addMenu(d->ConvertEventMenu);
    }

  qDeleteAll(d->ConvertEventMenu->actions());
  foreach (const vsEventInfo& ei, types)
    {
    QAction* action =
      d->ConvertEventMenu->addAction(ei.name,
                                     d->ConvertEventMapper, SLOT(map()));
    d->ConvertEventMapper->setMapping(action, ei.type);
    }
}

//-----------------------------------------------------------------------------
void vsRegionList::convertSelectedToEvents(int type)
{
  foreach (QTreeWidgetItem* item, this->selectedItems())
    {
    // Actual conversion done by vsCore
    emit this->regionConvertedToEvent(itemId(item), type);
    }
}
