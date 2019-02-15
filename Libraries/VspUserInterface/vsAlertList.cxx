/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsAlertList.h"

#include <qtConfirmationDialog.h>

#include <vgSwatchCache.h>

#include "vsAlertEditor.h"

Q_DECLARE_METATYPE(vsAlert)

QTE_IMPLEMENT_D_FUNC(vsAlertList)

//BEGIN item data helpers

namespace // anonymous
{

enum DataRole
{
  IdRole = Qt::UserRole,
  ActivationStateRole,
  AlertRole,
  MatchesRole
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
bool itemActivationState(QTreeWidgetItem* item)
{
  return item->data(0, ActivationStateRole).toBool();
}

//-----------------------------------------------------------------------------
void setItemActivationState(QTreeWidgetItem* item, bool newState)
{
  item->setData(0, ActivationStateRole, QVariant(newState));
  item->setText(1, newState ? "Active" : "Inactive");
}

//-----------------------------------------------------------------------------
vsAlert itemAlert(QTreeWidgetItem* item)
{
  return item->data(0, AlertRole).value<vsAlert>();
}

//-----------------------------------------------------------------------------
void setItemAlert(QTreeWidgetItem* item, vsAlert newAlert,
                  const vgSwatchCache* swatchCache)
{
  item->setData(0, AlertRole, QVariant::fromValue(newAlert));
  item->setText(0, newAlert.eventInfo.name);

  if (swatchCache)
    {
    const QColor color = newAlert.eventInfo.pcolor.toQColor();
    item->setIcon(0, swatchCache->swatch(color));
    }
}

//-----------------------------------------------------------------------------
void setItemAlertData(QTreeWidgetItem* item, vsAlert newAlert)
{
  item->setData(0, AlertRole, QVariant::fromValue(newAlert));
}

//-----------------------------------------------------------------------------
void setItemMatches(QTreeWidgetItem* item, int newMatches)
{
  item->setData(0, MatchesRole, QVariant(newMatches));
  item->setText(2, QString::number(newMatches));
}

} // namespace <anonymous>

//END item data helpers

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsAlertListPrivate

//-----------------------------------------------------------------------------
class vsAlertListPrivate
{
public:
  vsAlertListPrivate(vsAlertList* q);

  void removeAlerts(const QList<QTreeWidgetItem*>&);

  bool ShowInactive;

  vsAlertEditor* ActiveEditor;
  QTreeWidgetItem* EditItem;

  const vgSwatchCache* SwatchCache;

