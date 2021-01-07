// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgMixerWidget.h"

#include <QDebug>
#include <QHash>
#include <QList>
#include <QMultiHash>
#include <QStack>
#include <QtCore>

#include <qtDrawerWidget.h>
#include <qtScopedValueChange.h>

#include "vgMixerDrawer.h"
#include "vgMixerItem.h"

QTE_IMPLEMENT_D_FUNC(vgMixerWidget)

//BEGIN vgMixerWidgetPrivate

//-----------------------------------------------------------------------------
class vgMixerWidgetPrivate
{
public:
  QHash<int, vgMixerItem*> items_;
  QHash<int, vgMixerDrawer*> groups_;
  QMultiHash<vgMixerDrawer*, vgMixerItem*> groupItems_;
  QMultiHash<vgMixerDrawer*, vgMixerDrawer*> groupGroups_;
  QStack<vgMixerDrawer*> group_;
  bool enableUpdates_;

  void removeItem(int key);
  void removeGroup(vgMixerDrawer* group);
  void setGroupState(vgMixerDrawer* group, bool state);
  void setGroupInverted(vgMixerDrawer* group, bool state);
  void updateParentState(vgMixerItem*);
  void updateParentState(vgMixerDrawer*);
  Qt::CheckState groupState(vgMixerDrawer*);

  void setGroupValue(vgMixerDrawer* group, double value);
};

//-----------------------------------------------------------------------------
void vgMixerWidgetPrivate::removeItem(int key)
{
  vgMixerItem* item = this->items_.take(key);
  this->groupItems_.remove(item->parent(), item);
  this->updateParentState(item);
  delete item;
}

//-----------------------------------------------------------------------------
void vgMixerWidgetPrivate::removeGroup(vgMixerDrawer* group)
{
  foreach (vgMixerItem* item, this->groupItems_.values(group))
    {
    this->removeItem(item->key());
    }
  this->groupItems_.remove(group);

  foreach (vgMixerDrawer* drawer, this->groupGroups_.values(group))
    {
    this->removeGroup(drawer);
    }
  this->groupGroups_.remove(group);

  this->groups_.remove(group->id());
}

//-----------------------------------------------------------------------------
void vgMixerWidgetPrivate::setGroupState(vgMixerDrawer* group, bool state)
{
  foreach (vgMixerDrawer* subgroup, this->groupGroups_.values(group))
    {
    this->setGroupState(subgroup, state);
    }
  foreach (vgMixerItem* item, this->groupItems_.values(group))
    {
    item->setState(state);
    }
}

//-----------------------------------------------------------------------------
void vgMixerWidgetPrivate::setGroupInverted(vgMixerDrawer* group, bool state)
{
  foreach (vgMixerDrawer* subgroup, this->groupGroups_.values(group))
    {
    this->setGroupInverted(subgroup, state);
    }
  foreach (vgMixerItem* item, this->groupItems_.values(group))
    {
    item->setInverted(state);
    }
}

//-----------------------------------------------------------------------------
void vgMixerWidgetPrivate::updateParentState(vgMixerItem* i)
{
  vgMixerDrawer* parent = i->parent();
  parent->setCheckState(this->groupState(parent));
  this->updateParentState(parent);
}

//-----------------------------------------------------------------------------
void vgMixerWidgetPrivate::updateParentState(vgMixerDrawer* g)
{
  vgMixerDrawer* parent = g->parent();
  if (parent)
    {
    parent->setCheckState(this->groupState(parent));
    this->updateParentState(parent);
    }
}

//-----------------------------------------------------------------------------
Qt::CheckState
vgMixerWidgetPrivate::groupState(vgMixerDrawer* group)
{
  Qt::CheckState state = Qt::PartiallyChecked;

  foreach (vgMixerDrawer* subgroup, this->groupGroups_.values(group))
    {
    Qt::CheckState s = this->groupState(subgroup);
    if (s == Qt::PartiallyChecked)
      {
      return Qt::PartiallyChecked;
      }
    else if (state == Qt::PartiallyChecked)
      {
      state = s;
      }
    else if (state != s)
      {
      return Qt::PartiallyChecked;
      }
    }
  foreach (vgMixerItem* item, this->groupItems_.values(group))
    {
    Qt::CheckState s = (item->state() ? Qt::Checked : Qt::Unchecked);
    if (state == Qt::PartiallyChecked)
      {
      state = s;
      }
    else if (state != s)
      {
      return Qt::PartiallyChecked;
      }
    }
  return state;
}

