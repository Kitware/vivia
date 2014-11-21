/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvDescriptorInfoTree.h"

#include <QRegExp>

#include <qtScopedValueChange.h>
#include <qtStlUtil.h>

#include <vgUnixTime.h>

Q_DECLARE_METATYPE(QList<qint64>)

QTE_IMPLEMENT_D_FUNC(vvDescriptorInfoTree)

//BEGIN vvDescriptorInfoTreePrivate

//-----------------------------------------------------------------------------
class vvDescriptorInfoTreePrivate
{
public:
  vvDescriptorInfoTreePrivate(vvDescriptorInfoTree* q);

  static void reapChildren(QTreeWidgetItem* item,
                           QTreeWidgetItem* parent = 0);

  static QList<qint64> childDescriptors(const QTreeWidgetItem* item);
  static void setChildDescriptors(QTreeWidgetItem* item,
                                  const QVariant& newData);

  void setHeaderLabels();

  void repopulateDescriptors(QTreeWidgetItem* root = 0);
  void addDescriptorItems(QList<qint64>, QTreeWidgetItem* parent);

  QScopedPointer<vvDescriptorInfoTreeItemFactory> itemFactory;

  vvDescriptorInfoTree::Columns columns;
  QHash<qint64, vvDescriptor> descriptors;

  bool groupByStyle;
  vvDescriptorStyle::Map styleMap;

  bool hideGroupedItems;

protected:
  QTE_DECLARE_PUBLIC_PTR(vvDescriptorInfoTree)

  void addGroupedDescriptorItems(QList<qint64>, QTreeWidgetItem* parent);

  static bool isValidStyle(int);
  static void reapChildren(QTreeWidgetItem* item,
                           QTreeWidgetItem* parent,
                           QList<QTreeWidgetItem*>& reaped);

private:
  QTE_DECLARE_PUBLIC(vvDescriptorInfoTree)
};

//-----------------------------------------------------------------------------
vvDescriptorInfoTreePrivate::vvDescriptorInfoTreePrivate(
  vvDescriptorInfoTree* q) :
  itemFactory(new vvDescriptorInfoTreeItemFactory(q)),
  columns(vvDescriptorInfoTree::Name | vvDescriptorInfoTree::Source),
  groupByStyle(false),
  hideGroupedItems(false),
  q_ptr(q)
{
}

//-----------------------------------------------------------------------------
bool vvDescriptorInfoTreePrivate::isValidStyle(int s)
{
  // Test that s is non-zero and is a: exactly a power of two, or b: -1
  uint u = static_cast<uint>(s);
  return (u && !(u & (u - 1))) || (s == -1);
}