  static const char* const ConfirmMessage;

protected:
  QTE_DECLARE_PUBLIC_PTR(vsAlertList)

private:
  QTE_DECLARE_PUBLIC(vsAlertList)
};


const char* const vsAlertListPrivate::ConfirmMessage =
  "Removing alerts will remove all associated events. Once an alert is"
  " removed, it must be reloaded or recreated before it can be used again."
  " (To stop the system processing an alert without removing the alert;"
  " deactivate the alert instead.)\n\nProceed with removal of %1 %2?";

//-----------------------------------------------------------------------------
vsAlertListPrivate::vsAlertListPrivate(vsAlertList* q)
  : ShowInactive(true), ActiveEditor(0), EditItem(0), SwatchCache(0), q_ptr(q)
{
}

//-----------------------------------------------------------------------------
void vsAlertListPrivate::removeAlerts(const QList<QTreeWidgetItem*>& items)
{
  if (items.isEmpty())
    return;

  QTE_Q(vsAlertList);

  const char* noun = (items.count() > 1 ? "alerts" : "alert");
  const QString prompt = QString(vsAlertListPrivate::ConfirmMessage);
  bool sure = qtConfirmationDialog::getConfirmation(
                q->window(), "RemoveAllAlerts",
                prompt.arg(items.count()).arg(noun),
                QString("Remove %1").arg(noun),
                qtConfirmationDialog::RememberSession, "Are you sure?");
  if (sure)
    {
    foreach (QTreeWidgetItem* item, items)
      emit q->alertRemoved(itemId(item));
    }
  // Item(s) will be removed on signal from vsCore
}

//END vsAlertListPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsAlertList

//-----------------------------------------------------------------------------
vsAlertList::vsAlertList(QWidget* parent)
  : QTreeWidget(parent), d_ptr(new vsAlertListPrivate(this))
{
  this->sortItems(0, Qt::AscendingOrder);
}

//-----------------------------------------------------------------------------
vsAlertList::~vsAlertList()
{
}

//-----------------------------------------------------------------------------
void vsAlertList::setSwatchCache(const vgSwatchCache& sc)
{
  QTE_D(vsAlertList);
  d->SwatchCache = &sc;
}

//-----------------------------------------------------------------------------
void vsAlertList::addAlert(int id, vsAlert alert)
{
  QTE_D(vsAlertList);

  QTreeWidgetItem* item = new QTreeWidgetItem;
  setItemId(item, id);
  setItemAlert(item, alert, d->SwatchCache);
  setItemActivationState(item, true);
  item->setText(2, QString::number(0));
  this->addTopLevelItem(item);
}

//-----------------------------------------------------------------------------
void vsAlertList::updateAlert(int id, vsAlert alert)
{
  QTE_D(vsAlertList);

  foreach_child (QTreeWidgetItem* item, this->invisibleRootItem())
    {
    if (itemId(item) == id)
      {
      setItemAlert(item, alert, d->SwatchCache);
      if (item == d->EditItem && d->ActiveEditor)
        {
        d->ActiveEditor->setAlert(alert);
        }
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vsAlertList::updateAlertThreshold(int id, double threshold)
{
  QTE_D(vsAlertList);

  foreach_child (QTreeWidgetItem* item, this->invisibleRootItem())
    {
    if (itemId(item) == id)
      {
      vsAlert alert = itemAlert(item);
      alert.displayThreshold = threshold;
      setItemAlertData(item, alert);
      if (item == d->EditItem && d->ActiveEditor)
        {
        d->ActiveEditor->setAlert(alert);
        }
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vsAlertList::setAlertMatches(int id, int matches)
{
  for (int i = 0, count = this->topLevelItemCount(); i < count; ++i)
    {
    QTreeWidgetItem* item = this->topLevelItem(i);
    itemId(item) == id && (setItemMatches(item, matches), false);
    }
}

//-----------------------------------------------------------------------------
void vsAlertList::setAlertEnabled(int id, bool enabled)
{
  QTE_D(vsAlertList);

  foreach_child (QTreeWidgetItem* item, this->invisibleRootItem())
    {
    if (itemId(item) == id)
      {
      setItemActivationState(item, enabled);
      item->setHidden(!(enabled || d->ShowInactive));
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vsAlertList::removeAlert(int id)
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
void vsAlertList::removeSelected()
{
  QTE_D(vsAlertList);
  d->removeAlerts(this->selectedItems());
}

//-----------------------------------------------------------------------------
void vsAlertList::removeAll()
{
  QTE_D(vsAlertList);

  QList<QTreeWidgetItem*> items;
  foreach_child (QTreeWidgetItem* item, this->invisibleRootItem())
    items.append(item);

  d->removeAlerts(items);
}

//-----------------------------------------------------------------------------
void vsAlertList::activateSelected()
{
  this->setSelectedActivation(true);
}

//-----------------------------------------------------------------------------
void vsAlertList::deactivateSelected()
{
  this->setSelectedActivation(false);
}

//-----------------------------------------------------------------------------
void vsAlertList::setSelectedActivation(bool newState)
{
  foreach (QTreeWidgetItem* item, this->selectedItems())
    {
    if (itemActivationState(item) != newState)
      emit this->alertActivationChanged(itemId(item), newState);
    }
  // Item activation state(s) will be updated on signal from vsCore
}

//-----------------------------------------------------------------------------
void vsAlertList::setShowInactive(bool newFlag)
{
  QTE_D(vsAlertList);

  if (newFlag != d->ShowInactive)
    {
    d->ShowInactive = newFlag;
    for (int i = 0, count = this->topLevelItemCount(); i < count; ++i)
      {
      QTreeWidgetItem* item = this->topLevelItem(i);
      bool show = itemActivationState(item) || d->ShowInactive;
      item->setHidden(!show);
      }
    }
}

//-----------------------------------------------------------------------------
bool vsAlertList::edit(
  const QModelIndex& index, QAbstractItemView::EditTrigger trigger, QEvent* e)
{
  if (this->editTriggers().testFlag(trigger))
    {
    QTreeWidgetItem* item = this->itemFromIndex(index);
    if (item)
      {
      QTE_D(vsAlertList);

      vsAlertEditor editor;
      d->ActiveEditor = &editor;
      d->EditItem = item;

      connect(&editor, SIGNAL(changesApplied()),
              this, SLOT(editAlertChanged()));

      editor.setAlert(itemAlert(item));
      editor.selectName();
      editor.exec();

      d->ActiveEditor = 0;
      d->EditItem = 0;
      return false;
      }
    }

  return QTreeWidget::edit(index, trigger, e);
}

//-----------------------------------------------------------------------------
void vsAlertList::editAlertChanged()
{
  QTE_D(vsAlertList);

  if (d->ActiveEditor && d->EditItem)
    {
    vsAlert updatedAlert = d->ActiveEditor->alert();
    emit this->alertChanged(itemId(d->EditItem), updatedAlert);
    }
  // Item alert will be updated on signal from vsCore
}

//END vsAlertList