//-----------------------------------------------------------------------------
void vgMixerWidgetPrivate::setGroupValue(vgMixerDrawer* group, double value)
{
  foreach (vgMixerDrawer* subgroup, this->groupGroups_.values(group))
    {
    this->setGroupValue(subgroup, value);
    }
  foreach (vgMixerItem* item, this->groupItems_.values(group))
    {
    item->setValue(value);
    }
}

//END vgMixerWidgetPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vgMixerWidget

//-----------------------------------------------------------------------------
vgMixerWidget::vgMixerWidget(QWidget* parent) :
  qtDrawerWidget(parent),
  d_ptr(new vgMixerWidgetPrivate)
{
  QTE_D(vgMixerWidget);
  d->enableUpdates_ = true;
  this->clear();
}

//-----------------------------------------------------------------------------
vgMixerWidget::~vgMixerWidget()
{
}

//-----------------------------------------------------------------------------
qtDrawer* vgMixerWidget::createRoot()
{
  qtDrawer* r = new vgMixerDrawer(this);
  return r;
}

//-----------------------------------------------------------------------------
int vgMixerWidget::beginGroup(const QString& text, bool checkable)
{
  QTE_D(vgMixerWidget);

  int id = this->addGroup(text, checkable, d->group_.top()->id());
  d->group_.push(d->groups_.value(id));
  return id;
}

//-----------------------------------------------------------------------------
void vgMixerWidget::endGroup()
{
  QTE_D(vgMixerWidget);

  if (d->group_.size() <= 1)
    {
    qDebug() << "vgMixerWidget: error: endGroup called without matching call"
                " to beginGroup";
    return;
    }

  d->group_.pop();
}

//-----------------------------------------------------------------------------
int vgMixerWidget::addGroup(const QString& text, int parent)
{
  return this->addGroup(text, false, parent);
}

//-----------------------------------------------------------------------------
int vgMixerWidget::addGroup(const QString& text, bool checkable, int group)
{
  QTE_D(vgMixerWidget);

  vgMixerDrawer* parent = d->groups_.value((group < 0 ? 0 : group), 0);
  if (!parent)
    {
    qDebug() << "vgMixerWidget: error: cannot add group" << text <<
                " to parent group" << group << "- no such parent group exists";
    return -1;
    }

  int id = d->groupGroups_.count();
  vgMixerDrawer* drawer = new vgMixerDrawer(id, text, parent, checkable);

  connect(drawer, SIGNAL(checkToggled(int, bool)),
          this, SLOT(setGroupState(int, bool)));

  d->groups_.insert(id, drawer);
  d->groupGroups_.insert(parent, drawer);
  return id;
}

//-----------------------------------------------------------------------------
bool vgMixerWidget::addItem(int key, const QString& text, int group)
{
  QTE_D(vgMixerWidget);
  if (d->items_.contains(key))
    {
    qDebug() << "vgMixerWidget: error: cannot add item" << text
             << "with ID" << key << "- an item with that key already exists";
    return false;
    }

  vgMixerDrawer* parent =
    (group < 0 ? d->group_.top() : d->groups_.value(group, 0));
  if (group >= 0 && !parent)
    {
    qDebug() << "vgMixerWidget: error: cannot add item to group" << group
             << "- no such group exists";
    return false;
    }

  vgMixerItem* item = new vgMixerItem(key, text, parent);
  d->items_.insert(key, item);
  d->groupItems_.insert(parent, item);

  connect(item, SIGNAL(stateChanged(int, bool)),
          this, SIGNAL(stateChanged(int, bool)));
  connect(item, SIGNAL(invertedChanged(int, bool)),
          this, SIGNAL(invertedChanged(int, bool)));
  connect(item, SIGNAL(valueChanged(int, double)),
          this, SIGNAL(valueChanged(int, double)));

  connect(item, SIGNAL(stateChanged(int, bool)),
          this, SLOT(updateItemParentState(int)));
  connect(item, SIGNAL(invertedChanged(int,bool)),
          this, SLOT(updateItemParentState(int)));
  d->updateParentState(item);

  return true;
}

//-----------------------------------------------------------------------------
bool vgMixerWidget::removeGroup(int group)
{
  QTE_D(vgMixerWidget);

  vgMixerDrawer* drawer = d->groups_.value(group, 0);
  if (!drawer)
    {
    qDebug() << "vgMixerWidget: error: cannot remove group" << group
             << "- no such group exists";
    return false;
    }

  d->removeGroup(drawer);
  return true;
}