//-----------------------------------------------------------------------------
QList<qint64> vvDescriptorInfoTreePrivate::childDescriptors(
  const QTreeWidgetItem* item)
{
  QVariant data = item->data(0, vvDescriptorInfoTree::ChildDescriptorsRole);
  if (data.isValid() && data.canConvert<QList<qint64> >())
    {
    return data.value<QList<qint64> >();
    }
  return QList<qint64>();
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTreePrivate::setChildDescriptors(
  QTreeWidgetItem* item, const QVariant& newData)
{
  item->setData(0, vvDescriptorInfoTree::ChildDescriptorsRole, newData);
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTreePrivate::setHeaderLabels()
{
  QTE_Q(vvDescriptorInfoTree);

  // Clear any existing headers
  q->setHeaderItem(new QTreeWidgetItem);

  // Determine new labels
  QStringList headerLabels;
  if (this->columns.testFlag(vvDescriptorInfoTree::Name))
    {
    headerLabels << "Name (Type)";
    }
  if (this->columns.testFlag(vvDescriptorInfoTree::Source))
    {
    headerLabels << "Source";
    }
  if (this->columns.testFlag(vvDescriptorInfoTree::TimeRange))
    {
    headerLabels << "Start Time" << "End Time";
    }

  // Set new labels
  q->setHeaderLabels(headerLabels);
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTreePrivate::reapChildren(
  QTreeWidgetItem* item, QTreeWidgetItem* parent)
{
  QList<QTreeWidgetItem*> reaped;
  vvDescriptorInfoTreePrivate::reapChildren(item, parent, reaped);
  qDeleteAll(reaped);
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTreePrivate::reapChildren(
  QTreeWidgetItem* item, QTreeWidgetItem* parent,
  QList<QTreeWidgetItem*>& reaped)
{
  if (parent)
    {
    const int type = vvDescriptorInfoTree::itemType(item);
    if (type == vvDescriptorInfoTree::DescriptorItem ||
        type == vvDescriptorInfoTree::DescriptorStyleGroup)
      {
      reaped.append(item);
      return;
      }
    }

  foreach_child (QTreeWidgetItem* child, item)
    {
    vvDescriptorInfoTreePrivate::reapChildren(child, item, reaped);
    }

  vvDescriptorInfoTreePrivate::setChildDescriptors(item, QVariant());
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTreePrivate::repopulateDescriptors(QTreeWidgetItem* root)
{
  if (!root)
    {
    QTE_Q(vvDescriptorInfoTree);
    root = q->invisibleRootItem();
    }

  QList<qint64> descriptors =
    vvDescriptorInfoTreePrivate::childDescriptors(root);

  if (!descriptors.isEmpty())
    {
    // Clear and re-add descriptors
    vvDescriptorInfoTreePrivate::reapChildren(root);
    this->addDescriptorItems(descriptors, root);
    vvDescriptorInfoTreePrivate::setChildDescriptors(
      root, QVariant::fromValue(descriptors));
    return;
    }

  // Iterate over children
  foreach_child (QTreeWidgetItem* child, root)
    {
    this->repopulateDescriptors(child);
    }
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTreePrivate::addDescriptorItems(
  QList<qint64> newDescriptors, QTreeWidgetItem* parent)
{
  if (this->groupByStyle)
    {
    // Find existing group parents
    QHash<int, QTreeWidgetItem*> groupParents;
    foreach_child (QTreeWidgetItem* child, parent)
      {
      const int type = vvDescriptorInfoTree::itemType(child);
      if (type == vvDescriptorInfoTree::DescriptorStyleGroup)
        {
        int style = child->data(0, vvDescriptorInfoTree::StyleRole).toInt();
        if (vvDescriptorInfoTreePrivate::isValidStyle(style))
          {
          groupParents.insert(style, child);
          }
        }
      }

    // Sort descriptors by style
    QHash<int, QList<qint64> > groupedDescriptors;
    foreach (qint64 id, newDescriptors)
      {
      if (!this->descriptors.contains(id))
        {
        continue;
        }

      bool known = false;
      int styles = vvDescriptorStyle::styles(this->descriptors[id],
                                             this->styleMap, &known);
      if (known)
        {
        for (int g = 1; styles; g <<= 1)
          {
          if (styles & g)
            {
            groupedDescriptors[g].append(id);
            styles ^= g;
            }
          }
        }
      else
        {
        groupedDescriptors[-1].append(id);
        }
      }

    // Add descriptors to group parents
    foreach (int s, groupedDescriptors.keys())
      {
      // Get group parent, creating it if it does not exist
      QTreeWidgetItem* group = groupParents.value(s, 0);
      if (!group)
        {
        group = this->itemFactory->createGroupItem(s);

        vvDescriptorInfoTree::setItemType(
          group, vvDescriptorInfoTree::DescriptorStyleGroup);
        group->setData(0, vvDescriptorInfoTree::StyleRole, s);

        parent->addChild(group);
        }

      // Add descriptors to group
      this->addGroupedDescriptorItems(groupedDescriptors[s], group);

      // Update group count
      QRegExp ocre("\\s*\\(\\d+\\)$");
      group->setText(0, group->text(0).replace(ocre, "") +
                        QString(" (%1)").arg(group->childCount()));
      }
    }
  else
    {
    addGroupedDescriptorItems(newDescriptors, parent);
    }
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTreePrivate::addGroupedDescriptorItems(
  QList<qint64> newDescriptors, QTreeWidgetItem* parent)
{
  foreach (qint64 id, newDescriptors)
    {
    if (!this->descriptors.contains(id))
      {
      continue;
      }

    // Create item
    QTreeWidgetItem* item =
      this->itemFactory->createItem(id, this->descriptors[id]);

    // Set item data
    vvDescriptorInfoTree::setItemId(item, id);
    vvDescriptorInfoTree::setItemType(
      item, vvDescriptorInfoTree::DescriptorItem);

    // Add new item to parent
    parent->addChild(item);

    // Hide grouped items?
    if (this->hideGroupedItems)
      {
      item->setHidden(true);
      }
    }
}

//END vvDescriptorInfoTreePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvDescriptorInfoTreeItemFactory

//-----------------------------------------------------------------------------
vvDescriptorInfoTreeItemFactory::vvDescriptorInfoTreeItemFactory(
  vvDescriptorInfoTree* tree) : q_ptr(tree)
{
}

//-----------------------------------------------------------------------------
vvDescriptorInfoTreeItemFactory::~vvDescriptorInfoTreeItemFactory()
{
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vvDescriptorInfoTreeItemFactory::createItem(
  qint64, const vvDescriptor& descriptor)
{
  QTE_Q(vvDescriptorInfoTree);
  vvDescriptorInfoTreePrivate* d = q->d_func();

  QTreeWidgetItem* item = new QTreeWidgetItem;

  int ci = 0;
  if (d->columns.testFlag(vvDescriptorInfoTree::Name))
    {
    item->setText(ci++, qtString(descriptor.DescriptorName));
    }
  if (d->columns.testFlag(vvDescriptorInfoTree::Source))
    {
    item->setText(ci++, qtString(descriptor.ModuleName));
    }

  if (d->columns.testFlag(vvDescriptorInfoTree::TimeRange))
    {
    vgUnixTime ts(descriptor.Region.begin()->TimeStamp.Time);
    vgUnixTime te(descriptor.Region.rbegin()->TimeStamp.Time);
    item->setText(ci++, ts.timeString());
    item->setText(ci++, te.timeString());
    }

  return item;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vvDescriptorInfoTreeItemFactory::createGroupItem(int style)
{
  QTreeWidgetItem* item = new QTreeWidgetItem;

  if (style != -1)
    {
    vvDescriptorStyle::StyleFlag sf =
      static_cast<vvDescriptorStyle::StyleFlag>(style);
    item->setText(0, vvDescriptorStyle::string(sf));
    }
  else
    {
    item->setText(0, "(unrecognized)");
    }

  return item;
}

//END vvDescriptorInfoTreeItemFactory

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvDescriptorInfoTree static helpers

//-----------------------------------------------------------------------------
qint64 vvDescriptorInfoTree::itemId(
  const QTreeWidgetItem* item, bool* isValid)
{
  QVariant data = item->data(0, vvDescriptorInfoTree::IdRole);
  if (data.isNull() || data.type() != QVariant::LongLong)
    {
    if (isValid)
      {
      *isValid = false;
      }
    return -1;
    }
  return data.toLongLong();
}

//-----------------------------------------------------------------------------
int vvDescriptorInfoTree::itemType(const QTreeWidgetItem* item)
{
  return item->data(0, vvDescriptorInfoTree::TypeRole).toInt();
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setItemId(QTreeWidgetItem* item, qint64 newId)
{
  item->setData(0, IdRole, newId);
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setItemType(QTreeWidgetItem* item, int newType)
{
  item->setData(0, TypeRole, newType);
}

//END vvDescriptorInfoTree static helpers

///////////////////////////////////////////////////////////////////////////////

//BEGIN vvDescriptorInfoTree

//-----------------------------------------------------------------------------
vvDescriptorInfoTree::vvDescriptorInfoTree(QWidget* parent)
  : QTreeWidget(parent), d_ptr(new vvDescriptorInfoTreePrivate(this))
{
  QTE_D(vvDescriptorInfoTree);
  d->setHeaderLabels();
  d->styleMap = vvDescriptorStyle::map();

  connect(this, SIGNAL(itemSelectionChanged()),
          this, SLOT(emitSelectionChanges()));
}

//-----------------------------------------------------------------------------
vvDescriptorInfoTree::~vvDescriptorInfoTree()
{
}

//-----------------------------------------------------------------------------
QHash<qint64, vvDescriptor> vvDescriptorInfoTree::descriptors() const
{
  QTE_D_CONST(vvDescriptorInfoTree);
  return d->descriptors;
}

//-----------------------------------------------------------------------------
QList<vvDescriptor> vvDescriptorInfoTree::descriptors(
  QList<qint64> ids) const
{
  QTE_D_CONST(vvDescriptorInfoTree);

  QList<vvDescriptor> result;
  foreach (qint64 id, ids)
    {
    if (d->descriptors.contains(id))
      {
      result.append(d->descriptors[id]);
      }
    }

  return result;
}

//-----------------------------------------------------------------------------
QList<vvDescriptor> vvDescriptorInfoTree::descriptors(
  QList<QTreeWidgetItem*> items, bool includeChildren) const
{
  return this->descriptors(this->descriptorIds(items, includeChildren));
}

//-----------------------------------------------------------------------------
QList<qint64> vvDescriptorInfoTree::descriptorIds(
  QList<QTreeWidgetItem*> items, bool includeChildren) const
{
  QSet<qint64> results;
  if (includeChildren)
    {
    QSet<QTreeWidgetItem*> visitedItems;

    // Iterate over all items requested
    while (!items.isEmpty())
      {
      QTreeWidgetItem* item = items.takeFirst();

      // Skip items that have been visited already
      if (visitedItems.contains(item))
        {
        continue;
        }

      // Add result for this item, if it is a descriptor
      if (itemType(item) == DescriptorItem)
        {
        results.insert(itemId(item));
        }
      visitedItems.insert(item);

      // Add children to list for later consideration
      foreach_child (QTreeWidgetItem* child, item)
        {
        items.append(child);
        }
      }
    }
  else
    {
    // Iterate (non-recursively) over all items requested
    foreach (QTreeWidgetItem* item, items)
      {
      if (itemType(item) == DescriptorItem)
        {
        results.insert(itemId(item));
        }
      }
    }
  return results.toList();
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* vvDescriptorInfoTree::findItem(
  int type, qint64 id, QTreeWidgetItem* from)
{
  QTreeWidgetItemIterator iter(this);
  if (from)
    {
    QTreeWidgetItemIterator fiter(from);
    iter = ++fiter;
    }

  while (*iter)
    {
    if (itemType(*iter) == type)
      {
      if (id == -1 || itemId(*iter) == id)
        {
        return *iter;
        }
      }
    ++iter;
    }

  return 0;
}

//-----------------------------------------------------------------------------
QList<QTreeWidgetItem*> vvDescriptorInfoTree::findItems(int type, qint64 id)
{
  QList<QTreeWidgetItem*> items;

  QTreeWidgetItemIterator iter(this);
  while (*iter)
    {
    if (itemType(*iter) == type)
      {
      if (id == -1 || itemId(*iter) == id)
        {
        items.append(*iter);
        }
      }
    ++iter;
    }

  return items;
}

//-----------------------------------------------------------------------------
QList<QTreeWidgetItem*> vvDescriptorInfoTree::descriptorItems(qint64 id)
{
  return this->findItems(DescriptorItem, id);
}

//-----------------------------------------------------------------------------
bool vvDescriptorInfoTree::groupByStyle() const
{
  QTE_D_CONST(vvDescriptorInfoTree);
  return d->groupByStyle;
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setGroupByStyle(bool newState)
{
  QTE_D(vvDescriptorInfoTree);

  if (d->groupByStyle != newState)
    {
    d->groupByStyle = newState;
    d->repopulateDescriptors();
    }
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setHideGroupedItems(bool newState)
{
  QTE_D(vvDescriptorInfoTree);

  if (d->hideGroupedItems != newState)
    {
    d->hideGroupedItems = newState;
    d->repopulateDescriptors();
    }
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setColumns(
  vvDescriptorInfoTree::Columns newColumns, bool setHeaders)
{
  QTE_D(vvDescriptorInfoTree);

  if (d->columns != newColumns)
    {
    d->columns = newColumns;
    setHeaders && (d->setHeaderLabels(), false);
    d->repopulateDescriptors();
    }
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setStyleMap(vvDescriptorStyle::Map newMap)
{
  QTE_D(vvDescriptorInfoTree);

  if (d->styleMap != newMap)
    {
    d->styleMap = newMap;
    d->repopulateDescriptors();
    }
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setItemFactory(
  vvDescriptorInfoTreeItemFactory* newFactory)
{
  QTE_D(vvDescriptorInfoTree);
  d->itemFactory.reset(newFactory);
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setDescriptors(
  QHash<qint64, vvDescriptor> newDescriptors)
{
  QTE_D(vvDescriptorInfoTree);

  // Remove items associated with any previous descriptor set
  vvDescriptorInfoTreePrivate::reapChildren(this->invisibleRootItem());

  // Set new descriptor set
  d->descriptors = newDescriptors;
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::addDescriptorItems(
  QList<qint64> descriptors, QTreeWidgetItem* parent)
{
  QTE_D(vvDescriptorInfoTree);

  parent || (parent = this->invisibleRootItem());

  // Add descriptors
  qtDelayTreeSorting ds(this);
  d->addDescriptorItems(descriptors, parent);

  // Update parent's list of descriptors
  QList<qint64> combinedDescriptors =
    vvDescriptorInfoTreePrivate::childDescriptors(parent);
  combinedDescriptors.append(descriptors);
  vvDescriptorInfoTreePrivate::setChildDescriptors(
    parent, QVariant::fromValue(combinedDescriptors));
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::setDescriptorItems(
  QList<qint64> newDescriptors, QTreeWidgetItem* parent)
{
  QTE_D(vvDescriptorInfoTree);

  parent || (parent = this->invisibleRootItem());

  // Get parent's list of descriptors
  QList<qint64> oldDescriptors =
    vvDescriptorInfoTreePrivate::childDescriptors(parent);

  // Do nothing if the list would not change
  if (oldDescriptors == newDescriptors)
    {
    return;
    }

  // Get expanded group parents, if relevant
  QSet<int> expandedStyles;
  if (d->groupByStyle)
    {
    foreach_child (QTreeWidgetItem* group, parent)
      {
      if (group->isExpanded())
        {
        int type = vvDescriptorInfoTree::itemType(group);
        if (type == DescriptorStyleGroup)
          {
          int style = group->data(0, vvDescriptorInfoTree::StyleRole).toInt();
          expandedStyles.insert(style);
          }
        }
      }
    }

  // Clear existing descriptors
  vvDescriptorInfoTreePrivate::reapChildren(parent);

  // Add descriptors in new set
  this->addDescriptorItems(newDescriptors, parent);

  // Re-expand group parents
  if (d->groupByStyle)
    {
    foreach_child (QTreeWidgetItem* group, parent)
      {
      int type = vvDescriptorInfoTree::itemType(group);
      if (type == DescriptorStyleGroup)
        {
        int style = group->data(0, vvDescriptorInfoTree::StyleRole).toInt();
        group->setExpanded(expandedStyles.contains(style));
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::clear()
{
  QTreeWidget::clear();
  vvDescriptorInfoTreePrivate::setChildDescriptors(
    this->invisibleRootItem(), QVariant());
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoTree::emitSelectionChanges()
{
  QList<qint64> selectedIds = this->descriptorIds(this->selectedItems());
  emit this->descriptorSelectionChanged(selectedIds);
  emit this->descriptorSelectionChanged(this->descriptors(selectedIds));
}

//END vvDescriptorInfoTree