//-----------------------------------------------------------------------------
bool vgMixerWidget::removeItem(int key)
{
  QTE_D(vgMixerWidget);
  if (!d->items_.contains(key))
    {
    qDebug() << "vgMixerWidget: error: cannot remove item with ID" << key
             << "- no such item exists";
    return false;
    }

  d->removeItem(key);
  return true;
}

//-----------------------------------------------------------------------------
void vgMixerWidget::clear()
{
  QTE_D(vgMixerWidget);
  qtDrawerWidget::clear();

  d->items_.clear();
  d->groupItems_.clear();
  d->group_.clear();
  d->groups_.clear();
  d->groupGroups_.clear();

  vgMixerDrawer* r = qobject_cast<vgMixerDrawer*>(this->root());
  d->group_.push(r);
  d->groups_.insert(0, r);
  d->groupGroups_.insert(0, r);
}

//-----------------------------------------------------------------------------
QList<int> vgMixerWidget::keys()
{
  QTE_D(vgMixerWidget);
  return d->items_.keys();
}

//-----------------------------------------------------------------------------
#define WRAP_ITEM_GET(_type, _func, _default) \
  _type vgMixerWidget::_func(int key) const \
    { \
    QTE_D_CONST(vgMixerWidget); \
    if(!d->items_.contains(key)) \
      return _default; \
    return d->items_[key]->_func(); \
    }

WRAP_ITEM_GET(bool, state, false)
WRAP_ITEM_GET(bool, isInverted, false)
WRAP_ITEM_GET(double, value, qSNaN())
WRAP_ITEM_GET(double, minimum, qSNaN())
WRAP_ITEM_GET(double, maximum, qSNaN())
WRAP_ITEM_GET(double, singleStep, qSNaN())
WRAP_ITEM_GET(double, pageStep, qSNaN())

//-----------------------------------------------------------------------------
#define ITEM_SET_PROLOGUE \
  QTE_D(vgMixerWidget); \
  if(!d->items_.contains(key)) return;

#define GROUP_SET_PROLOGUE \
  QTE_D(vgMixerWidget); \
  if(!d->groups_.contains(group)) return;

//-----------------------------------------------------------------------------
void vgMixerWidget::setText(int key, const QString& text)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setText(text);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setInvertedText(int key, const QString& text)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setInvertedText(text);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setMinimum(int key, double minimum)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setMinimum(minimum);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setMaximum(int key, double maximum)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setMaximum(maximum);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setSingleStep(int key, double step)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setSingleStep(step);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setPageStep(int key, double step)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setPageStep(step);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setRange(int key, double minimum, double maximum)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setRange(minimum, maximum);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setRange(
  int key, double minimum, double maximum, double singleStep, double pageStep)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setRange(minimum, maximum, singleStep, pageStep);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setState(int key, bool state)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setState(state);
  if (d->enableUpdates_)
    {
    d->updateParentState(d->items_[key]);
    }
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setInverted(int key, bool state)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setInverted(state);
  if (d->enableUpdates_)
    {
    d->updateParentState(d->items_[key]);
    }
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setValue(int key, double value)
{
  ITEM_SET_PROLOGUE
  d->items_[key]->setValue(value);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setGroupVisible(int group, bool visible)
{
  GROUP_SET_PROLOGUE
  d->groups_[group]->setVisibility(visible);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setGroupState(int group, bool state)
{
  GROUP_SET_PROLOGUE
  qtScopedValueChange<bool> disableUpdates(d->enableUpdates_, false);
  d->setGroupState(d->groups_[group], state);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setGroupInverted(int group, bool state)
{
  GROUP_SET_PROLOGUE
  qtScopedValueChange<bool> disableUpdates(d->enableUpdates_, false);
  d->setGroupInverted(d->groups_[group], state);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setGroupValue(int group, double value)
{
  GROUP_SET_PROLOGUE
  d->setGroupValue(d->groups_[group], value);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::setExpanded(int group, bool expanded)
{
  GROUP_SET_PROLOGUE
  d->groups_[group]->setExpanded(expanded);
}

//-----------------------------------------------------------------------------
void vgMixerWidget::updateItemParentState(int key)
{
  ITEM_SET_PROLOGUE
  d->updateParentState(d->items_[key]);
}

//END vgMixerWidget
